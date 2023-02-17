#pragma once

#include "Arthmetic/IOperator.h"
#include "Arthmetic/RoutineArgument.h"
#include "Arthmetic/RoutineItemType.h"//IOp has this, shouldn't be here, once that shit actually works.

namespace Arthmetic
{
    //[[deprecated("Specification of operator unnecessary. Use 'Operator' instead.")]]
	struct OpenOperator : public IOperator//,
        //public RoutineItemFactoryComponent<OpenOperator, RoutineItemType::OpOpen>,
        //public RoutineItemFactoryComponent<OpenOperator, RoutineItemType::OpClose>
	{
		//If it's inner it's left, if its outer its right. As a inner, for every inner we pass,
		// that's another outter we must pass.
		bool _inner{};

		void Run(RoutineArgument* argument, float& return_value, size_t& index) override
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


                //What should it do if it never reaches this area?

                return;
            }
        }


        void LoadFromView(RecordIterator& data) override
        {
            switch (data->view[0])
            {
            case '(':
                logger::info("construct symbol \'(\'");
                _operator = OperatorType::ParOpen;
                _inner = true;
                break;

            case ')':
                logger::info("construct symbol \')\'");
                _operator = OperatorType::ParClose;
                _inner = false;
                
                break;

            default:
                ARTHMETIC_LOGGER(info, "Failure to handle operator, value \'{}\'", data->view[0]);
                throw nullptr;
            }
        }

        RoutineItemType GetItemType() override { return _inner ?  RoutineItemType::OpOpen : RoutineItemType::OpClose; }

		//OpenOperator(uint16_t pos, bool in) :
		//	_inner(in),
		//	IOperator(pos, in ? RoutineItemType::OpOpen : RoutineItemType::OpClose, in ? OperatorType::ParOpen : OperatorType::ParClose)
		//{}
	};

}