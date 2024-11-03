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

		ExtraValueInfo* extraValue = nullptr;
	};


	ENUM(MaskType)
	{
		kGet,
		kSet,
		//kCost, //To be a cost, something only needs to be able to get and set damage.
		kExclude,
		kTotal,
	};


	inline AliasMask globalMask[MaskType::kTotal];


	struct AliasSetting
	{
		AliasMask localMask[MaskType::kSet + 1];
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

			if (value->AllowsSetting() == true) {
				localMask[MaskType::kSet].set(alias, true);
				globalMask[MaskType::kSet].set(alias, true);
			}


			return true;
		}

		bool IsAliasValidSafe(uint8_t alias, bool requires_set)
		{
			if (!this)
				return false;

			MaskType type = requires_set ? MaskType::kSet : MaskType::kGet;

			return localMask[type].test(alias);
		}
	};





	ENUM(AliasSet)
	{
		//Note for ID related stuff I only need to preserve the ID of stuff I can actually change. Granted, that's a lot.
		//ExactID,
		//Prefix,
		//Suffix,
		//Contains,
		//List,
		Keyword,
		Plugin,
		Total,
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



	inline AliasData* GetDataFromSet(RE::TESForm* form, uint8_t value, MaskType mask, AliasSet set)
	{
		bool require_set = mask == MaskType::kSet;

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

				if (setting->IsAliasValidSafe(value, mask == MaskType::kSet) == true)
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
						RE::TESFile* file = current->masterPtrs[i];

						auto setting = GetPluginAlias(file);

						if (setting->IsAliasValidSafe(value, require_set) == true) {
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
		RequireSetting = 1 << 1,
		//IgnoreKeyword = 1 << 2,
		IgnorePlugin = 1 << 3,//Ignores plugin aliases, aliases like cost would prevent any spell from properly using the base cost.

	};



	//I would like to switch the 2 of these, that way I can make a parameter pack.
	static bool AliasToValue(RE::TESForm* form, UnionValue value, RE::ActorValue& out, QuerySettings settings = QuerySettings::None)
	{
		if (value.IsInvalid() || !form)
			return false;

		//Allows the value of "none" to be changed, but requires some manual placement rule to avoid over checking. Keyword, exact, and list
		// all having the least amount of chance for unintended mishaps with it's use.
		bool allow_none = settings & QuerySettings::AllowNone;
		bool requires_setting = settings & QuerySettings::RequireSetting;

		MaskType mask = MaskType::kGet;

		if (requires_setting)
		{
			mask = MaskType::kSet;
		}
		/*
		if (value.IsActorValue() == false)
		{
			switch (value.GetVirtualValue().value())
			{
			case VirtualValue::kCost:
				mask = MaskType::kCost;
				break;
			}
		}
		//*/

		RE::TESFileArray* source_array = form->sourceFiles.array;

		if (!source_array)
			return false;

		if (!allow_none && value.GetActorValue() == RE::ActorValue::kNone) {
			return false;
		}

		if (globalMask[mask].test(value) == false) {
			//logger::debug("Alias for {} not implementing.", (int)alias);
			return false;
		}



		AliasData* data = nullptr;

		for (AliasSet set = AliasSet::Keyword; set != AliasSet::Total && !data; set++)
		{
			//Later, this needs to take a compiled list of all the exclusions. if I ever get around to doing that.
			data = GetDataFromSet(form, value, mask, set);
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


}