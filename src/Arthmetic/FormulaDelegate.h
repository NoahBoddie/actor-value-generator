#pragma once


#include "Arthmetic/TargetType.h"
#include "Arthmetic/RoutineArgument.h"
#include "Arthmetic/IDelegate.h"

#include "Arthmetic/DelegateType.h"//Supposed to be in i delegate

namespace Arthmetic
{
	struct FormulaDelegate : public IDelegate, public RoutineItemFactoryComponent<FormulaDelegate, RoutineItemType::FunctionValue>
	{
		//This would probably just be a centralizer, likely will do nothing.

		union
		{
			IFormula* _formula;
			mutable char* _formulaName;
		};


		IFormula* GetFormula()
		{
			if (IsPartiallyValid() == false)
				return nullptr;

			return _formula;
		}


		//I have no idea why this needs the namespace specification.
		Arthmetic::DelegateType DelegateType() override { return DelegateType::Formula; }



		//Both of these runs need to share a single function.
		float Run(const Target target) override
		{
			IFormula* formula = GetFormula();

			ParameterList p_list(formula->_paramSettingList ? formula->_paramSettingList->size() : params.size());
			//ParameterList p_list(params.size());
			Target _target = _targeter->GetTargetSafe(target);
			logger::info("pre-two {} {}", params.size(), p_list.size());
			//Still rough
			for (int i = 0; i < params.size(); i++)
			{
				p_list[i] = params[i].ResolveArgument(nullptr);
			}

			logger::info("handle2");

			return formula->Run(_target, p_list);
		}

		void Run(RoutineArgument* argument, float& return_value, size_t& index) override
		{
			IFormula* formula = GetFormula();
			
			static int handle = 0;

			//logger::info("Other handle1 {}", handle++);
			Target _target = _targeter->GetTargetSafe(argument);

			ParameterList p_list(formula->_paramSettingList ? formula->_paramSettingList->size() : params.size());

			//logger::info("Other pre-two {} {}", params.size(), p_list.size());
			
			//Still rough
			for (int i = 0; i < params.size(); i++)
			{
				p_list[i] = params[i].ResolveArgument(argument);
				continue;

				if (p_list[i].GetType() == ArgumentType::Number)
					logger::info("formula number {} is {}", (int)p_list[i]._type, p_list[i].GetNumberParam(argument));
				else
					logger::info("formula type {}", (int)p_list[i]._type);
			}

			//logger::info("Other pre-handle2 {}", handle);

			return_value = formula->Run(_target, p_list);

			//logger::info("Other handle2 {}", handle--);
		}


		void LoadFromView(RecordIterator& it) override
		{
			std::string code{ it->view };
			
			SetData(code);
			
			it++;

			logger::info("Pc");

			auto before_it = it;

			while (MakeParameters(it) == false) {
				if (before_it == it)
					++it;

				before_it = it;
			}


			logger::info("Pp");
		}

		void OnDeclareOwner(std::deque<ArthmeticObject*>& owner_stack) override
		{
			IFormula* owner = dynamic_cast<IFormula*>(owner_stack.front());

			if (!owner)
			{
				ARTHMETIC_LOGGER(critical, "No formula found.");
				//This isn't so critical actually.
			}

			//_formula = formula;

			constexpr int space_taker = 0;

			for (auto& arg : params) {
				arg.SetOwner(owner, space_taker);
			}

			logger::debug("Formula Delegate ownership set.");
		}


		void OnLinkage(LinkerFlags link_type, bool& success) override
		{
			//if (link_type == LinkerFlags::Object)
			//	return;//I cannot stress how little I care about this rn.




			if (IsPartiallyValid() == false)
			{
				std::string code{ _formulaName };

				FreeData();

				if (formulaMap.contains(code) == true) {
					_formula = formulaMap[code];
					logger::info("Formula '{}' found and linked.", code);
				}
				else {
					logger::error("Formula '{}' not found", code);
					do_return(success = false);
				}
			}


			constexpr int space_taker = 0;

			bool link_param = link_type == LinkerFlags::External;
			bool link_obj = link_type == LinkerFlags::Object;

			for (auto& arg : params) {
				//There's likely a more elegant conditino but fuck it
				if (arg.IsParameter() == true && link_type == LinkerFlags::Object)
					continue;

				if (arg.IsParameter() == true && link_type == LinkerFlags::External || arg.IsObject() == true && link_type == LinkerFlags::Object)
					if (arg.SetOwner(nullptr, space_taker) == false)
						success = false;

				
			}
		}


		void SetData(std::string& str)
		{
			if (IsPartiallyValid() == true)
				return;

			FreeData();

			_formulaName = strdup(str.c_str());
		}

		void FreeData() const
		{
			if (IsPartiallyValid() == true || !_formulaName)
				return;

			logger::debug("freeing name '{}'", _formulaName);

			free(_formulaName);
		}

		~FormulaDelegate() { FreeData(); }

		RoutineItemType GetItemType() override { return RoutineItemType::FunctionValue; }
	};
}
