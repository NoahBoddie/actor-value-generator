#pragma once

namespace Arthmetic
{
	enum OperatorType
	{
		Invalid,
		//Sorting area
		BitwiseXOR,
		BitwiseOR,
		BitwiseAND,
		RightShift,
		LeftShift,
		//These NOT operators are a bit too spicy for this right now, as they're single way operators. That'll need work.
		//BitwiseNOT,
		//LogicalNOT,
		GreaterThan,
		LesserThan,
		GreaterOrEqual,
		LesserOrEqual,
		EqualTo,
		NotEqualTo,
		LogicalOR,
		LogicalAND,

		//Sorting area
		Subtract,
		Addition,
		Division,
		Multiply,
		Modulo,
		Exponent,
		ParClose,
		ParOpen,


		//These jump values make ++ and -- easier
		Jump_Parenthesis = ParOpen,
		//Jump_Not = BitwiseNOT,
		Jump_3D = Exponent,
		Jump_2D = Multiply,
		Jump_1D = Addition,
		Jump_Shift = RightShift,
		Jump_Relation = GreaterThan,
		Jump_Equality = EqualTo,
		Jump_BitAND = BitwiseAND,
		Jump_BitXOR = BitwiseXOR,
		Jump_BitOR = BitwiseOR,
		Jump_LogAND = LogicalAND,
		Jump_LogOR = LogicalOR,
	};





	inline bool ShouldOperate(OperatorType target_op, OperatorType query_op)
	{
		if (query_op == OperatorType::Invalid)
			return false;
		

		bool switch_type = true;

		if (switch_type) {
			switch (target_op) {
			
			//*
			case OperatorType::GreaterThan:
			case OperatorType::LesserThan:
			case OperatorType::GreaterOrEqual:
			case OperatorType::LesserOrEqual:
				return ContainsEnum<OperatorType, 
					OperatorType::GreaterThan,
					OperatorType::LesserThan,
					OperatorType::GreaterOrEqual,
					OperatorType::LesserOrEqual>(query_op);

			case OperatorType::EqualTo:
			case OperatorType::NotEqualTo:
				return ContainsEnum<OperatorType, 
					OperatorType::EqualTo,
					OperatorType::NotEqualTo>(query_op);

			case OperatorType::RightShift:
			case OperatorType::LeftShift:
				return ContainsEnum<OperatorType,
					OperatorType::RightShift,
					OperatorType::LeftShift>(query_op);
			//*/

			case OperatorType::Addition:
			case OperatorType::Subtract:
				if (query_op == OperatorType::Addition || query_op == OperatorType::Subtract)
					return true;

				goto def;
			case OperatorType::Multiply:
			case OperatorType::Division:
			case OperatorType::Modulo:
				if (query_op == OperatorType::Modulo || query_op == OperatorType::Multiply || query_op == OperatorType::Division)
					return true;

				goto def;
			default:
			def:
				if (target_op == query_op)
					return true;
			}

			return false;
		}



		if (target_op == query_op)
			return true;
		else if (target_op == OperatorType::Addition && query_op == OperatorType::Subtract)
			return true;
		else if (target_op == OperatorType::Addition && query_op == OperatorType::Subtract)
			return true;

		return false;
	}

	inline OperatorType& operator++(OperatorType& type, int)
	{
		switch (type)
		{
		case OperatorType::Jump_Parenthesis:
			return type = OperatorType::Invalid;
		
		//case OperatorType::Jump_Not:
		//	return type =  OperatorType::Jump_Parenthesis;
		
		case OperatorType::Jump_3D:
			//return type = OperatorType::Jump_Not;
			return type = OperatorType::Jump_Parenthesis;

		case OperatorType::Jump_2D:
			return type = OperatorType::Jump_3D;

		case OperatorType::Jump_1D:
			return type = OperatorType::Jump_2D;

		case OperatorType::Jump_Shift:
			return type = OperatorType::Jump_1D;

		case OperatorType::Jump_Relation:
			return type = OperatorType::Jump_Shift;

		case OperatorType::Jump_Equality:
			return type = OperatorType::Jump_Relation;

		case OperatorType::Jump_BitAND:
			return type = OperatorType::Jump_Equality;

		case OperatorType::Jump_BitXOR:
			return type = OperatorType::Jump_BitAND;

		case OperatorType::Jump_BitOR:
			return type = OperatorType::Jump_BitXOR;

		case OperatorType::Jump_LogAND:
			return type = OperatorType::Jump_BitOR;

		case OperatorType::Jump_LogOR:
			return type = OperatorType::Jump_LogAND;
		}

		return type = OperatorType::Invalid;

		switch (type) {
		case OperatorType::Subtract:
			return type = OperatorType::Addition;

		case OperatorType::Addition:
			return type = OperatorType::Division;

		case OperatorType::Division:
			return type = OperatorType::Multiply;

		case OperatorType::Multiply:
			return type = OperatorType::Modulo;

		case OperatorType::Modulo:
			return type = OperatorType::Exponent;

		case OperatorType::Exponent:
			return type = OperatorType::ParOpen;

		case OperatorType::ParClose:
		case OperatorType::ParOpen:
			return type = OperatorType::Invalid;
		}

		return type = OperatorType::Invalid;
	}

	inline OperatorType& operator--(OperatorType& type, int)
	{
		if (1 == 1)
		{
			switch (type)
			{
			case OperatorType::Jump_Parenthesis:
				//return type = OperatorType::Jump_Not;
				return type = OperatorType::Jump_3D;

				//case OperatorType::Jump_Not:
				//	return type =  OperatorType::Jump_3D;

			case OperatorType::Jump_3D:
				return type = OperatorType::Jump_2D;

			case OperatorType::Jump_2D:
				return type = OperatorType::Jump_1D;

			case OperatorType::Jump_1D:
				return type = OperatorType::Jump_Shift;

			case OperatorType::Jump_Shift:
				return type = OperatorType::Jump_Relation;

			case OperatorType::Jump_Relation:
				return type = OperatorType::Jump_Equality;

			case OperatorType::Jump_Equality:
				return type = OperatorType::Jump_BitAND;

			case OperatorType::Jump_BitAND:
				return type = OperatorType::Jump_BitXOR;

			case OperatorType::Jump_BitXOR:
				return type = OperatorType::Jump_BitOR;

			case OperatorType::Jump_BitOR:
				return type = OperatorType::Jump_LogAND;

			case OperatorType::Jump_LogAND:
				return type = OperatorType::Jump_LogOR;

			case OperatorType::Jump_LogOR:
				//default:
				return type = OperatorType::Invalid;
			}

			return type = OperatorType::Invalid;
		}

		switch (type) {
		case OperatorType::Subtract:
			return type = OperatorType::Invalid;

		case OperatorType::Addition:
			return type = OperatorType::Subtract;

		case OperatorType::Division:
			return type = OperatorType::Addition;

		case OperatorType::Multiply:
			return type = OperatorType::Division;

		case OperatorType::Modulo:
			return type = OperatorType::Multiply;

		case OperatorType::Exponent:
			return type = OperatorType::Modulo;

		case OperatorType::ParClose:
		case OperatorType::ParOpen:
			return type = OperatorType::Exponent;
		}

		return type = OperatorType::Invalid;
	}

}