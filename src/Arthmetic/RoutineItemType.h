#pragma once

#include "Arthmetic/ArthmeticUtility.h"

namespace Arthmetic
{
	enum struct RoutineItemType
	{
		None,
		Value,
		FunctionValue,  //Was gonna allow these to go next to a value, but no, bad idea.
		RoutineValue,   //Same with these?
		Operator,
		OpOpen,
		OpClose
	};

	//You can just return c strings here, these are constant
	inline std::string PrintRoutineItemType(RoutineItemType type)
	{
		switch (type) {
		case RoutineItemType::Value:
			return "Value";
		case RoutineItemType::FunctionValue:
			return "Function Value";
		case RoutineItemType::Operator:
			return "Operator";
		case RoutineItemType::OpOpen:
			return "OpOpen";
		case RoutineItemType::OpClose:
			return "OpClose";
		case RoutineItemType::None:
			return "None";
		}
		return "Unregistered Function Item Type";
	}



	inline bool IsValidCodeChar(RoutineItemType current, RoutineItemType& last_type)
	{
		//As an added convience, it will automatically set the last type.
		
		switch (current) {
		case RoutineItemType::Operator:
			if (last_type == RoutineItemType::None || last_type == current)
				//None is defined only when it just starts, no regular operator should be there.
				return false;

			break;
		case RoutineItemType::FunctionValue:
		case RoutineItemType::Value:
			if (last_type == RoutineItemType::Value || last_type == RoutineItemType::FunctionValue)
				return false;

			break;

		case RoutineItemType::OpClose:
			if (last_type == RoutineItemType::OpOpen)
				return false;

			goto op_check;
		case RoutineItemType::OpOpen:
			if (last_type == RoutineItemType::OpClose)//Can't interpret these next to each other (yet)
				return false;

		op_check:
			if (last_type == RoutineItemType::None)
				return false;

		break_time:

			break;

		default:
			ARTHMETIC_LOGGER(error, "Unregistered Function Item Type, forced validity failure.");
			return false;
		}

		last_type = current;

		return true;
	}

	/*This is functional, just the above includes new checks.
	inline bool IsValidCodeChar(RoutineItemType current, RoutineItemType& last_type)
	{
		//As an added convience, it will automatically set the last type.
		logger::info("{} to {}", PrintRoutineItemType(current), PrintRoutineItemType(last_type));
		switch (current) {
		case RoutineItemType::FunctionValue:
			if (last_type == RoutineItemType::Value)
				return false;
		case RoutineItemType::Value:
		case RoutineItemType::Operator:
			if (last_type == current)
				return false;

			break;

		case RoutineItemType::OpClose:
			if (last_type == RoutineItemType::OpOpen)
				return false;

		case RoutineItemType::OpOpen:
		break_time:

			break;

		default:
			ARTHMETIC_LOGGER(error, "ERROR: Unregistered Function Item Type, forced validity failure.");
			return false;
		}

		last_type = current;

		return true;
	}
	//*/

	
}