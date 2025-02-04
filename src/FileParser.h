#pragma once

#include "Types.h"
#include "ExtraValueInfo.h"

#include "ValueAliasHandler.h"

namespace AVG
{
	std::map<std::string, std::vector<std::string>> includeMap;


	AliasSet GetAliasSetFromName(std::string_view name)
	{
		bool is_plugin = name.contains(".es");

		return is_plugin ? AliasSet::Plugin : AliasSet::Keyword;
	}


	void HandleIncludeLists()
	{
		for (auto& [name, list] : includeMap) {
			
			auto set = GetAliasSetFromName(name);

			auto& alias_node = aliasMap[set][name];

			for (auto& entry : list)
			{
				if (entry == "") {
					logger::warn("Include entry is empty.");
					continue;
				}

				std::vector<std::string> results;
				
				//This needs to be using regex now.
				results = boost::split(results, entry, boost::is_any_of("="));

				std::string ev_name;

				//This is currently unchanging
				AliasMode mode = AliasMode::Default;
				
				UnionValue alias_value;

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

					if (alias_value.GetActorValue() == RE::ActorValue::kTotal) {
						//Shit is invalid, do not proceed.

						alias_value = StringToVirtualValue(results[1]);

						if (alias_value.GetVirtualValue() == VirtualValue::kTotal) {
							//Both are invalid, do not proceed.
							continue;
						}
					}

					if (set == AliasSet::Plugin && alias_value.GetActorValue() == RE::ActorValue::kNone) {
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

				if (alias_value.GetVirtualValue() == VirtualValue::kTotal) {
					if (Utility::IsValidValue(info->_aliasID) == true) {
						alias_value = info->_aliasID;
					}
					else {
						//This shit is invalid
						logger::error("Extra Value {} has no default alias to be used by include list.", ev_name);
						continue;
					}
				}

				
				logger::info("Including {} to {} at {}", ev_name, name, (int)alias_value);

				alias_node.AddSetting(alias_value, info, mode);
			}
			//*/
		}
	}


	bool HandleLegacyGlobal(std::string& out, std::string_view name)
	{
		if (name == "true" || name == "false") {
			//keyword names, cannot be established.
			return false;
		}

		//This will have to do a version check to make sure that it can handle something like default parameters if we ever get to that point.
		
		//Base = "Number: 1"
		//PlayerFactionAshlandersGlobal = "Object: AVG_Test.esp::0x592C"
		enum Mode
		{
			kType,
			kValue,	//Dealing withe the value depends on the situation
		};

		std::string type;

		std::string value;

	
		std::istringstream stream{ out };
		std::string into;

		Mode mode = kType;
		

		//object has 3 modes. EditorID search, straight form id search, local form id search. Make sure people are VERY aware, this does not
		// use properties, or handle formulas, and if one needs that, they should make a different project.

		//We can tell if it's an editor id if it 

		char search_char = ':';

		while (mode <= kValue && std::getline(stream, into, search_char)) {
			//Do stuff.
			if (into.empty())
				continue;

			switch (mode)
			{
			case kType:
				//This should clear any spaces if there are any.
				//into = into.substr(0, into.find(' '));
				boost::trim(into);

				switch (RGL::Hash<RGL::HashFlags::Insensitive>(into))
				{
				case "Object"_ih:
					type = "Form"; break;

				case "Number"_ih:
					type = "float"; break;

				case "String"_ih:
					type = "string"; break;

				default:
					logger::error("{} is not a valid legacy AVG type.", into);
					return false;
				}
				search_char = '\0';
				break;
			
			case kValue:
				switch (RGL::Hash(type))
				{
				case "Form"_h:
				{
					//If you wanted to do this you'd probably try to reverse find space and substring that and end,
					// and then do the same thing you did above. For now, this will do.
					logger::info("before '{}'", into);
					boost::trim(into);
					logger::info("after '{}'", into);

					if (strnicmp(into.c_str(), "0x", 2) == 0)
					{
						value = " = LookupByFormID(" + into + ")";
						break;
					}
					
					

					std::istringstream stream{ into };
					std::string buf;


					if (std::getline(stream, buf, ':') && stream.get() == ':')
					{
						//check the next part of the stream for 
						boost::trim(buf);
						
						value = " = LookupByLocalID( '" + buf + "' , ";

						if (std::getline(stream, buf))
						{
							boost::trim(buf);

							value += buf + " )";
						}
						else
						{
							logger::error("Expected values plugin name. {}", into);
							return false;
						}
					}
					else
					{
						value = " = LookupByEditorID( '" + into + "' )";
					}
				}
				break;
				case "float"_ih:
					//Needs checks.
					value = std::stof(into); 
					[[fallthrough]];
				case "string"_ih:
					value = " = " + into; break;
				default:
					logger::error("{} is not a valid legacy AVG type.", into);
					return false;
				}
				break;
			}

			mode++;

		}

		if (mode == kType) {
			logger::error("{} didn't have enough.", out);
			return false;
		}
		out = std::format("{} {}{};\n", type, name, value);

		//if we didn't make it to default print the name and say it's not valid.

		return true;
	}


	bool HandleLegacyParam(std::string& out)
	{
		//This will have to do a version check to make sure that it can handle something like default parameters if we ever get to that point.
		enum Mode
		{
			kName,
			kType,
			kDefault,
		};

		std::string name;

		std::string type;

		//std::string def;


		
		std::istringstream stream{ out };
		std::string into;

		Mode mode = kName;
		bool cont = true;

		char search_char = '=';

		while (cont && std::getline(stream, into, search_char)) {
			//Do stuff.
			if (into.empty())
				continue;

			switch (mode)
			{
			case kName:
				boost::trim(into);
				
				if (into.contains(' ') == true) {
					logger::error("Parameter string '{}' is not a valid.", into);
					return false;
				}
					
			
				name = into; 
				
				search_char = ':';

				break;

			case kType:
				boost::trim(into);

				switch (RGL::Hash<RGL::HashFlags::Insensitive>(into))
				{
				case "Object"_ih:
					type = "Form"; break;
				
				case "Number"_ih:
					type = "float"; break;
				
				case "String"_ih:
					type = "string"; break;
				default:
					logger::error("{} is not a valid legacy AVG type.", into);
					return false;
				}
				search_char = '\0';

				break;
			

			case kDefault:
				//std::getline(stream, into); Later, this might take the entire thing and just put it on there.
				//def = " = " + into;
				cont = false;
				break;
			}

			mode++;

		}

		if (mode < kType) {
			logger::error("Cannot convert legacy parameter '{}'.", into);
			return false;
		}

		out = std::format("{} {}", type, name);

		//if we didn't make it to default print the name and say it's not valid.

		return true;
	}


	bool HandleLegacySetFunc(std::string& out, RE::ActorValueModifier modifier)
	{
		//This will have to do a version check to make sure that it can handle something like default parameters if we ever get to that point.
		enum Mode
		{
			kName,
			kType,
			kDefault,
		};

		std::string name;

		std::string type;

		//std::string def;



		std::istringstream stream{ out };
		std::string into;

		Mode mode = kName;
		bool cont = true;

		char search_char = '=';

		while (cont && std::getline(stream, into, search_char)) {
			//Do stuff.
			if (into.empty())
				continue;

			switch (mode)
			{
			case kName:
				boost::trim(into);

				if (into.contains(' ') == true) {
					logger::error("Parameter string '{}' is not a valid.", into);
					return false;
				}


				name = into;

				search_char = ':';

				break;

			case kType:
				boost::trim(into);

				switch (RGL::Hash<RGL::HashFlags::Insensitive>(into))
				{
				case "Object"_ih:
					type = "Form"; break;

				case "Number"_ih:
					type = "float"; break;

				case "String"_ih:
					type = "string"; break;
				default:
					logger::error("{} is not a valid legacy AVG type.", into);
					return false;
				}
				search_char = '\0';

				break;


			case kDefault:
				//std::getline(stream, into); Later, this might take the entire thing and just put it on there.
				//def = " = " + into;
				cont = false;
				break;
			}

			mode++;

		}

		if (mode < kType) {
			logger::error("Cannot convert legacy parameter '{}'.", into);
			return false;
		}

		out = std::format("{} {}", type, name);

		//if we didn't make it to default print the name and say it's not valid.

		return true;
	}






//#define APPEND_SCRIPT_EXISTS

	void HandleFileInput(std::string& name, const FileNode& node, bool is_legacy)
	{
		//Doesn't work, seems to only get testEV, maybe my choice isn't entirely set up properly?


		//SO if something is an array of tables (It can check which is nice) then it will check the name for composite types.
		// Otherwise, what we'll do is check if there is a type value within, and if not, this key is not valid.



		if (auto test = node.as_table(); test)
		{
			auto& table = *test;

			//If it's one of these types. Later, it's basically if it's a reserved name or not.
			switch (RGL::Hash<RGL::HashFlags::Insensitive>(name))
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
					
					if (HandleLegacyGlobal(value, property_name) == true) {
						if (is_legacy) {
							legacy->AppendContent(value);
							logger::debug("Legacy property created: {}", value);
						}
						else {
							logger::warn("Properties through AVG no longer supported, use Lexicon scripts instead. Formatted to:\n{}", value);
						}
					}
				}
				return;
			}



			std::string type = table["type"].value_or("Invalid");
			ExtraValueType ev_type;
			switch (RGL::Hash<RGL::HashFlags::Insensitive>(type))
			{
		
			case "Routine"_ih:
			{
				std::string parameters;

				std::string native;

				std::string body;

				if (auto formula = table["formula"]; formula.is_value())
				{
					if (formula.is_string() == true)
						//hopefully there's actually something in this shit.
						body = "{ return "s + formula.value_or("") + "; }\n";
					else
						throw std::exception("Fucking formula needs to be a string. Address.");
				}
				else
				{
					body = ";\n";
				}


				auto param_view = table["params"];

				if (auto p_test = param_view.as_array(); p_test)
				{
					auto& param_array = *p_test;
					int i = 0;

					for (auto& entry : param_array)
					{
						if (entry.is_string() == false) {
							logger::critical("Non string entry found in entry {}.", i);
							throw std::exception("Non-string. Address.");
						}

						i++;

						std::string str = entry.value_or("");

						if (HandleLegacyParam(str) == false)
							throw std::exception("Fucking string doesn't work. Address.");

						parameters += ", " + str;
					}

				}

				//I wish to handle this differently later, iterating and comparing for a sheet of flags instead of this.			
				auto* flag_array = table["flags"].as_array();

				if (flag_array)
				{
					auto result = std::find_if(flag_array->cbegin(), flag_array->cend(), [](auto& it) {return it.value_or("") == "native"; });

					if (flag_array->end() == result)
						native = "external";

				}

				//default value cannot properly be interpreted at this time.
				//table["default"].value_or(0);

				std::string to_append = std::format("{} float {}(this Actor{}){}", native, name, parameters, body);

				if (is_legacy) {
					legacy->AppendContent(to_append);
					logger::debug("Legacy Routine created: {}", to_append);
				}
				else {
					logger::warn("Routines through AVG no longer supported, use Lexicon scripts instead. Formats to:\n{}", to_append);
			
				}

				return;
			}

			case "Adaptive"_ih:
				ev_type = ExtraValueType::Adaptive;
				goto create_ev;

			case "Exclusive"_ih:
				ev_type = ExtraValueType::Exclusive;
				goto create_ev;

			case "Functional"_ih:
				ev_type = ExtraValueType::Functional;

				create_ev:
				ExtraValueInfo::Create(name, ev_type, table, legacy);
				break;

			
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
}