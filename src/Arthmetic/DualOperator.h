#pragma once

#include "Arthmetic/IOperator.h"
#include "Arthmetic/RoutineArgument.h"

namespace Arthmetic
{
	struct DualOperator : public IOperator//, public RoutineItemFactoryComponent<DualOperator, RoutineItemType::Operator>
	{
		//Most operators are this type, the exception being parantheses
		//Perform//Should take a back number, and a front number, then switch for which to do.

		float Operate(float back, float front)
		{
			switch (_operator) {
			case OperatorType::Subtract:
				ARTHMETIC_LOGGER(info, "{} - {} = {}", back, front, back - front);
				return back - front;
			case OperatorType::Addition:
				ARTHMETIC_LOGGER(info, "{} + {} = {}", back, front, back + front);
				return back + front;
			case OperatorType::Division:
				ARTHMETIC_LOGGER(info, "{} / {} = {}", back, front, back / front);
				return back / front;
			case OperatorType::Multiply:
				ARTHMETIC_LOGGER(info, "{} * {} = {}", back, front, back * front);
				return back * front;
			case OperatorType::Modulo:
			{

				int _1 = static_cast<int32_t>(back);
				int _2 = static_cast<int32_t>(front);
				int _r = _1 % _2;
				ARTHMETIC_LOGGER(info, "{} % {} = {}", _1, _2, _r);
				return _r;
			}
			case OperatorType::Exponent:
				ARTHMETIC_LOGGER(info, "{} ^ {} = {}", back, front, pow(back, front));
				return pow(back, front);
			default:
				//Operation type is invalid, will not be NAN because that's hard to handle.
				return 0;
			}
		}

		void Run(RoutineArgument* argument, float& return_value, size_t& index) override
		{
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

		void LoadFromView(RecordIterator& data) override
		{
			switch (data->view[0])
			{
			case '*':
				logger::info("construct symbol \'*\'");
				_operator = OperatorType::Multiply;
				break;

			case '+':
				logger::info("construct symbol \'+\'");
				_operator = OperatorType::Addition;
				break;

			case '-':
				logger::info("construct symbol \'-\'");
				_operator = OperatorType::Subtract;
				break;

			case '/':
				logger::info("construct symbol \'/\'");
				_operator = OperatorType::Division;
				break;

			case '^':
				logger::info("construct symbol \'^\'");
				_operator = OperatorType::Exponent;
				
				break;

			case '%':
				logger::info("construct symbol \'%\'");
				_operator = OperatorType::Modulo;
				break;

			default:
				//cout << "failure";
				ARTHMETIC_LOGGER(info, "Failure to handle operator, value \'{}\'", data->view[0]);
				throw nullptr;
			}
		}

		RoutineItemType GetItemType() override { return RoutineItemType::Operator; }

		//DualOperator(uint16_t pos, OperatorType op) :
		//	IOperator(pos, RoutineItemType::Operator, op) {}
	};
}