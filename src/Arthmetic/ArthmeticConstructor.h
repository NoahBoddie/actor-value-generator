#pragma once

#include "Arthmetic/RoutineItem.h"
#include "Arthmetic/ArthmeticParser.h"
//#include "Arthmetic/FunctionDelegate.h"
//#include "Arthmetic/RoutineDelegate.h"
//#include "Arthmetic/FormulaDelegate.h"//temp
//#include "Arthmetic/DualOperator.h"
//#include "Arthmetic/OpenOperator.h"

//#include"Arthmetic/Subroutine.h"
//#include"Arthmetic/Coroutine.h"

namespace Arthmetic
{
	std::vector<RoutineItem*> CreateRoutineItems(RecordIterator& it, RecordIterator end);


	inline RoutineItem* MakeTopLevelItem(uint16_t pos, RecordIterator& it)
	{
		//This is called make top level item because these are all that can be placed outside of functions/routines, barring the use
		// of either of those themselves.

		//I would actually like this function to take dual operator, open operator and

		const RecordData& data = *it;

		//logger::info("Start of top");

		RoutineItem* result = nullptr;

		DataType type = data.GetType();

		switch (type)
		{
		case DataType::Number:
			return RoutineItem::Create(it, pos, RoutineItemType::Value);
		case DataType::Operator:
			return RoutineItem::Create(it, pos, RoutineItemType::Operator);
		default:
			logger::critical("Data type {} not detected for Top Level Item.", (int)type);
			throw nullptr;
		}

		//This implies there's only 2 data types here, Number and Operator, neither one needing specifying.
		if (data.GetType() == DataType::Number) {
			return RoutineItem::Create(it, pos, RoutineItemType::Value);
		}
		else
		{
			return RoutineItem::Create(it, pos, RoutineItemType::Operator);

			//This implemenation will change once when 
			switch (data.view[0])
			{
				//Both of these are linked to the same deal. Like above, these differences will get phased out.
			case '(':
				return RoutineItem::Create(it, pos, RoutineItemType::OpOpen);
			case ')':
				return RoutineItem::Create(it, pos, RoutineItemType::OpClose);


			default:
				return RoutineItem::Create(it, pos, RoutineItemType::Operator);

			//default:
			//	ARTHMETIC_LOGGER(info, "Failure to handle operator, value \'{}\'", data.view[0]);
			//	throw nullptr;
			}
		}
	}


	inline RoutineItem* TestMakeFunction(uint16_t& pos, RecordIterator& it)
	{
		//logger::info("Pre start");

		std::string code{ it->view };

		DataType type = it->GetType();

		//logger::info("Start");

		if (type == DataType::FormulaCall) {
			RoutineItem* new_func = RoutineItem::Create(it, pos, RoutineItemType::FunctionValue);
			return new_func;
		}
		else if (type == DataType::ParameterCall) {//temporary, needs to search smarter.
			RoutineItem* new_delegate = RoutineItem::Create(it, pos, RoutineItemType::RoutineValue);
			return new_delegate;
		}
		else {
			//logger::info("Nq {} {}", it->view, (int)it->GetType());

			return MakeTopLevelItem(pos, it);
		}
	}


	
	inline std::vector<RoutineItem*> CreateRoutineItems(RecordIterator& it, RecordIterator end)
	{
		uint16_t i = 0;
		
		std::vector<RoutineItem*> result_vect{};
		
		result_vect.reserve(std::distance(it, end));

		//std::vector<RoutineItem*> result_vect{};
		//result_vect.reserve(std::distance(it, end));

		while (it != end) {
			//string to_print = *it;

			//if (i == 0)
			//	cout << ", ";
			//else
			//	cout << "[";

			//cout << to_print << "(" << i << ")";
			
			//use vector::reserve instead of this, it's FAR fucking safer too.


			RoutineItem* new_code = TestMakeFunction(i, it);
			//logger::info("-after");
			result_vect.push_back(new_code);
			//result_vect.push_back(new_code);

			it++;
			i++;
		}

		result_vect.shrink_to_fit();

		return result_vect;
	}


	//This consists of a pair of names and values. The values are defined by the presence of 
	using ParsingArgumentList = std::vector<std::pair<std::string, std::string>>;


	//This comes in 2 parts, determining the type, the setting the value.
	inline int GetArgTypeFromString(std::string type_name)
	{
		auto spaces = type_name.find(' ');

		if (spaces != std::string::npos)//The point here is to cut off spaces
			type_name = type_name.substr(0, spaces);


		std::transform(type_name.begin(), type_name.end(), type_name.begin(),
			[](unsigned char c) { return std::tolower(c); });



		switch (hash(type_name))
		{
		case "string"_h:
			return static_cast<int>(ArgumentType::String);

		case "target"_h:
		case "object"_h:
			return static_cast<int>(ArgumentType::Object);

		case "number"_h:
			return static_cast<int>(ArgumentType::Number);


		case "any"_h:
			return static_cast<int>(ArgumentType::Number);
		default:
			//If nothing is found here, it's left upto this to infer the type.
			ARTHMETIC_LOGGER(error, "'{}' is not a valid argument type name", type_name);
			return -1;
		}
	}

	inline DelegateArgument CreateDelegateArgument(const std::string& a_value, int default_rules = 0)
	{

		DelegateArgument arg_value{};

		//This should also curb off some of the spaces possibly found in the type name.
		auto default_line = a_value.find(':');//Default line needs to account for spaces
		//Note, right here is where the cutt off needs to happen, looking for the default line (or
		// determining it doesn't have a default value).


		int type;



		if (default_line == std::string::npos) {//The point here is to cut off spaces
			type = GetArgTypeFromString(a_value);
			default_line = a_value.size();
		}
		else {
			type = GetArgTypeFromString(a_value.substr(0, default_line));
			default_line++;
		}

		if (type == -1)
			throw nullptr;//Invalid type detected, throw



		ArgumentType arg_type = static_cast<ArgumentType>(type);

		//spaces do not count
		auto value_begin = std::find_if(a_value.begin() + default_line, a_value.end(), [](char i) { return i != ' '; });

		//This part needs to be made into a function so I can reuse it. Preferably, one that's templated for ease of use.
		if (value_begin == a_value.end())
		{
			if (default_rules > 0) {
				ARTHMETIC_LOGGER(critical, "Must have a default value for ('{}')", a_value);
				throw nullptr;
			}

			arg_value = arg_type;
			//logger::info("(Non-Default) name: {}, type: {}", param_setting.name, arg_type);
		}
		else
		{
			if (default_rules < 0) {
				ARTHMETIC_LOGGER(critical, "Cannot have a default value following. ('{}')", a_value);
				throw nullptr;
			}

			std::string default_value(value_begin, a_value.end());

			//logger::info("(Default) name: {}, type: {}, string: {}", param_setting.name, arg_type, default_value);

			switch (arg_type)
			{
			case ArgumentType::String:
				arg_value = default_value;
				//logger::info("(!) String Value: {}", arg_value.GetStringParam());
				break;
			case ArgumentType::Number:
				arg_value = std::stof(default_value);
				//logger::info("(!) Number Value: {}", arg_value.GetNumberParam());
				break;

			case ArgumentType::Object:
				//ARTHMETIC_LOGGER(critical, "Object parameters are not current supported. ('{}')", a_value);
				arg_value.ConstructAs(DataType::Object, default_value);
				//logger::info("(!) Object Value: {}", default_value);
				break;
			}

			arg_value.ForceResolved();
		}

		return arg_value;
	}


	inline ParameterSetting CreateParameterSetting(const std::string& a_name, const std::string& a_value, int i = -1, int default_rules = 0)
	{
		//Defualt rules are as follows. If it's 0 it doesn't care. if it's above 0 then it mustn't be default.
		// if it's below 0

		ParameterSetting param_setting{};

		param_setting.index = i;


		param_setting.name = a_name;

		//This should also curb off some of the spaces possibly found in the type name.
		auto default_line = a_value.find(':');//Default line needs to account for spaces
		//Note, right here is where the cutt off needs to happen, looking for the default line (or
		// determining it doesn't have a default value).


		int type;



		if (default_line == std::string::npos) {//The point here is to cut off spaces
			type = GetArgTypeFromString(a_value);
			default_line = a_value.size();
		}
		else {
			type = GetArgTypeFromString(a_value.substr(0, default_line));
			default_line++;
		}

		if (type == -1)
			throw nullptr;//Invalid type detected, throw



		ArgumentType arg_type = static_cast<ArgumentType>(type);

		//spaces do not count
		auto value_begin = std::find_if(a_value.begin() + default_line, a_value.end(), [](char i) { return i != ' '; });

		//This part needs to be made into a function so I can reuse it. Preferably, one that's templated for ease of use.
		if (value_begin == a_value.end())
		{
			if (default_rules > 0) {
				ARTHMETIC_LOGGER(critical, "Cannot have a non-default value following a default one. ('{}':'{}')", a_name, a_value);
				throw nullptr;
			}

			param_setting.defaultValue = arg_type;
			logger::info("(Non-Default) name: {}, type: {}", param_setting.name, arg_type);
		}
		else
		{
			if (default_rules < 0) {
				ARTHMETIC_LOGGER(critical, "Cannot a default value here. ('{}':'{}')", a_name, a_value);
				throw nullptr;
			}

			std::string default_value(value_begin, a_value.end());

			logger::info("(Default) name: {}, type: {}, string: {}", param_setting.name, arg_type, default_value);

			switch (arg_type)
			{
			case ArgumentType::String:
				param_setting.defaultValue = default_value;
				logger::info("(!) String Value: {}", param_setting.defaultValue.GetStringParam());
				break;
			case ArgumentType::Number:
				param_setting.defaultValue = std::stof(default_value);
				logger::info("(!) Number Value: {}", param_setting.defaultValue.GetNumberParam());
				break;

			case ArgumentType::Object:
				//ARTHMETIC_LOGGER(critical, "Object parameters are not current supported. ('{}':'{}')", a_name, a_value);
				param_setting.defaultValue.ConstructAs(DataType::Object, default_value);
				logger::info("(!) Object Value: {}", default_value);
				break;
			}

			param_setting.defaultValue.ForceResolved();
		}

		return param_setting;
	}


	inline std::vector<ParameterSetting> CreateParameterSettings(const ParsingArgumentList arguments)
	{
		std::vector<ParameterSetting> result_vector(arguments.size());

		logger::info("size {}", result_vector.size());

		bool default_declared = false;

		int i = 0;

		for (auto& [a_name, a_value] : arguments)
		{
			ARTHMETIC_LOGGER(info, "Start ('{}':'{}')", a_name, a_value);

			//logger::info("i is {}", i);

			result_vector[i++] = CreateParameterSetting(a_name, a_value, i, default_declared);			
		}

		//logger::info("end");

		return result_vector;
	}

	
}