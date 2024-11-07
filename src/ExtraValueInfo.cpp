#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"
#include "ValueAliasHandler.h"


namespace AVG
{
	void ExtraValueInfo::Create(std::string_view name, ExtraValueType type, const FileNode& node, bool legacy)
	{
		std::array<std::unique_ptr<ExtraValueInfo>(*)(), ExtraValueType::Total> factory
		{
			[]() -> std::unique_ptr<ExtraValueInfo> { return std::unique_ptr<ExtraValueInfo>{new AdaptiveValueInfo}; },
			[]() -> std::unique_ptr<ExtraValueInfo> { return {}; },
			[]() -> std::unique_ptr<ExtraValueInfo> { return std::unique_ptr<ExtraValueInfo>{new FunctionalValueInfo}; },
		};

		std::unique_ptr<ExtraValueInfo> info = factory[type]();

		info->valueName = name;

		info->LoadFromFile(node, legacy);

		if (1 == 1)
		{
			AddExtraValueInfo(info.get(), type);

			//If is valid
			info.release();
		}
	}

	void AdaptiveValueInfo::LoadFromFile(const FileNode& node, bool is_legacy)
	{
		auto& table = *node.as_table();

		auto recovery_data = table["regen"];

		std::string rate;
		std::string delay;
		bool fixed;

		bool no_rec = false;

		if (recovery_data.is_table() == false) {//!!recovery_data, this is a double negative, not an operator.
			logger::info("No rec data");
			no_rec = true;
		}
		else {
			rate = recovery_data["rate"].value_or("");
			delay = recovery_data["delay"].value_or("");
			fixed = recovery_data["fixed"].value_or(false);

			logger::info("Rec data rate: {}, delay: {}", rate, delay);
			no_rec = rate == "";
		}

		if (!no_rec)
		{
			_recovery = new RecoveryData();
			_recovery->recoveryDelay = ValueFormula::Create(delay, is_legacy ? legacy : commons);//, "ActorValueGenerator::Commons");
			_recovery->recoveryRate = ValueFormula::Create(rate, is_legacy ? legacy : commons);
		}

		if (auto* rec = GetRecoveryDataSafe(); rec && fixed) {
			rec->flags |= RecoveryData::Fixed;
		}


		auto default_table = table["default"].as_table();

		if (default_table)
		{
			std::string update_formula = (*default_table)["formula"].value_or("");

			if (update_formula != "")
			{
				auto* default_data = MakeDefaultDataSafe();

				NULL_CONDITION(default_data)->defaultFunction = ValueFormula::Create(update_formula, is_legacy ? legacy : commons);


				std::string default_type = (*default_table)["type"].value_or("Implicit");

				switch (RGL::Hash<RGL::HashFlags::Insensitive>(default_type))
				{
				case "Implicit"_ih:
					NULL_CONDITION(default_data)->_type = DefaultData::Implicit;
					break;
				case "Explicit"_ih:
					NULL_CONDITION(default_data)->_type = DefaultData::Explicit;
					break;
				case "Constant"_ih:
					NULL_CONDITION(default_data)->_type = DefaultData::Constant;
					break;
				}
			}

		}


		//Total is now the error alias value.
		RE::ActorValue alias_value = Utility::StringToActorValue(table["alias"].value_or("Total"));

		if (alias_value == RE::ActorValue::kNone)
			logger::warn("The default alias of '{}' cannot be None. Move to an Include List if this is your intent", GetName());

		//Doing like this is temp
		_aliasID = alias_value;//Total means no alias now.

		if (auto plugin_test = table["plugins"].as_array(); Utility::IsValidValue(alias_value) && plugin_test)
		{
			auto& plugins = *plugin_test;

			for (auto& a_node : plugins)
			{
				//At a later point, if this node is an array we do more. But I'm so fucking tired like jesus fucking christ
				// this shit is heavy.

				std::string plugin_name = a_node.value_or("");

				if (plugin_name != "") {
					//logger::info("Adding {} to {} at {}", plugin_name, name, alias_value);
					logger::info("Adding {} to {} at {}", plugin_name, GetName(), (int)alias_value);
					aliasMap[AliasSet::Plugin][plugin_name].AddSetting(alias_value, this, AliasMode::Inherent);
				}
			}

		}


	}







	float AdaptiveValueInfo::GetExtraValue(RE::Actor* target, ExtraValueInput value_types)
	{
		if (!target)
			return 0;//I'd like to return NaN, with some additional information.

		ExtraValueStorage& ev_store = ExtraValueStorage::GetCreateStorage(target);

		return ev_store.GetValue(target, _dataID, value_types, this);


		//Needs to look up EVS, so this shit is just gonna wait for implementation.
	}

	bool AdaptiveValueInfo::SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier)
	{
		if (!target)
			return false;

		ExtraValueStorage& ev_store = ExtraValueStorage::GetCreateStorage(target);

		ExtraValueInput ev_mod = ExtraValueInput::None;

		switch (modifier) 
		{
		case RE::ACTOR_VALUE_MODIFIER::kTotal:
			ev_mod = ExtraValueInput::Base;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kPermanent:
			ev_mod = ExtraValueInput::Permanent;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kTemporary:
			ev_mod = ExtraValueInput::Temporary;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kDamage:
			ev_mod = ExtraValueInput::Damage;
			break;

		default:
			return false;
		}
		
		ev_store.SetValue(target, _dataID, value, ev_mod, this);

		return true;
	}

	bool AdaptiveValueInfo::ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier)
	{
		if (!target)
			return false;

		ExtraValueStorage& ev_store = ExtraValueStorage::GetCreateStorage(target);

		ExtraValueInput ev_mod = ExtraValueInput::None;

		switch (modifier) {
		case RE::ACTOR_VALUE_MODIFIER::kTotal:
			ev_mod = ExtraValueInput::Base;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kPermanent:
			ev_mod = ExtraValueInput::Permanent;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kTemporary:
			ev_mod = ExtraValueInput::Temporary;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kDamage:
			ev_mod = ExtraValueInput::Damage;
			break;

		default:
			return false;
		}

		ev_store.ModValue(target, aggressor, _dataID, value, ev_mod, this);

		return true;
	}




	bool NewInnerCheck(std::string_view mod, const toml::array& entry_array, std::string& out)
	{
		auto size = entry_array.size();
		if (size < 2) {
			//Needs more entries
			return false;
		}

		constexpr std::string_view legacy_set = "AffectActorValue";

		if (strnicmp(legacy_set.data(), entry_array[0].value_or(""), legacy_set.size()) != 0)
		{
			//Only AffectActorValue is supported
			return false;
		}

		for (auto& entry : entry_array)
		{
			if (entry.is_string() == false)
			{
				//entry(X) isn't a string
				return false;
			}
		}


		std::string result = std::format("ModActorValue( '{}', ", entry_array[1].value_or(""));
		
		result += 'Base';

		result += ", (to - from)";



		if (size >= 3)
		{
			result += std::format(" * {}", entry_array[2].value_or("1"));
		}

		result += " )";

		out = result;

		return true;
	}


	std::string NewOuterCheck(std::string_view mod, const toml::array& entry_array)
	{
		std::string result;

		bool expect_string = true;

		int encountered_string = 0;

		for (size_t i = 0; i < entry_array.size(); i++)
		{
			auto& entry = entry_array[i];
			auto entry_type = entry.type();

			if (entry_type == toml::node_type::array)
			{

				std::string buf;
				if (NewInnerCheck(mod, *entry.as_array(), buf) == true)
				{
					if (!expect_string) {
						result += " => " + buf;
					}
					else
					{
						result = buf;
					}
					//do something with buffer.
				}

				expect_string = false;
			}
			else if (entry_type == toml::node_type::string)
			{
				if (!expect_string) {
					//log something in error
					continue;
				}


				encountered_string++;
			}


		}

		if (expect_string)
		{
			if (NewInnerCheck(mod, entry_array, result) == true)
			{
				//say something
			}
		}

		return result;
	};


	void HandleSetNode(FileView entry, std::string_view modifier, std::string& out, bool legacy)
	{

		//Do this regardless of legacy
		if (auto p_test = entry.as_array(); p_test)
		{
			auto& a_array = *p_test;

			

			//Make modifier a string instead, I'll be getting that value anyways so best to just handle it there.
			std::string buffer = NewOuterCheck(modifier, a_array);
			
			
			if (!buffer.empty()) {
				if (legacy){
					out = std::move(buffer);
					logger::debug("Legacy Routine created: {}", out);
				}
				else{
					logger::warn("Set function format no longer supported, use single line formulas instead. Formats to:\n{}", buffer);
				}

			}
		}
		else if (entry.is_string() == true)
		{
			out = entry.value_or("");
		}
	}




	void FunctionalValueInfo::LoadFromFile(const FileNode& node, bool is_legacy)
	{

		auto& table = *node.as_table();

		ModifierArray<std::string> get_strings{};
		ModifierArray<std::string> set_strings{};

		auto get_info = table["get"];
		auto set_info = table["set"];

		auto node_type_G = get_info.type();
		auto node_type_S = set_info.type();

		using NodeType = decltype(node_type_G);

		bool is_readonly = false;



		switch (node_type_G)
		{
		case NodeType::table:
		{
			auto& get_table = *get_info.as_table();

			get_strings[ActorValueModifier::kTotal] = get_table["base"].value_or("");
			get_strings[ActorValueModifier::kPermanent] = get_table["permanent"].value_or("");
			get_strings[ActorValueModifier::kTemporary] = get_table["temporary"].value_or("");
			get_strings[ActorValueModifier::kDamage] = get_table["damage"].value_or("");

		}
		break;

		case NodeType::string:
		{
			//Is readonly now.
			auto& get_string = *get_info.as_string();

			get_strings[ActorValueModifier::kTotal] = get_string.get();

			is_readonly = true;
		}
		break;
		}

		//Needs to not do stuff if it's readonly (IE there's nothing to check the get values of other modifiers.
		switch (node_type_S)
		{
		case NodeType::table:
		{
			auto& set_table = *set_info.as_table();

			HandleSetNode(set_table["base"], "'base'", set_strings[ActorValueModifier::kTotal], is_legacy);
			HandleSetNode(set_table["permanent"], "'permanent'", set_strings[ActorValueModifier::kPermanent], is_legacy);
			HandleSetNode(set_table["temporary"], "'temporary'", set_strings[ActorValueModifier::kTemporary], is_legacy);
			HandleSetNode(set_table["damage"], "'damage'", set_strings[ActorValueModifier::kDamage], is_legacy);
		}
		break;

		case NodeType::string:
		{
			auto& set_string = *set_info.as_string();

			set_strings[ActorValueModifier::kTotal] = set_string.get();
		}
		break;

		case NodeType::array:
			HandleSetNode(set_info, "'base'", set_strings[ActorValueModifier::kTotal], is_legacy);
			break;
		}


		
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.

			//Make these a function.
			//RGL::Incl::operator++
			
			for (ActorValueModifier mod = ActorValueModifier::kPermanent; mod <= ActorValueModifier::kTotal; mod++)
			{
				if (get_strings[mod] != "") {
					logger::debug("making {}", get_strings[mod]);
					_get[mod] = ValueFormula::Create(get_strings[mod], is_legacy ? legacy : commons);
					_getFlags |= ModifierToValueInput(mod);
				}

				if (mod == ActorValueModifier::kTotal)
					break;
			}

			for (ActorValueModifier mod = ActorValueModifier::kPermanent; mod <= ActorValueModifier::kTotal; mod++)
			{
				if (set_strings[mod] == "")
					continue;

				
				//For there to be a set there, there must be a coresponding get function
				if (!(ModifierToValueInput(mod) & _getFlags)) {
					logger::info("corresponding get function must exist for set function.");//state which are in error
					continue;
				}
				
				_set[mod] = SetFormula::Create("cause", "from", "to", set_strings[mod], is_legacy ? legacy : commons);

				_setFlags |= ModifierToValueInput(mod);


			}
		}


		logger::info("Functional value {} created. Get: {:04B}, Set: {:04B}", GetName(), (int)GetFlags(), (int)SetFlags());

		RE::ActorValue alias_value = Utility::StringToActorValue(table["alias"].value_or("Total"));


		if (alias_value == RE::ActorValue::kNone)
			logger::warn("The default alias of '{}' cannot be None. Move to an Include List if this is your intent", GetName());


		//Doing like this is temp
		_aliasID = alias_value;

		if (auto plugin_test = table["plugins"].as_array(); Utility::IsValidValue(alias_value) && plugin_test)
		{
			auto& plugins = *plugin_test;

			for (auto& a_node : plugins)
			{
				//At a later point, if this node is an array we do more. But I'm so fucking tired like jesus fucking christ
				// this shit is heavy.

				std::string plugin_name = a_node.value_or("");

				if (plugin_name != "") {
					logger::info("Adding {} to {} at {}", plugin_name, GetName(), (int)alias_value);

					aliasMap[AliasSet::Plugin][plugin_name].AddSetting(alias_value, this, AliasMode::Inherent);

					//info->AddToMasterFiles(plugin_name);
				}
			}

		}

		return;
	}
}