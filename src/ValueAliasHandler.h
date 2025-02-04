#pragma once

#include "ExtraValueInfo.h"

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

		constexpr UnionValue(RE::ActorValue value) : _raw{ value == RE::ActorValue::kNone ? _raw = none : static_cast<uint8_t>(value) }
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
	};





	ENUM(AliasSet)
	{
		_begin = -1,

		//Note for ID related stuff I only need to preserve the ID of stuff I can actually change. Granted, that's a lot.
		//ExactID,
		//Prefix,
		//Suffix,
		//Contains,
		//List,
		
		Keyword,
		Plugin,
		Total,

		Start = AliasSet::_begin + 1,
	};

	inline std::map<std::string, AliasSetting> aliasMap[AliasSet::Total];




	inline AliasSetting* GetPluginAlias(RE::TESFile* plugin)
	{
		if (!plugin)
			return nullptr;

		//From here, til the contains, make that a function.
		std::string file_name(plugin->GetFilename());

		//Use find at a later point, more effecient 

		auto& pluginAliasMap = aliasMap[AliasSet::Plugin];


		if (pluginAliasMap.contains(file_name) == true)
		{
			AliasSetting& node = pluginAliasMap[file_name];

			//node.plugin = plugin;

			return &node;


		}

		return nullptr;
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


	inline AliasData* GetDataFromSet(RE::TESForm* form, uint8_t value, AliasSet set, const MaskType* masks, size_t a_size)
	{
		switch (set)
		{
		default:
			logger::critical("Cannot use.");
			throw nullptr;

			
		case AliasSet::Keyword:
		{
			RE::BGSKeywordForm* keyword_form = skyrim_cast<RE::BGSKeywordForm*>(form);

			if (!keyword_form)
				return nullptr;

			auto& keywordAliasMap = aliasMap[AliasSet::Keyword];


			for (int i = 0; i < keyword_form->numKeywords; i++)
			{
				RE::BGSKeyword* keyword = *keyword_form->GetKeywordAt(i);

				auto str = keyword->formEditorID.c_str();

				auto it = keywordAliasMap.find(str);

				if (keywordAliasMap.end() == it)
					continue;

				AliasSetting* setting = &(it->second);

				if (setting->IsAliasValidSafe(value, masks, a_size) == true)
					return &setting->nodes[value];
			}

			return nullptr;
		}

		case AliasSet::Plugin:
		{
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
			
		}
		break;
		}
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


		for (auto mask : masks)
		{
			if (globalMask[mask].test(value) == false) {
				//logger::debug("Alias for {} not implementing.", (int)alias);
				return false;
			}
		}



		AliasData* data = nullptr;

		for (AliasSet set = AliasSet::Start; set != AliasSet::Total && !data; set++)
		{
			//Later, this needs to take a compiled list of all the exclusions. if I ever get around to doing that.
			data = GetDataFromSet(form, value, set, masks.data(), masks.size());
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