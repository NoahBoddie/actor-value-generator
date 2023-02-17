#pragma once

#include "Arthmetic/ArthmeticUtility.h"
#include "Arthmetic/OperatorType.h"
#include "Arthmetic/RoutineItem.h"
#include "Arthmetic/RoutineArgument.h"
namespace Arthmetic
{
	//I think this item can actually be a single operator. The functionality while different can be largely divided.

	struct IOperator : public RoutineItem
	{
		OperatorType _operator{ OperatorType::Invalid };

		OperatorType GetOperator() override { return _operator; }

		//IOperator(uint16_t pos, RoutineItemType type, OperatorType op) :
		//	RoutineItem(pos, type), _operator(op) {}
	};


	struct Operator : public RoutineItem, public RoutineItemFactoryComponent<Operator, RoutineItemType::Operator>
	{
	protected:
		OperatorType _operator{ OperatorType::Invalid };
		//I'm thinking that this stores the length to which it will go, so i can stop using Close as an operator (Its kinda wasted space).

		float Operate(float back, float front)
		{
			int iback = static_cast<int32_t>(back);
			int ifront = static_cast<int32_t>(front);

			switch (_operator) {
			case OperatorType::Subtract:
				ARTHMETIC_LOGGER(debug, "{} - {} = {}", back, front, back - front);
				return back - front;
			case OperatorType::Addition:
				ARTHMETIC_LOGGER(debug, "{} + {} = {}", back, front, back + front);
				return back + front;
			case OperatorType::Division:
				ARTHMETIC_LOGGER(debug, "{} / {} = {}", back, front, back / front);
				return back / front;
			case OperatorType::Multiply:
				ARTHMETIC_LOGGER(debug, "{} * {} = {}", back, front, back * front);
				return back * front;
			case OperatorType::Modulo:
				ARTHMETIC_LOGGER(debug, "{} % {} = {}", iback, ifront, iback % ifront);
				return iback % ifront;

			case OperatorType::Exponent:
				ARTHMETIC_LOGGER(debug, "{} ^ {} = {}", back, front, pow(back, front));
				return pow(back, front);

			case OperatorType::BitwiseXOR:
				ARTHMETIC_LOGGER(debug, "{} XOR {} = {}", back, front, iback ^ ifront);
				return iback ^ ifront;

			case OperatorType::BitwiseAND:
				ARTHMETIC_LOGGER(debug, "{} & {} = {}", back, front, iback & ifront);
				return iback & ifront;

			case OperatorType::BitwiseOR:
				ARTHMETIC_LOGGER(debug, "{} | {} = {}", back, front, iback | ifront);
				return iback | ifront;

			case OperatorType::RightShift:
				ARTHMETIC_LOGGER(debug, "{} >> {} = {}", back, front, iback >> ifront);
				return iback >> ifront;

			case OperatorType::LeftShift:
				ARTHMETIC_LOGGER(info, "{} << {} = {}", back, front, iback << ifront);
				return iback << ifront;

			case OperatorType::LogicalOR:
				ARTHMETIC_LOGGER(debug, "{} || {} = {}", back, front, back || front);
				return back || front;

			case OperatorType::LogicalAND:
				ARTHMETIC_LOGGER(debug, "{} && {} = {}", back, front, back && front);
				return back && front;

			case OperatorType::GreaterThan:
				ARTHMETIC_LOGGER(debug, "{} > {} = {}", back, front, back > front);
				return back > front;


			case OperatorType::LesserThan:
				ARTHMETIC_LOGGER(debug, "{} < {} = {}", back, front, back < front);
				return back < front;


			case OperatorType::GreaterOrEqual:
				ARTHMETIC_LOGGER(debug, "{} >= {} = {}", back, front, back >= front);
				return back >= front;


			case OperatorType::LesserOrEqual:
				ARTHMETIC_LOGGER(debug, "{} <= {} = {}", back, front, back <= front);
				return back <= front;


			case OperatorType::EqualTo:
				ARTHMETIC_LOGGER(debug, "{} == {} = {}", back, front, back == front);
				return back == front;


			case OperatorType::NotEqualTo:
				ARTHMETIC_LOGGER(debug, "{} != {} = {}", back, front, back != front);
				return back != front;


			default:
				//Operation type is invalid, will not be NAN because that's hard to handle.
				return 0;
			}
		}

	public:

		OperatorType GetOperator() override { return _operator; }


		void Run(RoutineArgument* argument, float& return_value, size_t& index) override
		{
			get_switch (_operator)
			{
			case OperatorType::ParOpen:
			//case OperatorType::ParClose://This should be an error type, this does nothing.
			{
				uint32_t op_lock = 0;

				RoutineItem* item = this;

				for (int i = 1; item; i++)
				{
					size_t close_position = _position + i;

					item = argument->GetItem(close_position);

					if (!item) {
						return;
					}

					auto op = item->GetOperator();

					if (op == OperatorType::ParOpen) {
						op_lock++;
						continue;
					}

					if (op != OperatorType::ParClose) {
						continue;
					}
					else if (op_lock) {
						op_lock--;
						continue;
					}


					//Try to guard against a proceedure being too close

					return_value = argument->ProcessRange(_position + 1, i - 1);

					//For the love of god, stop using size.
					size_t tmp_position = _position;

					argument->SetResolveValueOut(return_value, tmp_position, close_position);
					index = close_position;
				}
			}
			break;

			case OperatorType::BitwiseXOR:
			case OperatorType::BitwiseOR:
			case OperatorType::LogicalOR:
			case OperatorType::BitwiseAND:
			case OperatorType::LogicalAND:
			case OperatorType::RightShift:
			case OperatorType::LeftShift:
			case OperatorType::GreaterThan:
			case OperatorType::LesserThan:
			case OperatorType::GreaterOrEqual:
			case OperatorType::LesserOrEqual:
			case OperatorType::EqualTo:
			case OperatorType::NotEqualTo:
			case OperatorType::Addition:
			case OperatorType::Division:
			case OperatorType::Exponent:
			case OperatorType::Modulo:
			case OperatorType::Multiply:
			case OperatorType::Subtract:
			{
				handle_operate:

				float back = GetBackValue(argument);
				
				float front = GetFrontValue(argument);

				float resolve_value = Operate(back, front);

				size_t back_index = _position - 1;
				size_t front_index = _position + 1;

				//cout << "\nback_index: " << back_index << ", front_index: " << front_index;

				argument->SetResolveValueOut(resolve_value, back_index, front_index);

				return_value = resolve_value;
				index = front_index;
			}
			break;

			default:
				break;//Unhandled operator. Probably throw some error.
			}
		}

		RoutineItemType GetItemType() override { return RoutineItemType::Operator; }

		void LoadFromView(RecordIterator& data) override
		{

			switch (hash(data->view))
			{
			case "|"_h:
			case "OR"_h:
				logger::debug("construct symbol '|' / 'OR'");
				_operator = OperatorType::BitwiseOR;
				break;

			case "or"_h:
			case "||"_h:
				logger::debug("construct symbol '|' / 'or'");
				_operator = OperatorType::LogicalOR;
				break;

			case "&"_h:
			case "AND"_h:
				logger::debug("construct symbol '&' / 'AND'");
				_operator = OperatorType::BitwiseAND;
				break;

			case"&&"_h:
			case "and"_h:
				logger::debug("construct symbol '&&' / 'and'");
				_operator = OperatorType::LogicalAND;
				break;

			case ">>"_h:
				logger::debug("construct symbol '>>'");
				_operator = OperatorType::RightShift;
				break;

			case "<<"_h:
				logger::debug("construct symbol '<>=><'");
				_operator = OperatorType::LeftShift;
				break;

			case "!="_h:
				logger::debug("construct symbol '!='");
				_operator = OperatorType::NotEqualTo;
				break;

			case "<"_h:
				logger::debug("construct symbol '<'");
				_operator = OperatorType::LesserThan;
				break;

			case ">"_h:
				logger::debug("construct symbol '>'");
				_operator = OperatorType::GreaterThan;
				break;

			case "<="_h:
				logger::debug("construct symbol '<='");
				_operator = OperatorType::LesserOrEqual;
				break;

			case ">="_h:
				logger::debug("construct symbol '>='");
				_operator = OperatorType::GreaterOrEqual;
				break;

			case "=="_h:
				logger::debug("construct symbol '=='");
				_operator = OperatorType::EqualTo;
				break;

			case "^^"_h:
			case "XOR"_h:
				logger::debug("construct symbol '^^' / 'OR'");
				_operator = OperatorType::BitwiseXOR;
				break;

			case "("_h:
				logger::debug("construct symbol \'(\'");
				_operator = OperatorType::ParOpen;
				//_inner = true;
				break;

			case ")"_h:
				logger::debug("construct symbol \')\'");
				_operator = OperatorType::ParClose;
				//_inner = false;
				break;

			case "*"_h:
				logger::debug("construct symbol \'*\'");
				_operator = OperatorType::Multiply;
				break;

			case "+"_h:
				logger::debug("construct symbol \'+\'");
				_operator = OperatorType::Addition;
				break;

			case "-"_h:
				logger::debug("construct symbol \'-\'");
				_operator = OperatorType::Subtract;
				break;

			case "/"_h:
				logger::debug("construct symbol \'/\'");
				_operator = OperatorType::Division;
				break;

			case "^"_h:
				logger::debug("construct symbol \'^\'");
				_operator = OperatorType::Exponent;

				break;

			case "%"_h:
				logger::debug("construct symbol \'%\'");
				_operator = OperatorType::Modulo;
				break;

			default:
				//cout << "failure";
				ARTHMETIC_LOGGER(info, "Failure to handle operator, value \'{}\'", data->view[0]);
				throw nullptr;
			}
		}


	};
}