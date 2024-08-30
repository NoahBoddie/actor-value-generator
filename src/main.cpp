
#include <stddef.h>

#include <toml++/toml.h>

#include "ActorValueExtendedList.h"
#include "EventSingleton.h"
#include "Hooks.h"

#include "ValueAliasHandler.h"

#include "Serialization/SerializationTypePlayground.h"

#include "ExtraValue.h"


using namespace RE::BSScript;
using namespace AVG;
using namespace Arthmetic;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;


//This set up should put me on a list.
void temp_NativeFormulaRegister();


namespace {
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        
        
#ifndef NDEBUG
        const auto level = spdlog::level::trace;
#else
        const auto level = spdlog::level::info;
#endif


        log->set_level(level);
        log->flush_on(level);

        spdlog::set_default_logger(std::move(log));
        //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
        spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
    }

    
    void InitializeSerialization() {
        //log::trace("Initializing cosave serialization...");
        //auto* serde = GetSerializationInterface();
        //serde->SetUniqueID(_byteswap_ulong('AVGN'));
        //serde->SetSaveCallback(Sample::HitCounterManager::OnGameSaved);
        //serde->SetRevertCallback(Sample::HitCounterManager::OnRevert);
        //serde->SetLoadCallback(Sample::HitCounterManager::OnGameLoaded);
        //log::trace("Cosave serialization initialized.");

		RGL::MainSerializer::Initialize('AVG');
    }


	void InitializeHooking()
	{
		auto size = ExtraValueInfo::GetCount();

		//ActorValueExtendedList::Create(257 - total);
		ActorValueExtendedList::Create(size + 1);
	}

	//void CreateFunction() {}
	//void CreateProperty() {}

	/*//Use this shit some time plz
	static std::vector<std::string> SplitString(const std::string& str, const std::string delimiter)
	{
		std::string input("geeks\tfor\tgeeks");
		std::vector<std::string> result;
		boost::split(result, input, boost::is_any_of("=>"));

	}
	//*/
	//I didn't want to do this but I'm temporarily at the mercy of this shit.
	std::map<std::string, std::vector<std::string>> includeMap;

	void HandleIncludeLists()
	{
		for (auto& [name, list] : includeMap) {
			bool is_plugin = name.contains(".es");

			auto& alias_node = is_plugin ? pluginAliasMap[name] : keywordAliasMap[name];
			/*
			for (auto& entry : list)
			{
				if (entry == "") {
					logger::warn("Include entry is empty.");
					continue;
				}

				std::vector<std::string> results;

				results = boost::split(results, entry, boost::is_any_of("="));

				std::string ev_name;


				//The list of aliases this will be being set to.
				std::vector<RE::ActorValue> alias_list;

				ExtraValueInfo* info = nullptr;

				std::string av_name;

				SpecialAlias spec_type = SpecialAlias::None;

				ExtraValueType type = ExtraValueType::Total;

				ValueID offset;

				if (results.size() > 1)
				{
					boost::trim(results[0]);
					boost::trim(results[1]);

					av_name = results[0];
					std::string left_str = results[1];
					
					//I want some way to do this.
					switch (hash<true>(left_str))
					{
					//case "RightCost"_ih:	//RightCharge
					//case "LeftCost"_ih:	//LeftCharge
					//case "EnchCost"_ih:	//Right/LeftCharge
					//case "MagicCost"_ih:	//Magicka/Right/LeftCharge
					case "SpellCost"_ih:	//Magicka
					{
						alias_list.push_back(RE::ActorValue::kMagicka);
						spec_type = SpecialAlias::Cost;
						
					}
					break;

					default:
					{
						RE::ActorValue alias_value = Utility::StringToActorValue(left_str);
						
						if (alias_value == RE::ActorValue::kTotal) {
							//Shit is invalid, do not proceed.
							continue;
						}

						if (is_plugin && alias_value == RE::ActorValue::kNone) {
							logger::warn("None is not a viable alias to use on a plugin.");
							continue;
						}

						alias_list.push_back(alias_value);
					}
					break;
					}
					info = ExtraValueInfo::GetValueInfoByName(av_name);

					logger::info("Custom alias for {} detected, using '{}'", results[0], results[1]);
				}
				else
				{
					av_name = boost::trim_copy(entry);

					info = ExtraValueInfo::GetValueInfoByName(av_name);

					if (!info) {
						logger::error("No ExtraValueInfo detected for '{}'", av_name);
						continue;
					}
					
					if (Utility::IsValidValue(info->_aliasID) == true) {
						alias_list.push_back(info->_aliasID);
					}
					else {
						//This shit is invalid
						logger::error("Extra Value {} has no default alias to be used by include list.", av_name);
						continue;
					}
				}

				if (info) {
					av_name = info->GetName();//To be proper in casing.
					offset = info->GetOffset();
					type = info->GetType();
				}
				else if (auto index = Utility::StringToActorValue(av_name); index != RE::ActorValue::kTotal)
				{
					offset = static_cast<ValueID>(index);
				}
				else {
					continue;
				}

				for (auto& alias_entry : alias_list)
				{

					logger::info("Including {} to {} at {}", ev_name, name, (int)alias_entry);
					alias_node.AddSettingTest(alias_entry, type, offset, av_name, info, spec_type, AliasSettingFlag::None);

				}
			}

			/*/
			for (auto& entry : list)
			{
				if (entry == "") {
					logger::warn("Include entry is empty.");
					continue;
				}

				std::vector<std::string> results;

				results = boost::split(results, entry, boost::is_any_of("="));
				
				std::string ev_name;
				
				RE::ActorValue alias_value = RE::ActorValue::kTotal;
				
				if (results.size() > 1)
				{
					boost::trim(results[0]);
					boost::trim(results[1]);
					
					ev_name = results[0];
					//std::string left_str = results[1];

					//I want some way to do this.
					//switch (hash<true>(left_str))
					//{
					//default:
					//}
					alias_value = Utility::StringToActorValue(results[1]);

					if (alias_value == RE::ActorValue::kTotal) {
						//Shit is invalid, do not proceed.
						continue;
					}

					if (is_plugin && alias_value == RE::ActorValue::kNone) {
						logger::warn("None is not a viable alias to use on a plugin.");
						continue;
					}

					logger::info("Custom alias for {} detected, using '{}'", results[0], results[1]);
				}
				else
				{
					ev_name = boost::trim_copy(entry);
				}

				//Do a split right here with trimming.

				ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

				if (!info) {
					logger::error("No ExtraValueInfo detected for '{}'", ev_name);
					continue;
				}

				//Do this version if some other element isn't specified
			
				if (alias_value == RE::ActorValue::kTotal) {
					if (Utility::IsValidValue(info->_aliasID) == true) {
						alias_value = info->_aliasID;
					}
					else {
						//This shit is invalid
						logger::error("Extra Value {} has no default alias to be used by include list.", ev_name);
						continue;
					}
				}
				

				if (1 != 1 && Utility::IsValidValue(alias_value) == false) {
					logger::error("Invalid alias value from '{}'", ev_name);
					continue;
				}

				logger::info("Including {} to {} at {}", ev_name, name, (int)alias_value);

				alias_node.AddSetting(alias_value, info, AliasSettingFlag::None);
			}
			//*/
		}
	}


	std::pair<std::string, std::string> SplitParameterDeclare(const std::string str)
	{
		//Rules.
		//Foist, split at the '=' sign.
		//Second, for the first one, keep going until you hit a space. Between the beginning and that, that is your string.
		//If an equals sign is not present but other characters are found, present an error, the type shall then be "any"
		//After the equals sign, the first non space value is substringed to the very last.

		std::pair<std::string, std::string> result;
		
		constexpr char tab_char = 0x09;
		constexpr char space_char = 0x20;
		auto it = str.begin();
		auto end = str.end();

		int stage = 0;

		while (it != end)
		{
			int push_it = 1;

			switch (stage)
			{
			case 0:
				if (*it == '=') {
					stage++;
					push_it++;
				}
				else if (*it != space_char && *it != tab_char) {
					//logger::info("c {}", *it);
					break;
				}

				result.first = std::string(str.begin(), it);
				
				stage++;

				break;

			case 1:
				if (*it == space_char || *it == tab_char) {
					break;
				}
				else if (*it != '=') {
					logger::error("only acceptable character between name and definition is '='. Found '{}'", *it);
					goto end;
				}

				stage++;

				break;

			case 2:
				if (*it == space_char || *it == tab_char) {
					break;
				}
				
				result.second = std::string(it, end);
				
				return result;
			
			}

			it += push_it;
		}
	
		if (result.first == "") {
			logger::error("Unable to find any value in string '{}'. Failing compile", str);
			throw nullptr;
		}

		logger::error("Unable to find type value in '{}'. Defaulting to any.", str);
	
	end:
		result.second = "any";

		return result;

	}



	void temp_ArrayCheck(FunctionalValueInfo* info, RE::ACTOR_VALUE_MODIFIER mod, const toml::array& entry_array, bool allow_array)
	{
		toml::node_type internal_type = toml::node_type::none;

		std::string name;

		std::vector<std::string> results{};


		for (size_t i = 0; i < entry_array.size(); i++)
		{
			auto entry_type = entry_array[i].type();

			if (internal_type == toml::node_type::none) {
				switch (entry_type)
				{
				case toml::node_type::array:
					if (!allow_array) {
						//arrays not allowed, but that's just one function down, not fatal.
						continue;
					}
					[[fallthrough]];
				case toml::node_type::string:
					internal_type = entry_type;
					break;

				default:
					//Error, wrong type detected. Parser throw?
					// Hard to know what if this is even an array of strings or array of arrays yet. is here, so fatal.
					return;
				}
			}

			if (entry_type != internal_type) {
				//Log error here.
				//std::string str = view.value_or("");
				//info->AddSetFunction("", mod, {});

				//We'll allow this to be a persisting value, but make sure it's known that it's invalid.
				if (internal_type == toml::node_type::string)
					results.push_back(entry_array[i].value_or("invalid"));

				return;
			}

			switch (entry_type)
			{
			case toml::node_type::array:
				temp_ArrayCheck(info, mod, *entry_array[i].as_array(), false);
				break;

			case toml::node_type::string:
				if (!i)
					name = entry_array[i].value_or("");
				else
					results.push_back(entry_array[i].value_or("invalid"));
				break;
			}

		}

		if (internal_type == toml::node_type::string) {
			info->AddSetFunction(name, mod, results);
		}
	};



	void temp_HandleSetBranch(FunctionalValueInfo* info, RE::ACTOR_VALUE_MODIFIER mod,  const FileView view)
	{
		if (!view)
			return;

		logger::debug("A");
		auto node_type = view.type();
		logger::debug("B");
		using NodeType = decltype(node_type);
		/*
		std::function<void(const toml::array&, bool)> array_check = [=](const toml::array& arr_ptr, bool allow_array) -> void
		{
			//if (!arr_ptr)
				return;

			auto& entry_array = *arr_ptr;

			NodeType internal_type = NodeType::none;
			
			std::string name;

			std::vector<std::string> results{};


			for (size_t i = 0; i < entry_array.size(); i++)
			{
				auto entry_type = entry_array[i].type();

				if (internal_type == NodeType::none) {
					switch (entry_type)
					{
					case NodeType::array:
						if (!allow_array) {
							//arrays not allowed, but that's just one function down, not fatal.
							continue;
						}
						[[fallthrough]];
					case NodeType::string:
						internal_type = entry_type;
						break;

					default:
						//Error, wrong type detected. Parser throw?
						// Hard to know what if this is even an array of strings or array of arrays yet. is here, so fatal.
						return;
					}
				}

				if (entry_type != internal_type) {
					//Log error here.
					//std::string str = view.value_or("");
					//info->AddSetFunction("", mod, {});
					
					//We'll allow this to be a persisting value, but make sure it's known that it's invalid.
					if (internal_type == NodeType::string)
						results.push_back(entry_array[i].value_or("invalid"));
					
					return;
				}

				switch (entry_type)
				{
				case NodeType::array:
					array_check(*entry_array[i].as_array(), false);
					break;

				case NodeType::string:
					if (!i)
						name = entry_array[i].value_or("");
					else
						results.push_back(entry_array[i].value_or("invalid"));
					break;
				}

			}

			if (internal_type == NodeType::string){
				info->AddSetFunction(name, mod, results);
			}
		};
		//*/
		logger::debug("C");
		switch (node_type)
		{
		case NodeType::string:
			info->AddSetFunction(view.value_or(""), mod, {});
			break;

		case NodeType::array:
			temp_ArrayCheck(info, mod, *view.as_array(), true);
			break;
		}
		logger::debug("D");
	}

	void HandleFileInput(std::string& name, const FileNode& node)
	{
		//Doesn't work, seems to only get testEV, maybe my choice isn't entirely set up properly?


		//SO if something is an array of tables (It can check which is nice) then it will check the name for composite types.
		// Otherwise, what we'll do is check if there is a type value within, and if not, this key is not valid.



		if (true != true && node.is_array_of_tables() == true)
		{
			//Ignore this for right now.

			switch (Arthmetic::hash<true>(name))
			{
			case "Properties"_ih:
				for (auto& entry : *node.as_array())
				{
					if (entry.is_table() == false) {
						//'{}' isn't a table under a table.
						continue;
					}

					for (auto& [key, entry] : *entry.as_table())
					{
						std::string property_name = key.data();
						std::string value = entry.value_or("");

						//Don't declare the same prop twice. No guarding right now.
						if (value == "")
							continue;
						Property* new_prop = new Property(value);
						
						new_prop->DeclareOwner();


						propertyMap[property_name] = new_prop;
					}
				}

				return;

			
			default:
				//composite type not recognized not recognized.
				logger::error("Composite type, '{}' isn't recognized.", name);
				return;
			}
		}
		else if (auto test = node.as_table(); test)
		{
			auto& table = *test;

			//If it's one of these types. Later, it's basically if it's a reserved name or not.
			switch (Arthmetic::hash<true>(name))
			{

			case "Include"_ih:
				for (auto& [key, list] : table) {
					std::string name = key.data();//Should split this properly, SOME how.

					if (list.is_array() == false) {
						//'{}' isn't a table under a table.
						logger::error("Include list for '{}' is not an array. Discarding entry.", name);
						continue;
					}

					auto& include_list = includeMap[name];

					logger::info("Including for '{}'", name);

					for (auto& entry : *list.as_array())
					{
						std::string include_name = entry.value_or("");

						if (include_name == "") {
							logger::error("Non-string entry found in include list '{}'.", name);
							continue;
						}

						logger::info("{}: {}", name, include_name);
						include_list.push_back(include_name);
					}

					include_list.shrink_to_fit();
				}

				return;

			case "Properties"_ih:
				for (auto& [key, entry] : table)
				{
					std::string property_name = key.data();
					std::string value = entry.value_or("");

					//Don't declare the same prop twice. No guarding right now.
					if (value == "")
						continue;
					Property* new_prop = new Property(value);

					new_prop->DeclareOwner();


					propertyMap[property_name] = new_prop;
				}
				
				return;
			}



			std::string type = table["type"].value_or("Invalid");

			switch (Arthmetic::hash<true>(type))
			{
			case "Routine"_ih:
			{
				//This will need to have some way to parse which one, coroutine or function iterface 
				// that needs to be created. I think a flag called native is what I'll do.
				// Settings are case sensitive, strings arent.
				//CreateFormula(key);

				//ParsingArgumentList
				//What you're gonna wanna do first is look for parameters. This is what we cycle through looking for parameters.
				//For delegate parameters I'm going to just manually set, I would like to submit a sorta null str
			
				std::string formula_code = table["formula"].value_or("0");

				//table->size()//I'm going to resize to this or it's size if it's valid.
				ParsingArgumentList parameters{ };

				//alternate names could be parameters/param/parameter.
				// If param or parameter is detected it will only accept one.
				auto param_view = table["params"];

				/*
				if (auto p_test = param_view.as_table(); p_test)
				{
					auto& param_table = *p_test;

					for (auto& [key, entry] : param_table)
					{
						std::string identifier = key.data();
						std::string type_value = entry.value_or("any");

						parameters.push_back(std::make_pair(identifier, type_value));
					}

				}
				//*/
				if (auto p_test = param_view.as_array(); p_test)
				{
					auto& param_array = *p_test;
					int i = 0;

					for (auto& entry : param_array)
					{
						if (entry.is_string() == false) {
							logger::critical("Non string entry found in entry {}.", i);
							throw nullptr;
						}
						
						i++;

						std::string str = entry.value_or("");

						parameters.push_back(SplitParameterDeclare(str));
					}

				}

				//This should be made in the function at some point
				IFormula* formula = nullptr;//parameters.size() ? (IFormula*)CreateFormula(formula_code, parameters) : (IFormula*)CreateSubroutine(formula_code);
					
				//I wish to handle this differently later, iterating and comparing for a sheet of flags instead of this.			
				auto* flag_array = table["flags"].as_array();

				if (flag_array)
				{
					auto result = std::find_if(flag_array->cbegin(), flag_array->cend(), [](auto& it) {return it.value_or("") == "native"; });
					
					if (flag_array->end() == result)
						formula = CreateFormula(parameters);
				
				}

				if (!formula)
				{
					formula = parameters.size() ? (IFormula*)CreateFormula(formula_code, parameters) : (IFormula*)CreateSubroutine(formula_code);
				}

				formula->defaultValue = table["default"].value_or(0);
				
				formulaMap[name] = formula;
				
				return;
			}


			//Combine functional and adaptive creation, and give them both their means of production.
			case "Adaptive"_ih:
			{
				AdaptiveValueInfo* info = nullptr;

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

				if (no_rec)
					info = new AdaptiveValueInfo(name);
				else
					info = new AdaptiveValueInfo(name, delay, rate);
				
				if (auto* rec = info->GetRecoveryDataSafe(); rec && fixed) {
					rec->flags |= RecoveryData::Fixed;
				}


				auto default_table = table["default"].as_table();

				if (default_table)
				{
					std::string update_formula = (*default_table)["formula"].value_or("");

					if (update_formula != "")
					{
						auto* default_data = info->MakeDefaultDataSafe();

						NULL_CONDITION(default_data)->defaultFunction = CreateSubroutine(update_formula);


						std::string default_type = (*default_table)["type"].value_or("Implicit");

						switch (Arthmetic::hash<true>(default_type))
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
					logger::warn("The default alias of '{}' cannot be None. Move to an Include List if this is your intent", name);

				//Doing like this is temp
				info->_aliasID = alias_value;//Total means no alias now.

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
							logger::info("Adding {} to {} at {}", plugin_name, name, (int)alias_value);
							pluginAliasMap[plugin_name].AddSetting(alias_value, info, AliasSettingFlag::None);
							
							//info->AddToMasterFiles(plugin_name);
						}
					}

				}

				return;
			}


			case "Functional"_ih:
			{
				//Functional not implemented yet.	
				//To have this work, I need the thing that allows you to create via string literals.
				FunctionalValueInfo* info = nullptr;

				ModifierArray<std::string> get_strings{};
				ModifierArray<std::string> set_strings{};

				auto get_info = table["get"];
				auto set_info = table["set"];

				auto node_type_G = get_info.type();
				auto node_type_S = set_info.type();

				using NodeType = decltype(node_type_G);

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
					auto& get_string = *get_info.as_string();

					get_strings[ActorValueModifier::kTotal] = get_string.get();
				}
				break;
				}
				/*
				switch (node_type_S)
				{
				case NodeType::table:
				{
					auto& set_table = *set_info.as_table();

					set_strings[ActorValueModifier::kTotal] = set_table["base"].value_or("");
					set_strings[ActorValueModifier::kPermanent] = set_table["permanent"].value_or("");
					set_strings[ActorValueModifier::kTemporary] = set_table["temporary"].value_or("");
					set_strings[ActorValueModifier::kDamage] = set_table["damage"].value_or("");
				}
				break;

				case NodeType::string:
				{
					auto& set_string = *set_info.as_string();

					set_strings[ActorValueModifier::kTotal] = set_string.get();
				}
				break;
				}
				//*/
				info = new FunctionalValueInfo(name, get_strings, set_strings);

				switch (node_type_S)
				{
				case NodeType::table:
				{
					auto& set_table = *set_info.as_table();
					logger::debug("1");
					temp_HandleSetBranch(info, ActorValueModifier::kTotal, set_table["base"]);
					logger::debug("2");
					temp_HandleSetBranch(info, ActorValueModifier::kPermanent, set_table["permanent"]);
					logger::debug("3");
					temp_HandleSetBranch(info, ActorValueModifier::kTemporary, set_table["temporary"]);
					logger::debug("4");
					temp_HandleSetBranch(info, ActorValueModifier::kDamage, set_table["damage"]);
					logger::debug("5");
				}
				break;

				case NodeType::string:
				{
					temp_HandleSetBranch(info, ActorValueModifier::kTotal, set_info);
				}
				break;
				}

				logger::info("Functional value {} created. Get: {:04B}, Set: {:04B}", info->GetName(), (int)info->GetFlags(), (int)info->SetFlags());

				RE::ActorValue alias_value = Utility::StringToActorValue(table["alias"].value_or("Total"));


				if (alias_value == RE::ActorValue::kNone)
					logger::warn("The default alias of '{}' cannot be None. Move to an Include List if this is your intent", name);


				//Doing like this is temp
				info->_aliasID = alias_value;

				if (auto plugin_test = table["plugins"].as_array(); Utility::IsValidValue(alias_value) && plugin_test)
				{
					auto& plugins = *plugin_test;

					for (auto& a_node : plugins)
					{
						//At a later point, if this node is an array we do more. But I'm so fucking tired like jesus fucking christ
						// this shit is heavy.

						std::string plugin_name = a_node.value_or("");

						if (plugin_name != "") {
							logger::info("Adding {} to {} at {}", plugin_name, name, (int)alias_value);

							pluginAliasMap[plugin_name].AddSetting(alias_value, info, AliasSettingFlag::None);

							//info->AddToMasterFiles(plugin_name);
						}
					}

				}

				return;
			}


			case "Invalid"_ih:
				//Invalid type '{}' not loaded.
				return;

			default:
				//Type not recognized.
				logger::error("Not valid type {} for {}", type, name);

				return;
			}
		}
		else
		{
		logger::error("{} is neither single nor composite type.", name);
		}

	}
	
	//SHAMELESSLY Ripped directly from po3's clib utils
	inline std::vector<std::string> get_configs(std::string_view a_folder, std::string_view a_suffix, std::string_view a_extension = ".ini"sv)
	{
		std::vector<std::string> configs{};

		for (const auto iterator = std::filesystem::directory_iterator(a_folder); const auto & entry : iterator) {
			if (entry.exists()) {
				if (const auto& path = entry.path(); !path.empty() && path.extension() == a_extension) {
					if (const auto& fileName = entry.path().string(); fileName.rfind(a_suffix) != std::string::npos) {
						configs.push_back(fileName);
					}
				}
			}
		}

		std::ranges::sort(configs);

		return configs;
	}

    

	static bool LoadFile(const std::string a_path)
	{
		try {
            //I wonder if this closes.
			const auto table = toml::parse_file(a_path);

			for (auto& [key, entry] : table) {
				std::string name = key.data();  //This may not be null terminated, may want to switch over to string views at some point.
				logger::info("Starting: {}-------------", name);
				HandleFileInput(name, entry);
			}

		} catch (const toml::parse_error& e) {
			//For now, I'm just gonna take the L
			/*
			std::ostringstream ss;
			ss
				<< "Error parsing file \'" << *e.source().path
				<< "\':\n"
				<< e.description()
				<< "\n  (" << e.source().begin << ")\n";
			logger::error(fmt::runtime(ss.str()));
			//*/

			logger::error("Parsing failed: {}", e.description());
			return false;
		}

		return true;
	}

	
	void PersonalLoad()
	{
		constexpr std::string_view core_path = "Data/SKSE/Plugins/ActorValueData";
		std::vector<std::string> configs = get_configs(core_path, "_AVG", ".toml");

		//Should register itself. Note, I really would like this to be a function instead of this on the outside doing this.
	
		//std::string ev_name = "HitsTaken";
		//new AdaptiveValueInfo(ev_name);
		logger::info("Config Count: {}", configs.size());

		for (auto& file_name : configs){
			LoadFile(file_name);
		}

		temp_NativeFormulaRegister();

		logger::info("Finishing the manifest. . .");
		ExtraValueInfo::FinishManifest();
		HandleIncludeLists();
		Hooks::Install();
		EventSingleton::Install();
	}




    /**
     * Register to listen for messages.
     *
     * <p>
     * SKSE has a messaging system to allow for loosely coupled messaging. This means you don't need to know about or
     * link with a message sender to receive their messages. SKSE itself will send messages for common Skyrim lifecycle
     * events, such as when SKSE plugins are done loading, or when all ESP plugins are loaded.
     * </p>
     *
     * <p>
     * Here we register a listener for SKSE itself (because we have not specified a message source). Plugins can send
     * their own messages that other plugins can listen for as well, although that is not demonstrated in this example
     * and is not common.
     * </p>
     *
     * <p>
     * The data included in the message is provided as only a void pointer. It's type depends entirely on the type of
     * message, and some messages have no data (<code>dataLen</code> will be zero).
     * </p>
     */
    void InitializeMessaging() {
        if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
            switch (message->type) {
                // Skyrim lifecycle events.
                case MessagingInterface::kPostLoad: // Called after all plugins have finished running SKSEPlugin_Load.
					FormExtraValueHandler::Register();
					//ArithmeticAPI::RequestInterface();
					//ActorValueGeneratorAPI::RequestInterface();
				    
					break;
					// It is now safe to do multithreaded operations, or operations against other plugins.
                
				case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.
					PersonalLoad();
					ArthmeticObject::FinalizeLinkage(LinkerFlags::External);
					break;

                case MessagingInterface::kInputLoaded: // Called when all game data has been found.
                    break;

                case MessagingInterface::kDataLoaded: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
					ArthmeticObject::FinalizeLinkage(LinkerFlags::Object);
					FormExtraValueHandler::Initialize();
                    InitializeHooking();
                    break;

                // Skyrim game events.
                case MessagingInterface::kNewGame: // Player starts a new game from main menu.
                case MessagingInterface::kPreLoadGame: // Player selected a game to load, but it hasn't loaded yet.
                    // Data will be the name of the loaded save.
                case MessagingInterface::kPostLoadGame: // Player's selected save game has finished loading.
                    // Data will be a boolean indicating whether the load was successful.
					//logger::info("Loaded Now.");
					break;
                case MessagingInterface::kSaveGame: // The player has saved a game.
                    // Data will be the save name.
                case MessagingInterface::kDeleteGame: // The player deleted a saved game from within the load menu.
                    break;
            }
        })) {
            stl::report_and_fail("Unable to register message listener.");
        }
    }
}

/**
 * This if the main callback for initializing your SKSE plugin, called just before Skyrim runs its main function.
 *
 * <p>
 * This is your main entry point to your plugin, where you should initialize everything you need. Many things can't be
 * done yet here, since Skyrim has not initialized and the Windows loader lock is not released (so don't do any
 * multithreading). But you can register to listen for messages for later stages of Skyrim startup to perform such
 * tasks.
 * </p>
 */

//*

	//using namespace Arthmetic;


	//float GetRandomRange(RE::Actor* target, const std::vector<Arthmetic::DelegateArgument>& parameter)
	float GetRandomRange(Target target, const ArgumentList args)
	{
		if (target) {
			//We remove the load order defined part of the seed. This basically allows
			// the specific forms to have different but deterministic results.
			uint32_t seed = target->GetFormID() & 0x00FFFFFF;
		
			std::srand(seed);
		}

		float min = args[0]->GetNumberParam();
		logger::info("pre param check");
		float max = args[1]->GetNumberParam();

		//auto str = args[2]->GetStringParam();

		//logger::info("SentString is  \"{}\", min:{}, max:{}", str, min, max);

		//Flawed implementation, do not care.
		int range = max - min + 1;

		int num = rand() % range + min;

		//cout << "Special Function: min = " << min << ", max = " << max << ", result is num " << num << ";";
		ARTHMETIC_LOGGER(debug, "Special Function: min = {}, max = {}, result is num {};", min, max, num);
		return num;
	};


	float GetActorValue(Target target, const ArgumentList args)
	{
		//Now in this sort of situation, I think it's pro
		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor)
			return 0;//Actually throw an exception.

		std::string av_name = args[0]->GetStringParam();

		ExtraValueInput input = static_cast<ExtraValueInput>(args[1]->GetNumberParam());

		float value = Psuedo::GetEitherValue(target_actor, av_name, input);

		logger::trace("{}'s check of AV:'{}'({}) is {}",target_actor->GetName(), av_name, (int)input, value);

		if (isnan(value) == true)
			return 0;//Actually throw an exception.

		return value;
	};
	

	float GetCrimeGold(Target target, const ArgumentList args)
	{
		//Now in this sort of situation, I think it's pro
		RE::Actor* target_actor = target->As<RE::Actor>();
		RE::TESFaction* faction = nullptr;
		
		float result = 0;

		int flag = args[0]->GetNumberParam();


		if (!target_actor)
			faction = target->As<RE::TESFaction>();
		else if (target_actor->IsPlayerRef() == true) {
			RE::PlayerCharacter * player = RE::PlayerCharacter::GetSingleton();

			for (auto& [faction, crime_struct] : player->GetCrimeValue().crimeGoldMap)
			{
				if (flag != 2)//IE if its ab
					result += crime_struct.nonViolentCur;

				if (flag != 1)//IE if its 2
					result += crime_struct.violentCur;
			}
			
			return result;
		}
		else
			faction = target_actor->GetCrimeFaction();

		if (!faction)
			return 0;



		//if any other value than 1 or 2, gets both.
		if (flag != 2)//IE if its 1
			result += faction->GetCrimeGoldNonViolent();

		if (flag != 1)//IE if its 2
			result += faction->GetCrimeGoldViolent();
	
		return result;
	};


	float GetLevel(Target target, const ArgumentList args)
	{

		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor) {
			//Report
			return 0;
		}

		auto result = target_actor->GetLevel();

		//logger::info("result?? {}", result);

		return (float)result;
	};

	float GetPlayerLevelDistance(Target target, const ArgumentList args)
	{
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		if (!player)
			return 0;
		
		bool abs_value = args[0]->GetNumberParam();

		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor) {
			//Report
			return 0;//Or should it just return player....
		}


		if (!target_actor->IsPlayerRef()) {
			//Report
			return 0;
		}

		

		float value = player->GetLevel() - target_actor->GetLevel();

		return abs_value ? abs(value) : value;
	};


	float HasPerks(Target target, const ArgumentList args)
	{
		//String, number, maybe number. Name, should count all, maybe skill requirement.
		//could be interesting if I introduce skill requirements
		//skill requirements could work like this. Negative number/0 means anything under or equal the absolute value, while positive number
		// means anything above or equal to that value.


		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor) {
			//Report
			logger::error("Target is null.");
			return 0;
		}

		//BGSSkillPerkTreeNode* perkTree
		std::string av_name = args[0]->GetStringParam();
		bool should_count = args[1]->GetNumberParam();
		
		RE::ActorValueList* singleton = RE::ActorValueList::GetSingleton();
		
		if (!singleton) {
			//report
			logger::error("Actor Value List singleton has not loaded yet.");
			return 0;
		}

		RE::ActorValue av = singleton->LookupActorValueByName(av_name);

		

		if (av == RE::ActorValue::kNone) {
			//report
			logger::error("Actor Value '{}' couldn't be found", av_name);
			return 0;
		}


		float count = 0;

		if (av < RE::ActorValue::kTotal)
		{
			RE::ActorValueInfo* info = singleton->GetActorValue(av);

			if (!info) {
				logger::error("Actor Value Info '{}' couldn't be found", av_name);
				return 0;
			}

			if (!info->perkTree) {
				logger::error("Actor Value '{}' has no perks.", av_name);
				return 0;
			}

			//No way to get the exact amount of what we are gonna get so I'm just gonna make it twenty.
			std::vector<RE::BGSSkillPerkTreeNode*> visited_nodes{};
			
			std::function<int(RE::BGSSkillPerkTreeNode*)> node_check = [&](RE::BGSSkillPerkTreeNode* node, int direction = 0) -> int
			{
				int count = 0;
				int add = 0;

				add = target_actor->HasPerk(node->perk);
				
				count += add;

				if (add && !should_count)
					return count;
				
				//It would seem the first entry is empty. Makes sense to tell if youve reached the first I guess?
				RE::BGSPerk* next_perk = node->perk ? node->perk->nextPerk : nullptr;

				while (next_perk)
				{
					add = target_actor->HasPerk(next_perk);
					
					count += add;

					if (add && !should_count)
						return count;


					next_perk = next_perk->nextPerk;
				}

				for (auto& child_node : node->children)
				{
					if (std::find(visited_nodes.begin(), visited_nodes.end(), child_node) != visited_nodes.end())
						continue;

					add = node_check(child_node);
					
					visited_nodes.push_back(child_node);

					if (add)
					{
						count += add;

						if (!should_count)
							return count;
					}
				}

				return count;
			};

			return node_check(info->perkTree);
			//RE::BGSSkillPerkTreeNode* 
		}
		else
		{
			//Skills implementation, holding off on.
		}

		return 0;
	};


	float GetGlobalValue(Target target, const ArgumentList args)
	{
		RE::TESGlobal* global = target->As<RE::TESGlobal>();

		//This should be guarded, but it's me being lazy.
		return global ? global->value : 0;
	};

	//Used while I don't have targeting shit
	float GetGlobalValueParam(Target target, const ArgumentList args)
	{
		RE::TESForm* param = args[0]->GetObjectParam();


		//This should be guarded, but it's me being lazy.
		return GetGlobalValue(param, args);
	};


	float GetItemCount(Target target, const ArgumentList args)
	{
		//Notice, I wish to improve this shit plz.
		RE::TESObjectREFR* refr = target->As<RE::TESObjectREFR>();

		if (!refr) {
			//Report
			return 0;
		}

		RE::TESBoundObject* object = args[0]->GetObjectParam()->As<RE::TESBoundObject>();
		
		if (!object) {
			return 0;
		}
		auto inventory_counts = refr->GetInventoryCounts();
		
		return inventory_counts[object];
	};

	float GetHasMagicEffect(Target target, const ArgumentList args)
	{
		RE::Actor* actor = target->As<RE::Actor>();

		if (!actor) {
			return 0;
		}


		RE::TESForm* base = args[0]->GetObjectParam();

		if (!base) {
			logger::error("base object is null.");
			return 0;
		}
		logger::info("{} {} AV", (int)base->GetFormType(), base->GetName());

		RE::EffectSetting* effect = base->As<RE::EffectSetting>();

		if (!effect) {
			logger::error("effect is not an effect.");
			return 0;
		}

		bool should_count = args[1]->GetNumberParam();
		int regard_ability = args[2]->GetNumberParam();

		if (!should_count && !regard_ability) {
			logger::info("firing short hand");//make debug when done producing.
			return actor->AsMagicTarget()->HasMagicEffect(effect);
		}
		

		float count = 0;


		RE::BSSimpleList<RE::ActiveEffect*>* effect_list = actor->AsMagicTarget()->GetActiveEffectList();

		if (!effect_list || effect_list->empty() == true) {
			logger::error("{} doesn't have any active effects.", actor->GetName());
			return 0;
		}

		for (auto active_effect : *effect_list)
		{
			bool active = active_effect->conditionStatus == RE::ActiveEffect::ConditionStatus::kTrue;

			//All other values are any.
			if (regard_ability == 1 && !active || regard_ability == 2 && active)
				continue;

			if (active_effect->GetBaseObject() == effect) {
				count++;

				if (!should_count)
					return count;
			}

		}

		return count;
	};


	float GetGameSetting(Target target, const ArgumentList args)
	{
		RE::Actor* target_actor = target->As<RE::Actor>();
		
		auto* singleton = RE::GameSettingCollection::GetSingleton();

		if (!singleton) {
			//report
			return 0;
		}

		std::string setting_name = args[0]->GetStringParam();
		
		RE::Setting* setting = singleton->GetSetting(setting_name.c_str());

		if (!setting) {
			//report
			return 0;
		}

		switch (setting->GetType())
		{
		case RE::Setting::Type::kBool:
			return setting->GetBool();

		case RE::Setting::Type::kFloat:
			return setting->GetFloat();

		case RE::Setting::Type::kSignedInteger:
			return setting->GetSInt();

		default:
			//report.
			logger::error("Setting type invalid.");
		}
		return 0;
	};

	//These 2 should account for context I think.
	float IsInFaction(Target target, const ArgumentList args)
	{
		if (!target || target->formType != RE::FormType::ActorCharacter)
			return 0;

		RE::TESForm* query_form = args[0]->GetObjectParam();

		//If min is lower than max, it's reversed, and requires that the ranks be not within the range
		float min_rank = args[1]->GetNumberParam();
		float max_rank = args[2]->GetNumberParam();

		bool inverted = min_rank > max_rank;

		if (inverted){
			std::swap(min_rank, max_rank);
		}

		if (!query_form)
			return 0;

		//logger::info("TEST, {}, {}", target->As<RE::BGSKeywordForm>() != nullptr, query_form->formType == RE::FormType::Keyword);

		if (query_form->formType != RE::FormType::Faction)
			return 0;
		//If the range is equal, it will only get what's current.
		if (min_rank == -1 && max_rank == -1) {
			return target->As<RE::Actor>()->IsInFaction(query_form->As<RE::TESFaction>());
		}
		else
		{
			RE::TESFaction* faction = query_form->As<RE::TESFaction>();
			RE::Actor* actor = target->As<RE::Actor>();

			auto* faction_changes = actor->extraList.GetByType<RE::ExtraFactionChanges>();

			auto fact_lamba = [=](auto& fact_data) {
				return fact_data.faction == faction &&
					fact_data.rank != -1 &&
					(fact_data.rank > min_rank - 1) == !inverted &&
					(fact_data.rank < max_rank + 1) == !inverted;
			};

			if (faction_changes)
			{
				auto end = faction_changes->factionChanges.end();
				auto it = std::find_if(faction_changes->factionChanges.begin(), end, fact_lamba);

				if (it != end)
					return 1;
			}
			auto baseNPC = actor->GetActorBase();
			if (baseNPC)
			{
				auto end = baseNPC->factions.end();
				auto it = std::find_if(baseNPC->factions.begin(), end, fact_lamba);

				if (it != end)
					return 1;
			}
		}
		//It shouldn't reach this point but it's whatever right now.

		return 0;
	};

	float IsRace(Target target, const ArgumentList args)
	{
		if (!target || target->formType != RE::FormType::ActorCharacter)
			return 0;

		//if (!target || target->formType != RE::FormType::NPC)
		//	return 0;

		RE::TESForm* query_form = args[0]->GetObjectParam();
		

		if (!query_form || query_form->formType != RE::FormType::Race)
			return 0;


		float original = args[1]->GetNumberParam();

		RE::Actor* actor = target->As<RE::Actor>();

		//we have 
		//actor->GetActorBase()->originalRace
		//and if player, charGenRace
		// For now I'm going to use original race. Unsure if it'll work.

		RE::TESNPC* base = actor->GetActorBase();

		RE::TESRace* act_race = original && base ? base->originalRace : actor->GetActorRuntimeData().race;

		return act_race == query_form->As<RE::TESRace>();

	};



	//These 2 should account for context I think.
	float GetKeyword(Target target, const ArgumentList args)
	{
		if (!target)
			return 0;

		RE::TESForm* query_form = args[0]->GetObjectParam();
		float match_all = args[1]->GetNumberParam();

		if (!query_form)
			return 0;

		//logger::info("TEST, {}, {}", target->As<RE::BGSKeywordForm>() != nullptr, query_form->formType == RE::FormType::Keyword);

		if (query_form->formType != RE::FormType::Keyword)
			return 0;

		if (query_form->formType == RE::FormType::FormList)
			return target->HasKeywordInList(query_form->As<RE::BGSListForm>(), match_all);
		
		if (RE::BGSKeywordForm* keyword_form = target->As<RE::BGSKeywordForm>(); keyword_form)
			return keyword_form->HasKeyword(query_form->As<RE::BGSKeyword>());

		if (RE::TESObjectREFR* refr = target->As<RE::TESObjectREFR>(); refr)
			return refr->HasKeyword(query_form->As<RE::BGSKeyword>());

		//It shouldn't reach this point but it's whatever right now.
		
		return 0;
	};


	float GetKeywordString(Target target, const ArgumentList args)
	{

		if (!target)
			return 0;


		std::string keyword_string = args[0]->GetStringParam();
		
		if (RE::BGSKeywordForm* keyword_form = target->As<RE::BGSKeywordForm>();
			keyword_form && keyword_string != "")
			return keyword_form->HasKeywordString(keyword_string);

		if (RE::TESObjectREFR* refr = target->As<RE::TESObjectREFR>(); refr)
			if (RE::BGSKeyword* keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(keyword_string); keyword)
				return refr->HasKeyword(keyword);


		return 0;
	};




	Target Target::LookUpTarget(std::string context)
	{
		//I'd like to rework this function. It should be broken down into 2 parts, one part being in Arithmetic.
		// First, it should parse. This is the seperation if scope to the left of '::' the address to the right of it, and extra data notations being
		// ruled out with ::: or something like that.

		RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

		if (!handler)
			return nullptr;

		constexpr std::array<std::string_view, 3> extension_names{ ".esp", ".esl", ".esm" };
		constexpr std::array<std::string_view, 2> viable_numbers{ "0x", "0X" };

		auto extension_check = [](char& it)
		{
			std::string_view check(&it, 4);

			switch (hash(check))
			{
			case hash(extension_names[0]):
			case hash(extension_names[1]):
			case hash(extension_names[2]):
				return true;

			default:
				return false;
			}
		};

		auto number_check = [](char& it)
		{
			std::string_view check(&it, 2);

			switch (hash(check))
			{
			case hash(viable_numbers[0]):
			case hash(viable_numbers[1]):
				return true;

			default:
				return false;
			}
		};

		//auto it = std::find_first_of(context.begin(), context.end(), extension_names.begin(), extension_names.end());
		auto it = std::find_if(context.begin(), context.end(), extension_check);


		if (context.end() == it) {
			//This is gonna have to be smarter at some point, like check for only has numbers
			if (_strnicmp(context.c_str(), "0x", 2) == 0)
			{
				auto value = std::stoul(context, nullptr, 16);

				if (value > 0xFFFFFFFF) {
					//report.
					return Target();
				}
				
				return RE::TESForm::LookupByID(static_cast<RE::FormID>(value));
			}
			else
			{
				return RE::TESForm::LookupByEditorID(context);
			}
		}

		std::string target_plugin = std::string(context.begin(), it + 4);

		//Here's an idea, you use different :: to signify which thing we're supposed to be using.
		auto hex_point = context.find("::");
		
		//I'm actually thinking of using the hex point to represent calls for extra data.
		

		if (hex_point == std::string::npos) {//The point here is to cut off spaces
			logger::error("colon not found");
			throw nullptr;
		}


		hex_point += 2;


		//it = std::find_first_of(context.begin() + hex_point, context.end(), viable_numbers.begin(), viable_numbers.end());
		it = std::find_if(context.begin() + hex_point, context.end(), number_check);

		if (context.end() == it) {
			logger::error("hex notation not found");
			throw nullptr;
		}

		context = std::string(it, context.end());



		auto value = std::stoul(context, nullptr, 16);

		//Shortened because it knocks off the first 2 counts
		if (value > 0x00FFFFFF) {
			logger::error("value from '{}' is larger than max local ID.", context);
			return Target();
		}

		logger::info("Looking up '{:X}'('{}') from '{}' . . .", value, context, target_plugin);
		
		return handler->LookupForm(static_cast<RE::FormID>(value), target_plugin);
	}

	inline REL::Version _currentVersion{};
	uint32_t RGL::GetProjectVersion()
	{
		//constexpr REL::Version version(1, 8, 7, 4);

		//1.0.0.0 Use the Commonlib versioning later. It has a packing function.
		return _currentVersion.pack();
	}

	void temp_AssignFormula(std::string key, float(*func)(Target, const ArgumentList))
	{
		auto result = formulaMap.find(key);
		
		if (formulaMap.end() != result) {
			//This won't have to be done once we do the cortn an func_int merger.
			auto* func_intfc = dynamic_cast<FunctionInterface*>(result->second);

			if (func_intfc)
				func_intfc->_callback = func;
			else
				logger::error("Fucking {} is NOT a function interface.", key);
		}
		else
		{
			logger::error("{} never existed. . . Despairge", key);
		}
	}

	void temp_NativeFormulaRegister()
	{
		temp_AssignFormula("GetRandomRange", GetRandomRange);
		temp_AssignFormula("GetActorValue", GetActorValue);
		temp_AssignFormula("GetCrimeGold", GetCrimeGold);
		temp_AssignFormula("IsRace", IsRace);
		temp_AssignFormula("IsInFaction", IsInFaction);
		temp_AssignFormula("GetKeyword", GetKeyword);
		temp_AssignFormula("GetHasMagicEffect", GetHasMagicEffect);
		temp_AssignFormula("GetItemCount", GetItemCount);
		temp_AssignFormula("GetGlobalValueParam", GetGlobalValueParam);
		temp_AssignFormula("GetCrimeGold", GetCrimeGold);
		temp_AssignFormula("GetKeywordString", GetKeywordString);
		temp_AssignFormula("GetGameSetting", GetGameSetting);
		temp_AssignFormula("HasPerks", HasPerks);
		temp_AssignFormula("GetPlayerLevelDistance", GetPlayerLevelDistance);
		temp_AssignFormula("GetLevel", GetLevel);

	}

	void StartOther()
	{
		return;
		std::string func1 = "GetActorValue('SingleWielding', Damage | Permanent) - GetActorValue('SingleWielding', Damage | Permanent)";
		std::string func = "label_1; GetActorValue(::ending = 'SingleWielding', ActorValue::Damage | ActorValue::Permanent); return result";

		std::string::const_iterator start, end;
		start = func.begin();
		end = func.end();

		const boost::regex test_regex1("(?<Minus>- )|(?<Object><([^>](?!\\s)){0,}(?=>).)|(?<Quotes>'[^']{0,}.)|(?<Identifiers>(\\w|::)+)|(?<Symbols>[^\\w\\d\\s'][^\\w\\d\\s']{0,})");
		const boost::regex test_regex("(?<Header>^[\\w;]{0,} {0,}(?=;).)|(?<Minus>- )|(?<Object><:.+(?=:>)..)|(?<Digits>[\\d\\.]+)|(?<Quotes>'[^']+.)|(?<Identifiers>(\\w|::)+)|(?<Symbols>[^\\w\\d\\s'\\.][^\\w\\d\\s'\\.:]{0,})");
		
		boost::smatch what;
		std::vector<std::string> results;
		while (boost::regex_search(start, end, what, test_regex))
		{
			results.push_back(what[0]);
			start = what[0].second;
		}

		if (results.size()) 
		{
			logger::info("Function: {}", func);
		
			for (auto& str : results)
			{
				logger::info("{}", str);

			}
		}
		
		
		logger::info("Testing Types, {}", TestFunc_Type());

		exportMap["AffectActorValue"] = AffectActorValue;

		return;

		FunctionInterface* get_random_range = new FunctionInterface();
		get_random_range->defaultValue = 1.0f;
		get_random_range->_callback = GetRandomRange;
		formulaMap["GetRandomRange"] = get_random_range;

		{
			ParsingArgumentList parameter_list
			{
				{"actor_value", "String" },
				{"modifier_flags", "Number: 15" }
			};
			FunctionInterface* get_actor_value = new FunctionInterface(CreateParameterSettings(parameter_list));
			get_actor_value->defaultValue = 0.0f;
			get_actor_value->_callback = GetActorValue;
			formulaMap["GetActorValue"] = get_actor_value;
		}

		{
			ParsingArgumentList parameter_list
			{
				{"crime_type", "Number: 0" }
			};
			FunctionInterface* get_crime_gold = new FunctionInterface(CreateParameterSettings(parameter_list));
			get_crime_gold->defaultValue = 0.0f;
			get_crime_gold->_callback = GetCrimeGold;
			formulaMap["GetCrimeGold"] = get_crime_gold;
		}
		
		//GetKeywordString
		//GetKeyword
		//GetGameSetting
		//GetHasMagicEffect
		//GetItemCount
		//GetGlobalValue
		//HasPerks
		//GetPlayerLevelDistance
		//GetLevel
		//GetCrimeGold

#define can_use_object_now
#ifdef can_use_object_now
		{
			ParsingArgumentList parameter_list
			{
				{"keyword", "Object" }
			};
			FunctionInterface* i_GetKeyword = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_GetKeyword->defaultValue = 0.0f;
			i_GetKeyword->_callback = GetKeyword;
			formulaMap["GetKeyword"] = i_GetKeyword;
		}

		{
			ParsingArgumentList parameter_list
			{
				{"magic_effect", "Object" },
				{"should_count", "Number: 0" },
				{"regard_ability", "Number: 0" }
			};
			FunctionInterface* i_GetHasMagicEffect = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_GetHasMagicEffect->defaultValue = 0.0f;
			i_GetHasMagicEffect->_callback = GetHasMagicEffect;
			formulaMap["GetHasMagicEffect"] = i_GetHasMagicEffect;
		}

		{
			ParsingArgumentList parameter_list
			{
				{"item", "Object" }
			};
			FunctionInterface* i_GetItemCount = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_GetItemCount->defaultValue = 0.0f;
			i_GetItemCount->_callback = GetItemCount;
			formulaMap["GetItemCount"] = i_GetItemCount;
		}


		//This set up can be done via routine if I want to remove this as a function later.
		{
			ParsingArgumentList parameter_list
			{
				{"global_variable", "Object" }
			};
			FunctionInterface* i_GetGlobalValueParam = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_GetGlobalValueParam->defaultValue = 0.0f;
			i_GetGlobalValueParam->_callback = GetGlobalValueParam;
			formulaMap["GetGlobalValueParam"] = i_GetGlobalValueParam;
		}


		/*
		{
			FunctionInterface* i_GetGlobalValue = new FunctionInterface();
			i_GetGlobalValue->defaultValue = 0.0f;
			i_GetGlobalValue->_callback = GetGlobalValue;
			formulaMap["GetGlobalValue"] = i_GetGlobalValue;
		}
		//*/
#endif

		{
			ParsingArgumentList parameter_list
			{
				{"crime_type", "Number: 0" }
			};
			FunctionInterface* get_crime_gold = new FunctionInterface(CreateParameterSettings(parameter_list));
			get_crime_gold->defaultValue = 0.0f;
			get_crime_gold->_callback = GetCrimeGold;
			formulaMap["GetCrimeGold"] = get_crime_gold;
		}

		
		{
			ParsingArgumentList parameter_list
			{
				{"keyword_name", "String" }
			};
			FunctionInterface* i_GetKeywordString = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_GetKeywordString->defaultValue = 0.0f;
			i_GetKeywordString->_callback = GetKeywordString;
			formulaMap["GetKeywordString"] = i_GetKeywordString;
		}
		
		{
			ParsingArgumentList parameter_list
			{
				{"setting_name", "String" }
			};
			FunctionInterface* i_GetGameSetting = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_GetGameSetting->defaultValue = 0.0f;
			i_GetGameSetting->_callback = GetGameSetting;
			formulaMap["GetGameSetting"] = i_GetGameSetting;
		}

		{
			ParsingArgumentList parameter_list
			{
				{"av_name", "String" },
				{"should_count", "Number: 0" }
			};
			FunctionInterface* i_HasPerks = new FunctionInterface(CreateParameterSettings(parameter_list));
			i_HasPerks->defaultValue = 0.0f;
			i_HasPerks->_callback = HasPerks;
			formulaMap["HasPerks"] = i_HasPerks;
		}


		{
			FunctionInterface* i_GetPlayerLevelDistance = new FunctionInterface();
			i_GetPlayerLevelDistance->defaultValue = 0.0f;
			i_GetPlayerLevelDistance->_callback = GetPlayerLevelDistance;
			formulaMap["GetPlayerLevelDistance"] = i_GetPlayerLevelDistance;
		}

		{
			FunctionInterface* i_GetLevel = new FunctionInterface();
			i_GetLevel->defaultValue = 0.0f;
			i_GetLevel->_callback = GetLevel;
			formulaMap["GetLevel"] = i_GetLevel;
		}



		//propertyMap["true"] = new ArthmeticValue(1);
		//propertyMap["false"] = new ArthmeticValue(0);


		//propertyMap["Base"] = new ArthmeticValue(1);
		//propertyMap["Permanent"] = new ArthmeticValue(2);
		//propertyMap["Temporary"] = new ArthmeticValue(4);
		//propertyMap["Damage"] = new ArthmeticValue(8);
		//propertyMap["All"] = new ArthmeticValue(15);

		return;

		{
			logger::info("Testing Collection...");

			ParsingArgumentList parameter_list
			{
				{"value", "number" },
				{"target_name", "string" },
				{"modifier", "Number:      2" },
				//{"focus", "Target" },
				{"av_name", "String: Health" },
				{"check_value", "number:10.5" },
				{"reason", "string: None in particular" }
			};


			//Still not ready for this version, the routine delegate doesn't actually work.
			//std::string co_func = "modifier * GetRandomRange(value, check_value, reason)";
			//std::string co_func = "2 * GetRandomRange(value, check_value, reason)";
			std::string co_func = "modifier * GetRandomRange(value, check_value, reason)";

			formulaMap["TestFunctionE"] = CreateFormula(co_func, parameter_list);
		}

		return;
		
		//auto result = CreateParameterSettings(parameter_list);

	

		//std::string function = "true - 22. - .2 + 2. + (3 * 4) / GetRandomRange(true, 5, 'TestString') * (5 % 6) ^ 7";
		std::string function = "true - 22. - .2 + 2. + (3 * 4) / TestFunctionE(1.5, 'Unknown-Target') * (5 <= 6) ^ 7";

		//auto* sub = CreateSubroutine(function);

		//Fix such a thing,
		//sub->IReadyArthmetic::RunImpl(nullptr);

		return;
	}


//*/

/*
//Taken from BTPS, want to study for api stuff



namespace Plugin
{
	using namespace std::literals;

	inline constexpr REL::Version VERSION
	{
		// clang-format off
		1u,
		0u,
		0u,
		// clang-format on
	};

	inline constexpr auto NAME = "ActorValueGenerator"sv;
}


extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v{};
	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.UsesAddressLibrary();
	v.HasNoStructUse();

	return v;
}();


extern "C" DLLEXPORT void* SKSEAPI RequestPluginAPI(const BTPS_API_decl::Version a_interfaceVersion)
{
	auto api = BTPS_API::GetSingleton();

	logger::info("BTPS: API is being requested, version {}", a_interfaceVersion);

	switch (a_interfaceVersion)
	{
	case BTPS_API_decl::Version::V0:
		logger::info("BTPS: API request successful");
		return static_cast<void*>(api);
	}

	logger::info("BTPS: API request called with invalid version");
	return nullptr;
}

*/
/*
	extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
		SKSE::PluginVersionData v;

		v.PluginVersion(Plugin::VERSION);
		v.PluginName(Plugin::NAME);
		v.AuthorName("NoahBoddie");
		v.UsesAddressLibrary(true);
		v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });//RUNTIME_LATEST_VR
		v.HasNoStructUse(true);

		return v;
	}();
//*/

SKSEPluginLoad(const LoadInterface* skse) {
#ifdef _DEBUG
	//Need a way to only have this happen when holding down a key
	if (GetKeyState(VK_RCONTROL) & 0x800) {
		constexpr auto text1 = L"Request for debugger detected. If you wish to attach one and press Ok, do so now if not please press Cancel.";
		constexpr auto text2 = L"Debugger still not detected. If you wish to continue without one please press Cancel.";
		constexpr auto caption = L"Debugger Required";
		
		int input = 0;

		do
		{
			input = MessageBox(NULL, !input ? text1 : text2, caption, MB_OKCANCEL);
		} 
		while (!IsDebuggerPresent() && input != IDCANCEL);
	}
#endif
	
	InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
	_currentVersion = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), _currentVersion);


    Init(skse);
    InitializeMessaging();
    InitializeSerialization();

	
    log::info("{} has finished loading.", plugin->GetName());

	StartOther();

	return true;
}
