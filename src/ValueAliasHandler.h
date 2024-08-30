#pragma once

#include "ExtraValueInfo.h"




namespace AVG
{
	enum struct AliasSettingFlag : uint8_t
	{
		None = 0,
		
		LastAppliesNonDerived = 1 << 0,//This is kinda the only one worth doing right now.
		//This isn't one I'll be using yet, but force use alias, but force use alias will force a plugin to apply aliases to all
		// forms in its plugin. (EX. 1) Actually, Im not sure if the idea works.
		//ForceUseAlias = 1 << 1,//If this goes how I think it will,
		Exclude = 0xFF//Exclude removes all aliases from consideration when a plugin (or form if I can set that up) is processed.
	};


	//This is supposed to be AV::kTotal / 8 ceiled, but that's not a constexpr.
	constexpr size_t AliasByteRequirement = 21;



	//EX. 1: So, say I make an alias of variable01, but later use Variable02 in an effect. Then, another plugin overrides this to use
	// variable01. Well, normally, since that plugin has no say here it would remain variable one. BUT when defined with this flag it will
	// consider it, despite the fact
	struct AliasFlag
	{
		uint8_t index{};
		uint8_t alias_bit{};

		RE::ActorValue GetActorValue() const
		{
			if (index == AliasByteRequirement && alias_bit == 0b10000000)
				return RE::ActorValue::kNone;


			int base = index * 8;

			switch (alias_bit)
			{
			case 0b00000001:
				base += 1;
				break;

			case 0b00000010:
				base += 2;
				break;

			case 0b00000100:
				base += 3;
				break;

			case 0b00001000:
				base += 4;
				break;

			case 0b00010000:
				base += 5;
				break;

			case 0b00100000:
				base += 6;
				break;

			case 0b01000000:
				base += 7;
				break;

			case 0b10000000:
				base += 8;
				break;
			}

			base -= 1;

			return static_cast<RE::ActorValue>(base);
		}

		constexpr AliasFlag() = default;
		//*
		AliasFlag(RE::ActorValue alias) :
			index(alias == RE::ActorValue::kNone ? AliasByteRequirement - 1 : (uint32_t(alias) / 8)),
			alias_bit(alias == RE::ActorValue::kNone ? 0b10000000 : (1 << uint32_t(alias) % 8))
		{
			//logger::debug("Made of {} -> {} / {:08B}", (int)alias, index, alias_bit);
		}



		AliasFlag& operator=(RE::ActorValue alias)
		{
			uint32_t alias_num = static_cast<int32_t>(alias);
			bool none = alias_num == -1;
			index = none ? AliasByteRequirement - 1 : int(alias_num / 8);

			alias_bit = none ? 0b10000000 : (1 << alias_num % 8);

			//logger::info("debug of {} -> {} / {:08B}", (int)alias, index, alias_bit);
		}
	};



	struct AliasSetting
	{
		//It's invalid if it looks like this.
		ExtraValueType _type = ExtraValueType::Total;
		ValueID _offset = ExtraValueInfo::FunctionalID;
		std::string _name;//Might be more accurate.
		
		ExtraValueInfo* extraValue;
		AliasSettingFlag flag;

		operator RE::ActorValue() const
		{
			ValueID base = _offset;

			if (_type != ExtraValueType::Total)
				base += 165 + ExtraValueInfo::GetBeginIndex(_type);

			return static_cast<RE::ActorValue>(base);
		}

		operator bool() const { return extraValue != nullptr; }
		//operator bool() const { return _offset != ExtraValueInfo::FunctionalID; }
		
		AliasSetting() = default;
		AliasSetting(ExtraValueInfo* ev, AliasSettingFlag f) : extraValue(ev), flag(f) {}//Can redesign this to get the offset.
		AliasSetting(ExtraValueType t, ValueID o, AliasSettingFlag f) : _type(t), _offset(o), flag(f) {}
		AliasSetting(ExtraValueType t, ValueID o, std::string n, AliasSettingFlag f) : _type(t), _offset(o), _name{n}, flag(f) {}
	};
	
	/*//This is a different version that I'm unsure if I wish to do at this moment.
	class AliasSetting
	{
	private:
		mutable ExtraValueInfo* _info;
		mutable std::string _name;
	public:
		AliasSettingFlag flag;

		ExtraValueInfo* GetInfo() const
		{
			if (_info)
				return _info;

			if (_name == "")
				return nullptr;

			_info = ExtraValueInfo::GetValueInfoByName(_name);


			if (!_info)
				_name = "";

			return _info;

		}

		operator bool() const { return GetInfo() != nullptr; }
		AliasSetting() = default;
		AliasSetting(ExtraValueInfo* ev, AliasSettingFlag f) : _info(ev), flag(f) {}
		AliasSetting(std::string ev_name, AliasSettingFlag f) : _name(ev_name), flag(f) {}
	};
	//*/

	struct AliasMask : public std::array<uint8_t, AliasByteRequirement>
	{
		bool HasAlias(AliasFlag flag)
		{
			bool result =  operator[](flag.index) & flag.alias_bit;

			//logger::debug("TEST has alias {} {:B} => {}", flag.index, flag.alias_bit, result);

			return result;
		}
	};

	ENUM(AliasType)
	{
		GetMask,
		SetMask,
		CostMask,
		Total
	};
	
	//probably wont use.
	ENUM(SpecialAlias)
	{
		None,
		Cost,
	};


	inline AliasMask globalAliases[AliasType::Total];

	inline AliasMask allAliases;
	//inline AliasMask allCostAliases;
	inline AliasMask allSetAliases;

	struct PluginAliasNode
	{
		//don't really need that bit do I?
		//std::string plugin_name;
		//ExtraValueInfo* avAliasArray[RE::ActorValue::kTotal];
		//RE::TESFile* plugin = nullptr;//This isn't required
		
		//AliasMask costAliases{};
		AliasMask filledAliases{};
		AliasMask setableAliases{};

		AliasMask localAliases[AliasType::Total];

		template <class T, class K, class V>
		using map_of_maps = std::unordered_map<T, std::unordered_map<K, V>>;

		map_of_maps<SpecialAlias, RE::ActorValue, AliasSetting> aliasDataMap;//I'm running out of names, I'm a shit programmer.
		std::unordered_map<RE::ActorValue, AliasSetting>& actorValueAliases = aliasDataMap[SpecialAlias::None];



		//Used to be std::map, kept crashing so now it's unordered_map. No fucking clue why but it's probably for the better.
		//std::unordered_map<RE::ActorValue, AliasSetting> actorValueAliases{};

		bool AddSettingTest(RE::ActorValue alias, ExtraValueType type, ValueID offset, std::string name, ExtraValueInfo* info, 
			SpecialAlias spec_type = SpecialAlias::None, AliasSettingFlag flags = AliasSettingFlag::None)
		{

			if (alias == RE::ActorValue::kTotal) {
				logger::error("Alias type invalid.");
				return false;
			}
			AliasSetting& setting = aliasDataMap[spec_type][alias];
			//AliasSetting& setting = actorValueAliases[alias];//  alias == RE::ActorValue::kNone ? actorValueAliases[RE::ActorValue::kAggression] : actorValueAliases[alias];

			if (setting) {
				logger::warn("Value Alias {} for {} is being overriden by {}", (int)alias, setting._name, name);
			}
			//logger::info("address {:X}", (uint64_t)this);

			setting = AliasSetting(type, offset, name, flags);

			//aliasDataMap[spec_type][alias] = setting;

			AliasFlag flag(alias);


			if (spec_type == SpecialAlias::None)
			{
				localAliases[AliasType::GetMask][flag.index] |= flag.alias_bit;
				globalAliases[AliasType::GetMask][flag.index] |= flag.alias_bit;

				if (!info || info->AllowsSetting() == true) {
					localAliases[AliasType::SetMask][flag.index] |= flag.alias_bit;
					globalAliases[AliasType::SetMask][flag.index] |= flag.alias_bit;
				}

			}
			else
			{
				if (!info || info->AllowsSetting() == true) {
					localAliases[AliasType::CostMask][flag.index] |= flag.alias_bit;
					globalAliases[AliasType::CostMask][flag.index] |= flag.alias_bit;
				}

			}
			
			logger::debug("testing alias bit for {}, index: {}, bit: {:B}", name, flag.index, flag.alias_bit);



			return true;
		}

		bool AddSetting_(RE::ActorValue alias, ExtraValueInfo* value, AliasSettingFlag flags, SpecialAlias spec_type = SpecialAlias::None)
		{
			return AddSettingTest(alias, value->GetType(), value->GetOffset(), value->GetName(), value, spec_type, flags);
		}

		bool AddSetting(RE::ActorValue alias, ExtraValueInfo* value, AliasSettingFlag flags, SpecialAlias spec_type = SpecialAlias::None)
		{
			//if (alias == RE::ActorValue::kNone) {
			//	logger::error("Alias cannot be None.");
			//	return false;
			//}

			if (alias == RE::ActorValue::kTotal) {
				logger::error("Alias type invalid.");
				return false;
			}
			AliasSetting& setting = aliasDataMap[spec_type][alias];
			//AliasSetting& setting = actorValueAliases[alias];//  alias == RE::ActorValue::kNone ? actorValueAliases[RE::ActorValue::kAggression] : actorValueAliases[alias];

			if (setting) {
				logger::warn("Value Alias {} for {} is being overriden by {}", (int)alias, setting.extraValue->GetName(), value->GetName());
			}
			//logger::info("address {:X}", (uint64_t)this);

			setting = AliasSetting(value, flags);
			
			//aliasDataMap[spec_type][alias] = setting;

			AliasFlag flag(alias);


			if (spec_type == SpecialAlias::None)
			{
				localAliases[AliasType::GetMask][flag.index] |= flag.alias_bit;
				globalAliases[AliasType::GetMask][flag.index] |= flag.alias_bit;
				
				if (value->AllowsSetting() == true) {
					localAliases[AliasType::SetMask][flag.index] |= flag.alias_bit;
					globalAliases[AliasType::SetMask][flag.index] |= flag.alias_bit;
				}

			}
			else
			{
				if (value->AllowsSetting() == true) {
					localAliases[AliasType::CostMask][flag.index] |= flag.alias_bit;
					globalAliases[AliasType::CostMask][flag.index] |= flag.alias_bit;
				}
				
			}
			//return;
			/*
			uint32_t alias_num = static_cast<uint32_t>(alias);

			int index = int(alias_num / 8);

			uint32_t alias_bit = (1 << alias_num % 8);

			filledAliases[index] |= alias_bit;

			if (value->AllowsSetting() == true)
				setableAliases[index] |= alias_bit;

			allAliases[index] |= alias_bit;

			logger::info("testing alias bit for {}, index: {}, bit: {:B}", value->GetName(), index, alias_bit);

			/*/

			
			//*/
			
			filledAliases[flag.index] |= flag.alias_bit;

			if (value->AllowsSetting() == true) {
				setableAliases[flag.index] |= flag.alias_bit;
				allSetAliases[flag.index] |= flag.alias_bit;
			}
			allAliases[flag.index] |= flag.alias_bit;

			logger::debug("testing alias bit for {}, index: {}, bit: {:B}", value->GetName(), flag.index, flag.alias_bit);



			return true;
		}


		bool IsAliasValidSafe(RE::ActorValue alias, bool requires_set)
		{
			if (!this)
				return false;
			
			AliasFlag flag(alias);

			AliasType type = requires_set ? AliasType::SetMask : AliasType::GetMask;

			if (localAliases[type].HasAlias(flag) == false)
				return false;

			//if (filledAliases.HasAlias(flag) == false)
			//	return false;

			//if (requires_set && setableAliases.HasAlias(flag) == false)
			//	return false;

			return true;
		}
	};

	

	inline std::map<std::string, PluginAliasNode> pluginAliasMap;
	inline std::map<std::string, PluginAliasNode> keywordAliasMap;



	PluginAliasNode* GetPluginAlias(RE::TESFile* plugin)
	{
		RE::Actor* a = nullptr;

		if (!plugin)
			return nullptr;

		//From here, til the contains, make that a function.
		std::string file_name(plugin->GetFilename());

		//Use find at a later point, more effecient 

		if (pluginAliasMap.contains(file_name) == true)
		{
			PluginAliasNode& node = pluginAliasMap[file_name];

			//node.plugin = plugin;

			return &node;
			

		}

		return nullptr;
	}

	enum AliasPriority : uint8_t
	{
		Source,
		Last, 
		Current,
		Total
		//Excluded = 1 << 7
	};

	struct AliasPriorityList
	{
		std::array<PluginAliasNode*, AliasPriority::Total> aliases{nullptr, nullptr, nullptr };


		PluginAliasNode* GetPrimaryNode()
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

		AliasPriorityList(RE::TESFileArray* source_array, RE::ActorValue alias, bool requires_set)
		{
			if (!source_array)
				return;

			AliasFlag flag(alias);

			//if (allAliases.HasAlias(flag) == false) {
				//logger::info("Alias for {} not implementing.", (int)alias);
			//	return;
			//}
			
			RE::TESFile** sourceFiles = source_array->data();
			int end = source_array->size() - 1;

			RE::TESFile* current = sourceFiles[0];
			RE::TESFile* source = sourceFiles[end];
			RE::TESFile* last = nullptr;


			if (PluginAliasNode* node = GetPluginAlias(source); node->IsAliasValidSafe(alias, requires_set) == true) {
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

				if (PluginAliasNode* node = GetPluginAlias(last);  node->IsAliasValidSafe(alias, requires_set) == true) {
					aliases[AliasPriority::Last] = node;
					break;
				}
			}
			

			if (last == current) {
				aliases[AliasPriority::Current] = aliases[AliasPriority::Last];
				return;
			}

			if (PluginAliasNode* node = GetPluginAlias(current); node) {
				if (node->IsAliasValidSafe(alias, requires_set) == true) {
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


	inline PluginAliasNode* GetNode(RE::TESFileArray* source_array, RE::ActorValue alias, bool require_set)
	{
		AliasPriorityList list(source_array, alias, require_set);
		return list.GetPrimaryNode();
	}


	inline PluginAliasNode* GetKeywordNode(RE::BGSKeywordForm* keyword_form, RE::ActorValue alias, bool require_set)
	{
		if (!keyword_form)
			return nullptr;

		for (int i = 0; i < keyword_form->numKeywords; i++)
		{
			RE::BGSKeyword* keyword = *keyword_form->GetKeywordAt(i);
			
			std::string str = keyword->formEditorID.c_str();

			auto it = keywordAliasMap.find(str);

			if (keywordAliasMap.end() == it)
				continue;

			PluginAliasNode* node = &(it->second);

			if (node->IsAliasValidSafe(alias, require_set) == true)
				return node;
		}

		return nullptr;
	}
	
	ENUM(AliasQuerySettings)
	{
		None = 0,
		AllowNone = 1 << 0,
		RequireSetting = 1 << 1,
		RequireCost = 1 << 2,
		IgnoreKeyword = 1 << 3,
		IgnorePlugin = 1 << 4,

	};

	struct ValueAliasHandler
	{
		//I want this to have 2 forms, one that takes keyword forms, then one that takes regular forms. OR, make it a concept so
		// I can switch it.


		//I would like to switch the 2 of these, that way I can make a parameter pack.
		static RE::ActorValue AliasToValue(RE::ActorValue alias, RE::TESForm* form, int& change_num, AliasQuerySettings settings)
		{
			bool requires_setting = settings & AliasQuerySettings::RequireSetting;
			bool allow_none = settings & AliasQuerySettings::AllowNone;

			bool ignore_keyword = settings & AliasQuerySettings::IgnoreKeyword;
			bool ignore_plugin = settings & AliasQuerySettings::IgnorePlugin;

			AliasType mask_type = AliasType::GetMask;
			SpecialAlias spec_type = SpecialAlias::None;
			cycle_switch(settings)
			{
				case AliasQuerySettings::RequireSetting:
					mask_type = AliasType::SetMask;
					goto break_settings;

				case AliasQuerySettings::RequireCost:
					mask_type = AliasType::CostMask;
					spec_type = SpecialAlias::Cost;
					goto break_settings;
			}

			if (!form)
				return alias;

			RE::TESFileArray* source_array = form->sourceFiles.array;

			if (!source_array)
				return alias;

			if (!allow_none && alias == RE::ActorValue::kNone)
				return alias;
			
			AliasFlag flag = alias;

			if (globalAliases[mask_type].HasAlias(flag) == false) {
				//logger::debug("Alias for {} not implementing.", (int)alias);
				
				return alias;
			}
			

			//Move back to the consideration, I change my mind, this has no place here.
			if (allAliases.HasAlias(flag) == false) {
				//if (alias == RE::ActorValue::kNone)
				//	logger::info("Alias for {} not implementing.", (int)alias);
				////return alias;
			}

			if (requires_setting && allSetAliases.HasAlias(flag) == false) {
				////logger::warn("Alias for {} not able to be set.", (int)alias);
				////return alias;
			}
			//logger::info("D");

			PluginAliasNode* plugin_alias = nullptr;
			
			if (!ignore_keyword)
				plugin_alias = GetKeywordNode(form->As<RE::BGSKeywordForm>(), alias, requires_setting);

			//This is NOT supposed to happen here.
			//if (!plugin_alias)
			//	return alias;


			//logger::info("C {}", plugin_alias != nullptr);
			if (!ignore_plugin && !plugin_alias)
				plugin_alias = GetNode(source_array, alias, requires_setting);

			//For this shit to fix this, a severe issue has taken root. I need to address whatever is determining
			// it "HAS FLAGS" when it comes through. Likely due to the keyword if I had to guess.
			//logger::info("C s {}", plugin_alias->actorValueAliases.size());

			if (!plugin_alias)// || plugin_alias->actorValueAliases.contains(alias) == false)
				return alias;

			//use find here please, it will be easier for me.

			//logger::info("primary found, continuing");
			//logger::info("B {:X}", (uint64_t)plugin_alias);

			

			//I'm doing this because using none crashes the fucking thing. Very cool, much poggers.

			//RE::ActorValue access_alias = alias == RE::ActorValue::kNone ? RE::ActorValue::kTotal : alias;

			//plugin_alias->actorValueAliases[alias];
			//logger::info("afore");

			//AliasSetting& setting = plugin_alias->actorValueAliases[alias];

			//if (alias == RE::ActorValue::kNone)//This is bogus, it will fail, but thats not the point.
			//	info = plugin_alias->actorValueAliases[RE::ActorValue::kAggression].extraValue;
			//else
			//	info = plugin_alias->actorValueAliases[alias].extraValue;

			
			//logger::info("afore {}", (uintptr_t)&setting);

			//ExtraValueInfo* info = setting.extraValue;// = plugin_alias->actorValueAliases[access_alias].extraValue;;


			//I believe this may have returned and started crashing again, SOME how.
			ExtraValueInfo* info = plugin_alias->aliasDataMap[spec_type][alias].extraValue;//->actorValueAliases[alias].extraValue;
			//ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(plugin_alias->aliasDataMap[spec_type][alias]);//->actorValueAliases[alias].extraValue;
			
			//logger::info("A");
			
			if (!info)
				return alias;

			//logger::info("After");


			RE::ActorValue result = info->GetValueIDAsAV();

			change_num++;

		end:
			logger::info("Actor Value {} has been converted into Extra Value {} for {:08X}.", (int)alias, (int)result, form->formID);

			return result;
		}
		
		//This needs a better restriction. this don't cut it.
		template <class... AQS>//requires(std::is_same_v<AQS..., AliasQuerySettings>)
		static inline RE::ActorValue AliasToValue(RE::ActorValue alias, RE::TESForm* form, int& change_num, AQS... aqs)
		{
			std::array<AliasQuerySettings, sizeof...(AQS)> settings{ aqs... };

			AliasQuerySettings combined_settings = AliasQuerySettings::None;

			std::for_each(settings.begin(), settings.end(), [&](auto it) {combined_settings |= it; });

			return AliasToValue(alias, form, change_num, combined_settings);
		}

		static inline RE::ActorValue AliasToValue(RE::ActorValue alias, RE::TESForm* form, int& change_num)
		{
			return AliasToValue(alias, form, change_num, AliasQuerySettings::None);
		}

		template <class... AQS>//requires(std::is_same_v<AQS..., AliasQuerySettings>)
		static inline RE::ActorValue AliasToValue(RE::ActorValue alias, RE::TESForm* form, AQS... aqs)
		{
			int dump = false;
		
			return AliasToValue(alias, form, dump, aqs...);
		}
	};
}