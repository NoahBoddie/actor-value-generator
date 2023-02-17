#pragma once

#include "Arthmetic/ArthmeticUtility.h"
#include "Arthmetic/TargetType.h"
#include "Arthmetic/IRoutine.h"
#include "Arthmetic/RoutineItem.h"
#include "Arthmetic/DelegateArgument.h"

namespace Arthmetic
{
	//I think this needs to carry around parameters, something that can be refered to
	struct RoutineArgument : public BasicSelector
	{
		RoutineArgument* _parent = nullptr;

		ParameterList* _args{ nullptr };

		bool _finished = false;

		//Make a unique_ptr when you get the chance plz
		//For now I'm using vectors, and I'll probably go back when I'm done with this dog shit site.
		std::vector<float> _resolvedItems;

		//One shouldn't set resolves directly, needs functions.

		//One is given the vector these are stored in., then the range and the float is resolved
		//I want to treat these like const, you're not allowed to edit these from an argument. Build that in when you can.
		
		//std::vector<std::unique_ptr<RoutineItem>>& _functionItems;
		decltype(IRoutine::_code)& _functionItems;
		
		size_t _range;


		//RE::Actor* _target = nullptr;

		//RoutineArgument(RE::Actor* target, IRoutine* routine) : _functionItems(routine->_code)
		
		bool HasResolveValue(size_t index) { return !isinf(_resolvedItems[index]); }

		bool HasResolveValue(size_t index, float& value)
		{
			if (GetResolveEnd(index) == false)
				return false;
			//cout<< "(!!!" << index << ")";
			value = _resolvedItems[index];

			return true;
		}

		bool GetResolveEnd(size_t& index)
		{
			//returns the index if it's the value if there is no value to jump override
			// returns the index it's pointing to if it is.

			float raw_value = _resolvedItems[index];

			if (isinf(raw_value) == true) {
				//cout << "\nhit(A) " << index << " = " << raw_value << std::endl;
				return false;
			}

			if (isnan(raw_value) == true) {
				uint32_t position = ArthmeticUtility::ConvertToValue(raw_value);

				//cout << position << " v " << index << "|";
				//cout << "\nhit(B) " << index <<"(" << position << ")" << " = new, " << ConvertToValue(_resolvedItems[position]) << std::endl;
				index = position;

				return true;
			} else {
				//cout << "\nhit(C) " << index << " = " << raw_value << std::endl;
				return true;
			}

			return false;

			if (isnan(_resolvedItems[index]) == true) {
				return false;
			}

			if (index == _range - 1) {
				//Under these circumstances, there's no way there's anything past it.
				return true;
			}

			float raw_value1 = _resolvedItems[index];

			float raw_value2 = _resolvedItems[index + 1];

			if (isnan(raw_value2) == true) {
				return true;
			}

			//Note, it's very unlikely (pretty much not possible) for 2 different NaN values to end up next to each other.
			// So if one is pointing to another that has a value,or both have the same value, then the index will point to the result

			if (raw_value1 == raw_value2) {
				index = llround(raw_value1);
				return true;
			} else {
				index = llround(raw_value2);
				return true;
			}
		}

		bool GetResolveBegin(size_t& index)
		{
			//returns the index if it's the value if there is no value to jump override
			// returns the index it's pointing to if it is.

			if (index == 0) {
				//Under these circumstances, there's no way there's anything past it.
				return true;
			}

			float raw_value = _resolvedItems[index];

			if (isinf(raw_value) == true) {
				//cout << "(" << index << "not resolve)";
				return false;
			}

			size_t return_index = index - 1;
			//cout << "*HIT " << return_index << std::endl;
			while (return_index > 0) {
				//cout << "**HIT " << return_index << std::endl;

				float raw_value = _resolvedItems[return_index];

				if (isinf(raw_value) == true) {
					index = return_index + 1;
					//cout << "HIT " << index << "/" << return_index << " >> " << raw_value;
					return true;
				}
				return_index--;
			}

			index = 0;

			return true;
		}

		void SetResolveValueOut(float value, size_t& begin, size_t& end)
		{
			//Here conduct size checks, check begin is begin and end is end.

			//Next, Get the resolve beginning on begin, and the end on end, then assign all values, with the last being the value.
			//cout <<  "START" << std::endl;
			GetResolveBegin(begin);
			GetResolveEnd(end);

			//cout <<  "begin: " << begin << ", end: " << end << std::endl;

			float packed_mantissa = ArthmeticUtility::ConvertToNaN(end);

			if (begin == 0 && end == _range - 1) {
				_finished = true;
			}

			for (int i = begin; i < end; i++) {
				_resolvedItems[i] = packed_mantissa;
			}

			_resolvedItems[end] = value;
		}

		void SetResolveValue(float value, size_t begin, size_t end)
		{
			SetResolveValueOut(value, begin, end);
		}

		RoutineItem* GetItem(size_t index)
		{
			if (index >= _range)
				return nullptr;
			
			return static_cast<RoutineItem*>(_functionItems[index].get());
		}

		float GetItemValue(size_t index)
		{
			auto* item = GetItem(index);

			if (!item)
				return 0;

			float return_value{};

			if (HasResolveValue(index, return_value) == false)
				return_value = item->RunImpl(this);

			return return_value;
		}

		float ProcessFunction()
		{
			//if there are no operators, just return a value. Do later though.

			return ProcessRange(0);
		}

		float ProcessRange(int pos = 0, int range = -1)
		{
			OperatorType search_op = OperatorType::Jump_Parenthesis;

			//Confusing I know, but _range is size. I'm sorry future me.

			//Stop using size_t. fr.
			int tmp_range = _range;

			size_t limit = range < 0 ? _range : fmin(pos + range, tmp_range);

			if (limit - pos <= 1) {  //If exactly the same or theres like a difference of 1.
				auto* item = GetItem(pos);

				if (!item) {
					//report and continue
					//cout << "Item at " << pos << " is null.";
					return 0;
				}

				return item->RunImpl(this);
			}

			float return_value = 0;
			//This shouldn't need or to function? But this loop won't exit.
			while (search_op != OperatorType::Invalid && !_finished) {
				for (size_t i = pos; i < limit && !_finished; i++) {
					auto* item = GetItem(i);

					if (!item) {
						//report and continue
						//cout << "Item at " << i << " is null.";
						continue;
					}

					auto op = item->GetOperator();

					if (ShouldOperate(search_op, op) == false)  //op != search_op)//
						continue;
					//*/

					//This SHOULD check for both resolution and jump past the resolve phase.
					if (GetResolveEnd(i) == true) {
						continue;
					}
					//cout << "running: " << i;
					
					item->RunImpl(this, return_value, i);

					//return 0;
				}
				auto old = search_op;
				//cout << "dhjit";

				search_op--;
			}

			if (_finished)
				logger::info("complete {}" , return_value);
			else
				logger::error("NOT Completed.", return_value);

			return return_value;
		}

		/*Do not fucking care right now.
    RoutineArgument(ArthmeticArguement& argument, vector<RoutineItem*> items)
    {

        //No real importance atm.
        _parent = &argument;

        _range = items.size();
        _functionItems = items;
        _resolvedItems = vector<float>(_range, INFINITY);
    }
    //*/


		//Ways to create a routine argument.
		//A copy of a routine argument, in which, it will get it's variables but also it's top face argument (Either being it or it's source parent)
		//It can be created through a routine, but really any way that allows you to copy code.

		RoutineArgument* AsRoutineArg() override { return this; }

		RoutineArgument* GetOriginalArgument()
		{
			return !this ? nullptr : _parent ? 
				_parent : this;
		}

		RoutineArgument(IRoutine* routine, Target tar, ParameterList* arguments = nullptr) :
			_functionItems(routine->_code), BasicSelector(tar), _args(arguments)
		{
			//Self has to exist. Over time, I'd like to just basically allow
			//_targets = new ArgTargetParams();//Maybe make one that invents its own, but I'd like to set a precendent of needing to use a full arg list.
			//_targets[ArthmeticTargetType::Self] = self;

			//This needs to be able to handle a target at some point.
			//_target = target;

			_range = _functionItems.size();
			//_functionItems = items;
			_resolvedItems = std::vector<float>(_range, INFINITY);
		}

		RoutineArgument(RoutineArgument* argument) :
			_functionItems(argument->_functionItems), BasicSelector(argument->GetTarget()), _args(argument->_args)
		{
			_parent = argument->GetOriginalArgument();

			_range = _functionItems.size();
			//_functionItems = items;
			_resolvedItems = std::vector<float>(_range, INFINITY);
		}


	};
}