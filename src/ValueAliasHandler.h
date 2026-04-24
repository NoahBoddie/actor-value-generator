#pragma once

#include "ExtraValueInfo.h"
#include "string.h"

namespace AVG
{
	//constexpr 


	class ExtraValueInfo;

	enum struct AliasMode
	{
		Default,	//Whether later plugins use alias is determined by if it's a master of the current
		Exclusive,	//Exclusively used on the it was included on
		Inclusive,
		Inherent,	//Cannot be removed by exclusions.
	};

	//The actor values that need covering will be:
	// all of the actor values of course, 
	// none (represented by total ig)
	// cost
	// left cost
	// right cost
	// dual cost
	// Staff cost
	//All these costs are effectively imaginary

	enum struct VirtualValue
	{
		kCost = RE::ActorValue::kTotal,
		kLeftCost,
		kRightCost,
		kNone,

		kTotal,

	};

	inline VirtualValue StringToVirtualValue(std::string_view name)
	{
#define CASE_OF_RETURN(mc_case_name) case #mc_case_name##_ih: return VirtualValue::k##mc_case_name

		switch(Hash<HashFlags::Insensitive>(name))
		{
			CASE_OF_RETURN(Cost);
			CASE_OF_RETURN(LeftCost);
			CASE_OF_RETURN(RightCost);
		}
		
#undef CASE_OF_RETURN

		return VirtualValue::kTotal;
	}


	using AliasMask = std::bitset<(size_t)VirtualValue::kTotal>;



	struct UnionValue
	{

		static constexpr uint8_t none = (uint8_t)VirtualValue::kNone;
		static constexpr uint8_t invalid = (uint8_t)VirtualValue::kTotal;
		uint8_t _raw = invalid;


		//These will not work in concept. None is a valid value between them.
		std::optional<RE::ActorValue> GetActorValue()
		{
			if (IsActorValue() == false)
				return std::nullopt;

			if (_raw == invalid)
				return std::nullopt;

			if (_raw == none) {
				return RE::ActorValue::kNone;
			}

			return static_cast<RE::ActorValue>(_raw);
		}

		std::optional<VirtualValue> GetVirtualValue()
		{
			if (IsActorValue() == true)
				return std::nullopt;

			return static_cast<VirtualValue>(_raw);
		}

		uint8_t value()
		{
			return _raw;
		}

		operator uint8_t()
		{
			return _raw;
		}

		bool IsInvalid()
		{
			return _raw == invalid;
		}

		bool IsActorValue() const
		{
			return  _raw == none || _raw < (int)RE::ActorValue::kTotal && _raw != invalid;
		}


		constexpr UnionValue() = default;

		constexpr UnionValue(RE::ActorValue value) : _raw{ value == RE::ActorValue::kNone ? none : static_cast<uint8_t>(value) }
		{
			
		}

		constexpr UnionValue(VirtualValue value) : _raw{ static_cast<uint8_t>(value) }
		{
		}

		UnionValue& operator=(RE::ActorValue value)
		{
			if (value == RE::ActorValue::kNone) {
				_raw = none;
			}
			else {
				_raw = static_cast<uint8_t>(value);
			}

			return *this;
		}

		UnionValue& operator=(VirtualValue value)
		{
			_raw = static_cast<uint8_t>(value);
			return *this;
		}


	};


	struct AliasData
	{
		AliasMode mode = AliasMode::Default;

		//I'm thinking that some aliases can be applied only to skills of some kind.

		ExtraValueInfo* extraValue = nullptr;
	};


	ENUM(MaskType)
	{
		kGet,
		kDamage,
		kModifier,
		kSkill,
		kAdvance,
		//kCost, //To be a cost, something only needs to be able to get and set damage.
		kExclude,
		kTotal,
		kLast = MaskType::kTotal - 1,
	};


	inline AliasMask globalMask[MaskType::kTotal];
	constexpr size_t sizef = sizeof(globalMask);

	struct AliasParams
	{
		UnionValue alias;
		std::span<MaskType> types;
	};

	struct AliasSetting
	{
		AliasMask localMask[MaskType::kLast];
		std::unordered_map<uint8_t, AliasData> nodes;


		bool AddSetting(UnionValue alias, ExtraValueInfo* value, AliasMode mode)
		{
			//if (alias == RE::ActorValue::kNone) {
			//	logger::error("Alias cannot be None.");
			//	return false;
			//}

			if (alias.GetVirtualValue() == VirtualValue::kTotal) {
				logger::error("Alias type invalid.");
				return false;
			}

			AliasData& data = nodes[alias];
			//AliasSetting& setting = actorValueAliases[alias];//  alias == RE::ActorValue::kNone ? actorValueAliases[RE::ActorValue::kAggression] : actorValueAliases[alias];

			if (data.extraValue) {
				logger::warn("Value Alias {} for {} is being overriden by {}", (int)alias, data.extraValue->GetName(), value->GetName());
			}
			//logger::info("address {:X}", (uint64_t)this);

			data.extraValue = value;
			data.mode = mode;

			localMask[MaskType::kGet].set(alias, true);
			globalMask[MaskType::kGet].set(alias, true);

			if (value->AllowsDamage() == true) {
				localMask[MaskType::kDamage].set(alias, true);
				globalMask[MaskType::kDamage].set(alias, true);
			}

			if (value->AllowsModifier() == true) {
				localMask[MaskType::kModifier].set(alias, true);
				globalMask[MaskType::kModifier].set(alias, true);
			}


			if (value->AllowsSkill() == true) {
				localMask[MaskType::kSkill].set(alias, true);
				globalMask[MaskType::kSkill].set(alias, true);
			}

			if (value->AllowsAdvance() == true) {
				localMask[MaskType::kAdvance].set(alias, true);
				globalMask[MaskType::kAdvance].set(alias, true);
			}



			return true;
		}

		bool IsAliasValidSafe(uint8_t alias, const MaskType* types, size_t size)
		{
			if (!this)
				return false;

			while (size > 0)
			{
				auto type = types[--size];

				if (localMask[type].test(alias) == false){
					return false;
				}
			}


			return true;
		}

		bool IsAliasValidSafe(uint8_t alias, std::span<const MaskType> types)
		{
			if (!this)
				return false;

			for (auto type : types)
			{
				if (localMask[type].test(alias) == false) {
					return false;
				}
			}


			return true;
		}

		bool IsAliasValidSafe(AliasParams& params)
		{
			if (!this)
				return false;

			for (auto type : params.types)
			{
				if (localMask[type].test(params.alias) == false) {
					return false;
				}
			}


			return true;
		}

	};





	ENUM(AliasSet)
	{
		_begin = -1,

		//Note for ID related stuff I only need to preserve the ID of stuff I can actually change. Granted, that's a lot.
		ExactForm,
		ExactID,
		Match,
		Contains,
		List,
		
		Keyword,
		Plugin,
		Total,

		Start = AliasSet::_begin + 1,
	};

	//inline std::map<std::string, AliasSetting> aliasMap[AliasSet::Total];

	struct IAliasMapper
	{
		//evals if it can be added
		virtual AliasSetting& Obtain(const std::string& str) = 0;

		virtual AliasSetting* Find(RE::TESForm* form, const std::string& str, AliasParams* params) = 0;
		//Should have a kind of add function
	};


	static constexpr std::string_view affixChars = "...";
	static constexpr size_t affixLength = affixChars.size();


	struct ContainsID
	{
		enum Enum
		{
			kPrefix,
			kSuffix, 
			kContains,
		};
		std::string text;

		ContainsID() = default;
		ContainsID(const std::string& str) : text{ str } {}
		

		bool Match(std::string_view view)
		{
			return view.contains(text);
		}
	};

	struct MatchID : public ContainsID
	{
		MatchID(const std::string& str)
		{
			if (str.starts_with(affixChars) == true) {
				text = str.substr(affixLength);
				type = Enum::kSuffix;
			}
			else if (str.ends_with(affixChars) == true) {
				text = str.substr(0, str.size() - affixLength);
				type = Enum::kPrefix;
			}
			else {
				//This shouldn't happen
				text = str;
				type = Enum::kContains;
			}
		}

		Enum type{};

		bool Match(std::string_view view)
		{
			switch (type)
			{
			case Enum::kPrefix:
				return view.size() > text.size() && view.find(text) == 0;

			case Enum::kSuffix:
				return view.size() > text.size() && view.find(text) == (view.size() - text.size());
		
			case Enum::kContains: 
				return __super::Match(view);
		
			default:
				return false;
			}

			return view.contains(text);
		}
	};


	template <std::derived_from<ContainsID> Key>
	struct EditorIDAliasMap : public IAliasMapper
	{
		std::vector<std::pair<Key, AliasSetting>> data;


		AliasSetting& Obtain(const std::string& str) override
		{
			auto& setting = data.emplace_back(Key{ str }, AliasSetting{});

			return setting.second;
		}



		AliasSetting* Find(RE::TESForm* form, const std::string& str, AliasParams* params) override
		{
			for (auto& [id, setting] : data)
			{
				if (id.Match(str) == true)
					if (!params || setting.IsAliasValidSafe(params->alias, params->types))
						return &setting;
			}

			return nullptr;
		}

	};

	using ContainsIDAliasMap = EditorIDAliasMap<ContainsID>;
	using MatchIDAliasMap = EditorIDAliasMap<MatchID>;


	struct IFormStringAliasMap : public IAliasMapper
	{
		using FormStringMap = std::map<std::string, AliasSetting>;


		bool _init = false;
		FormStringMap _stringData;

		


		AliasSetting& Obtain(const std::string& str) override
		{
			return _stringData[str];
		}

		virtual void Initialize() = 0;


		void InitImpl()
		{
			if (!_init) {
				
				Initialize();
				_stringData.clear();
			}
		}
	};
	

	struct FormAliasMap : public IFormStringAliasMap
	{


		std::map<RE::TESForm*, AliasSetting> data;
		
		void Initialize() override
		{
			for (auto& [key, setting] : _stringData)
			{
				try
				{
					RE::TESForm* form = GetForm(key);

					if (form) {
						data[form] = std::move(setting);
					}
					else {
						logger::error("Failed to find alias target: {}", key);
					}
				}
				catch (std::invalid_argument& error)
				{
					logger::error("Invalid form string: {}", key);
				}
				//else message

				
			}
		}




		AliasSetting& Obtain(const std::string& str) override
		{
			return _stringData[str];
		}

		AliasSetting* Find(RE::TESForm* form, const std::string& str, AliasParams* params) override
		{
			InitImpl();

			auto it = data.find(form);
			
			if (data.end() == it) {
				return nullptr;
			}
			
			auto result = &it->second;

			if (params && result->IsAliasValidSafe(params->alias, params->types) == false){
				return nullptr;
			}
			return result;
		}


	};

	struct FormListAliasMap : public IFormStringAliasMap
	{


		std::vector<std::pair<RE::BGSListForm*, AliasSetting>> data;
		

		void Initialize()
		{
			data.reserve(_stringData.size());

			for (auto& [key, setting] : _stringData)
			{
				try
				{
					RE::BGSListForm* form = GetForm<RE::BGSListForm>(key);

					if (form) {
						data.emplace_back(form, std::move(setting));
					}
					else {
						logger::error("Failed to find Formlist target: {}", key);
					}
				}
				catch (std::invalid_argument& error)
				{
					logger::error("Invalid form string: {}", key);
				}
				
			}
			
		}


		AliasSetting* Find(RE::TESForm* form, const std::string& str, AliasParams* params) override
		{
			InitImpl();

			for (auto& [list, setting] : data)
			{
				if (!list)
					continue;


				auto it = std::find(list->forms.begin(), list->forms.end(), form);
				
				if (list->forms.end() == it) {
					continue;
				}

				if (params && setting.IsAliasValidSafe(params->alias, params->types) == false) {
					continue;
				}

				return &setting;
			}

			return nullptr;
		}

	};

	struct StringAliasMap : public IAliasMapper
	{
		std::map<std::string, AliasSetting> data;

		AliasSetting& Obtain(const std::string& str) override
		{
			return data[str];
			
		}

		AliasSetting* Find(RE::TESForm* form, const std::string& view, AliasParams* params) override
		{
			auto it = data.find(view);

			if (data.end() == it)
				return nullptr;
			
			AliasSetting* setting = &(it->second);
			
			return !params || setting->IsAliasValidSafe(params->alias, params->types) ? setting : nullptr;

		}
	};


	struct AliasMap

	{
		//Put this in a struct maybe?
		inline static FormAliasMap exactFormAliasMap;
		inline static StringAliasMap exactIdAliasMap;
		inline static MatchIDAliasMap matchIDAliasMap;
		inline static ContainsIDAliasMap containsIDAliasMap;
		inline static FormListAliasMap formListAliasMap;
		inline static StringAliasMap keywordAliasMap;
		inline static StringAliasMap pluginAliasMap;

		inline static std::array<IAliasMapper*, AliasSet::Total> aliasMap;
		

		static void CheckInit()
		{
			if (aliasMap[0] == nullptr)
			{
				aliasMap[AliasSet::ExactForm] = &exactFormAliasMap;
				aliasMap[AliasSet::ExactID] = &exactIdAliasMap;
				aliasMap[AliasSet::Match] = &matchIDAliasMap;
				aliasMap[AliasSet::Contains] = &containsIDAliasMap;
				aliasMap[AliasSet::List] = &formListAliasMap;
				aliasMap[AliasSet::Keyword] = &keywordAliasMap;
				aliasMap[AliasSet::Plugin] = &pluginAliasMap;
			}

		}

		static AliasSet GetSetFromString(std::string& str)
		{
			
			if (int i = str.starts_with('|') + str.ends_with('|'); i)
			{
				if (i == 2) {
					str = str.substr(1, str.size() - 2);
					return AliasSet::List;//formlist
				}
			}
			else if (int i = str.starts_with('<') + str.ends_with('>'); i) {
				if (i == 2) {
					str = str.substr(1, str.size() - 2);

					if (int i = str.starts_with(affixChars) + str.ends_with(affixChars); i) {
						if (i == 2) {
							str = str.substr(affixLength, str.size() - affixLength * 2);
							return AliasSet::Contains;//contains
						}
						else{
							return AliasSet::Match;//prefix/suffix

						}
					}
					else {
						return AliasSet::ExactID;//exact
					}
				}

			}
			else if (auto it = str.find("::"); it != std::string::npos) {
				if (it == 0) {
					str = str.substr(2);
				}
				
				return AliasSet::ExactForm;//exact form
			}
			else if (str.contains(".es") == true) {
				return AliasSet::Keyword;
			}
			else {
				return AliasSet::Keyword;//keyword
			}

			return AliasSet::Total;
		}


		static AliasSetting* ObtainSetting(std::string text, AliasSet* out = nullptr)
		{

			CheckInit();

			AliasSet set = GetSetFromString(text);

			if (out) {
				*out = set;
			}

			if (set == AliasSet::Total) {
				return nullptr;
			}

			return &aliasMap[set]->Obtain(text);

		}

		inline static AliasSetting* ObtainSetting(std::string text, AliasSet& out)
		{
			return ObtainSetting(std::move(text), &out);
		}
		static IAliasMapper& GetAliasMap(AliasSet set)
		{
			//rename this

			CheckInit();

			return *aliasMap[set];
		}
	};
	

	

	//~~~
	
	//ExactID,
	//Prefix,
	//Suffix,
	//Contains,
	//List,

	//Keyword,
	//Plugin,
	



	//inline std::tuple<int> aliasTuple{};
	//static_assert(std::tuple_size_v<decltype(aliasTuple)> == AliasSet::Total);


	inline AliasSetting* GetPluginAlias(RE::TESFile* plugin, AliasParams* params)
	{
		if (!plugin)
			return nullptr;

		std::string file_name(plugin->GetFilename());

		auto& alias_map = AliasMap::GetAliasMap(AliasSet::Plugin);

		return alias_map.Find(nullptr, file_name, params);
		
	}


	inline AliasSetting* GetPluginAlias(RE::TESFile* plugin, AliasParams& params)
	{
		return GetPluginAlias(plugin, &params);
	}



	inline bool FileIsMasterToFile(RE::TESFile* master, RE::TESFile* student)
	{
		if (!master || !student)
			return false;

		auto size = student->masterCount;
		for (int i = 0; i < size; i++)
		{
			if (student->masterPtrs[i] == master)
				return true;

		}
		return false;
	}

	/*
	Is this really not used?
	inline AliasData* GetPluginNode(RE::TESFileArray* source_array, uint8_t value, bool require_set)
	{
		if (!source_array)
			return nullptr;

		auto& sourceFiles = *source_array;
		

		RE::TESFile* current = sourceFiles[0];
		//RE::TESFile* source = sourceFiles[size - 1];//If a file is equal to this, it cannot have its aliases taken away.
		

		AliasSetting* result = GetPluginAlias(current);
		
		if (result->IsAliasValidSafe(value, require_set) == false)
		{
			result = nullptr;

			auto size = source_array->size();


			for (int i = 1; i < size; i++)
			{
				RE::TESFile* file = sourceFiles[i];

				if (FileIsMasterToFile(file, current) == false)
					continue;

				auto setting = GetPluginAlias(current);

				if (setting->IsAliasValidSafe(value, require_set) == true) {
					result = setting;
					break;
				}
			}

			if (!result)
			{
				auto size = current->masterCount;
				
				for (int i = 0; i < size; i++)
				{
					RE::TESFile* file = sourceFiles[i];

					auto setting = GetPluginAlias(file);

					if (setting->IsAliasValidSafe(value, require_set) == true) {
						result = setting;
						break;
					}
				}
			}

		}

		if (result)
		{
			return result ? &result->nodes[value] : nullptr;
		}
	}
	//*/



	//*
	enum AliasPriority : uint8_t
	{
		SourceMaster,
		Source,
		SourceChild,
		LastMaster,
		Last,
		CurrentMaster,
		Current,
		Total
		//Excluded = 1 << 7
	};


	namespace
	{
		AliasSetting* GetMaster(RE::TESFile* target, AliasParams* params)
		{
			AliasSetting* result = nullptr;

			auto size = target->masterCount;

			for (int i = 0; i < size; i++)
			{
				RE::TESFile* file = target->masterPtrs[i];

				if (auto setting = GetPluginAlias(file, params)) {
					result = setting;
					break;
				}
			}


			return result;
		}

		void LoadPrimary(RE::TESFileArray* source_array, AliasParams& params)
		{
			std::array<AliasSetting*, AliasPriority::Total> aliases{};

			if (!source_array)
				return;


			RE::TESFile** sourceFiles = source_array->data();
			int end = source_array->size() - 1;

			RE::TESFile* current = sourceFiles[0];
			RE::TESFile* source = sourceFiles[end];
			RE::TESFile* last = nullptr;


			if (AliasSetting* node = GetPluginAlias(source, &params)) {
				aliases[AliasPriority::Source] = node;
			}

			for (int i = 0; i <= end; i++)
			{
				//I would like this to have 2 mods one where last hasn't been found, and one where it has been found
				// but hierarchy is being searched. Because currently, if something that branches form source 
				// overrides that won't count.

				if (i == end) {
					aliases[AliasPriority::Last] = aliases[AliasPriority::Source];
					last = source;
					break;
				}

				last = sourceFiles[i];

				if (AliasSetting* node = GetPluginAlias(last, &params)) {
					aliases[AliasPriority::Last] = node;
					break;
				}
			}


			bool useLast = false;
			
			//Last may have a setting that rids this of it
			if (FileIsMasterToFile(last, current) == true) {
				useLast = true;
			}


			if (last == current) {
				aliases[AliasPriority::Current] = aliases[AliasPriority::Last];
				//return; static_assert(false);
			}
			



			if (AliasSetting* node = GetPluginAlias(current, nullptr); node) {
				if (node->IsAliasValidSafe(params.alias, params.types) == true) {
					aliases[AliasPriority::Current] = node;
				}
			}


			else {
				//If you are a child of one of the last.

				//If one is a child of both, whichever is lower should go first. So, last.
				bool is_source = false;

				for (int i = 0; i < current->masterCount; i++) {
					if (!is_source && current->masterPtrs[i] == source) {
						aliases[AliasPriority::Current] = aliases[AliasPriority::Source];
					}
					else if (current->masterPtrs[i] == last) {
						aliases[AliasPriority::Current] = aliases[AliasPriority::Last];
						break;
					}

				}
			}
		}

	}

	struct AliasPriorityList
	{
		std::array<AliasSetting*, AliasPriority::Total> aliases{};


		AliasSetting* GetPrimarySetting()
		{
			if (aliases[AliasPriority::Current])
				return aliases[AliasPriority::Current];

			if (aliases[AliasPriority::Last])
				return aliases[AliasPriority::Last];

			if (aliases[AliasPriority::Source])
				return aliases[AliasPriority::Source];

			//logger::info("returning no primary.");

			return nullptr;

		}

		AliasPriorityList() = default;

		void LoadPrimary(RE::TESFileArray* source_array, AliasParams& params)
		{
			if (!source_array)
				return;


			RE::TESFile** sourceFiles = source_array->data();
			int end = source_array->size() - 1;

			RE::TESFile* current = sourceFiles[0];
			RE::TESFile* source = sourceFiles[end];
			RE::TESFile* last = nullptr;


			if (AliasSetting* node = GetPluginAlias(source, params)) {
				aliases[AliasPriority::Source] = node;
			}

			for (int i = 0; i <= end; i++)
			{
				//I would like this to have 2 mods one where last hasn't been found, and one where it has been found
				// but hierarchy is being searched. Because currently, if something that branches form source 
				// overrides that won't count.

				if (i == end) {
					aliases[AliasPriority::Last] = aliases[AliasPriority::Source];
					last = source;
					break;
				}

				last = sourceFiles[i];

				if (AliasSetting* node = GetPluginAlias(last, params)) {
					aliases[AliasPriority::Last] = node;
					break;
				}
			}


			if (last == current) {
				aliases[AliasPriority::Current] = aliases[AliasPriority::Last];
				return;
			}

			if (AliasSetting* node = GetPluginAlias(current, nullptr); node) {
				if (node->IsAliasValidSafe(params) == true) {
					aliases[AliasPriority::Current] = node;
				}
			}
			else {
				//If you are a child of one of the last.

				//If one is a child of both, whichever is lower should go first. So, last.
				bool is_source = false;

				for (int i = 0; i < current->masterCount; i++) {
					if (!is_source && current->masterPtrs[i] == source) {
						aliases[AliasPriority::Current] = aliases[AliasPriority::Source];
					}
					else if (current->masterPtrs[i] == last) {
						aliases[AliasPriority::Current] = aliases[AliasPriority::Last];
						break;
					}

				}
			}
		}

	};


	inline AliasSetting* GetSettingFromPriority(RE::TESFileArray* source_array, AliasParams& params)
	{
		AliasPriorityList list{};
		list.LoadPrimary(source_array, params);
		return list.GetPrimarySetting();
	}
	//*/


	inline AliasData* GetDataFromSet(RE::TESForm* form, AliasSet set, AliasParams& params)
	{

		auto& alias_map = AliasMap::GetAliasMap(set);

		switch (set)
		{
		default:
			logger::critical("Cannot use.");
			throw nullptr;

		case AliasSet::List:
		case AliasSet::ExactForm: {
			if (AliasSetting* setting = alias_map.Find(form, "", &params))
				return &setting->nodes[params.alias];

			break;
		}
		case AliasSet::ExactID:
		case AliasSet::Contains:
		case AliasSet::Match: {
			if (AliasSetting* setting = alias_map.Find(form, clib_util::editorID::get_editorID(form), &params))
				return &setting->nodes[params.alias];

			break;
		}

		case AliasSet::Keyword:
		{
			RE::BGSKeywordForm* keyword_form = skyrim_cast<RE::BGSKeywordForm*>(form);

			if (!keyword_form)
				return nullptr;


			for (int i = 0; i < keyword_form->numKeywords; i++)
			{
				RE::BGSKeyword* keyword = *keyword_form->GetKeywordAt(i);
				
				if (AliasSetting* setting = alias_map.Find(form, keyword->formEditorID.c_str(), &params))
					return &setting->nodes[params.alias];
				
			}

			return nullptr;
		}

		case AliasSet::Plugin:
		{
			//*
			auto result = GetSettingFromPriority(form->sourceFiles.array, params);

			return result ? &result->nodes[params.alias] : nullptr;
			/*/
			auto source_array = form->sourceFiles.array;

			if (!source_array)
				return nullptr;

			auto& sourceFiles = *source_array;


			RE::TESFile* current = sourceFiles[0];
			//RE::TESFile* source = sourceFiles[size - 1];//If a file is equal to this, it cannot have its aliases taken away.


			AliasSetting* result = GetPluginAlias(current);

			if (result->IsAliasValidSafe(value, masks, a_size) == false)
			{
				result = nullptr;

				auto size = source_array->size();


				for (int i = 1; i < size; i++)
				{
					RE::TESFile* file = sourceFiles[i];

					if (FileIsMasterToFile(file, current) == false)
						continue;

					auto setting = GetPluginAlias(current);

					if (setting->IsAliasValidSafe(value, masks, a_size) == true) {
						result = setting;
						break;
					}
				}

				if (!result)
				{
					auto size = current->masterCount;

					for (int i = 0; i < size; i++)
					{
						RE::TESFile* file = current->masterPtrs[i];

						auto setting = GetPluginAlias(file);

						if (setting->IsAliasValidSafe(value, masks, a_size) == true) {
							result = setting;
							break;
						}
					}
				}

			}

			return result ? &result->nodes[value] : nullptr;
			//*/

			break;
		}
		
		}

		return nullptr;
	}



	//What should the priority be? I think it should be arranged by how intentional a given thing is
	ENUM(QuerySettings)
	{
		None = 0,
		AllowNone = 1 << 0,
		RequiresDamage = 1 << 1,
		RequiresModifier = 1 << 2,
		RequiresSkill = 1 << 3,
		RequiresAdvance = 1 << 4,
		//IgnoreKeyword = 1 << 2,
		IgnorePlugin = 1 << 5,//Ignores plugin aliases, aliases like cost would prevent any spell from properly using the base cost.
	};



	//I would like to switch the 2 of these, that way I can make a parameter pack.
	static bool AliasToValue(RE::TESForm* form, UnionValue value, RE::ActorValue& out, QuerySettings settings = QuerySettings::None)
	{
		if (value.IsInvalid() || !form)
			return false;

		//Allows the value of "none" to be changed, but requires some manual placement rule to avoid over checking. Keyword, exact, and list
		// all having the least amount of chance for unintended mishaps with it's use.
		bool allow_none = settings & QuerySettings::AllowNone;



		std::vector<MaskType> masks{ MaskType::kGet };
		//std::span<MaskType> test = masks;

		if (settings & QuerySettings::RequiresDamage) {
			masks.push_back(MaskType::kDamage);
		}

		if (settings & QuerySettings::RequiresModifier) {
			masks.push_back(MaskType::kModifier);
		}

		//if advance, it's also a skill so no reason to mark that too.
		if (settings & QuerySettings::RequiresAdvance) {
			masks.push_back(MaskType::kAdvance);
		}
		else if (settings & QuerySettings::RequiresSkill) {
			masks.push_back(MaskType::kSkill);
		}





		RE::TESFileArray* source_array = form->sourceFiles.array;

		if (!source_array)
			return false;

		if (!allow_none && value.GetActorValue() == RE::ActorValue::kNone) {
			return false;
		}

		if (value > (int)VirtualValue::kTotal) {
			logger::warn("Stored Actor Value is already above total at  {}", (int)value);
			return false;
		}

		for (auto mask : masks)
		{
			if (globalMask[mask].test(value) == false) {
				//logger::debug("Alias for {} not implementing.", (int)alias);
				return false;
			}
		}



		AliasData* data = nullptr;

		AliasParams params{ value, masks };

		for (AliasSet set = AliasSet::Start; set != AliasSet::Total && !data; set++)
		{
			//Later, this needs to take a compiled list of all the exclusions. if I ever get around to doing that.
			data = GetDataFromSet(form, set, params);
		}

		if (!data)
			return false;

		

		ExtraValueInfo* info = data->extraValue;//->actorValueAliases[alias].extraValue;
		//ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(plugin_alias->aliasDataMap[spec_type][alias]);//->actorValueAliases[alias].extraValue;

		//logger::info("A");

		if (!info)
			return false;

		//logger::info("After");

		RE::ActorValue result;
		
		result = info->GetValueIDAsAV();

		logger::info("Actor Value {} has been converted into Extra Value {} for {:08X}.", magic_enum::enum_name(out), info->GetName(), form->formID);

		out = result;

		return true;
	}


	static bool AliasToValue(RE::TESForm* form, RE::ActorValue& out, QuerySettings settings = QuerySettings::None)
	{
		return AliasToValue(form, out, out, settings);
	}

	/*
	static bool AliasToValue(RE::TESForm* form, UnionValue value, SKSE::stl::enumeration<RE::ActorValue>& out, QuerySettings settings = QuerySettings::None)
	{
		return AliasToValue(form, out.get(), reinterpret_cast<RE::ActorValue&>(out), settings);
	}


	static bool AliasToValue(RE::TESForm* form, SKSE::stl::enumeration<RE::ActorValue>& out, QuerySettings settings = QuerySettings::None)
	{
		return AliasToValue(form, out.get(), reinterpret_cast<RE::ActorValue&>(out), settings);
	}
	//*/
}