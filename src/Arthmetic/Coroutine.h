#pragma once


#include "Arthmetic/TargetType.h"
#include "Arthmetic/IRoutine.h"
//#include "Arthmetic/IReadyArthmetic.h"
#include "Arthmetic/RoutineArgument.h"

namespace Arthmetic
{
	struct Coroutine : public IRoutine
	{
		//Note, currently coroutine is refered to as a type that can run on its own, but this is false.
		// So subroutine will need to derive from ReadyArthmetic as well as ArthmeticObject, while IRoutine will be a different
		// sort of object. it will be something that stores routine items, but it won't use run functions.




		void Run(Target target, ParameterList p_list, float& result) override
		{
			float default_value = p_list.defaultValue.IsResolved() ? p_list.defaultValue.GetNumberParam() : defaultValue;

			logger::info("mouse 1");

			for (int i = 0; i < p_list.size(); i++)
			{
				if (p_list[i].IsResolved() == false) {
					//Need to check if this shit is actually resolved before doing this shit btw.
					logger::info("other before type {}", (int)p_list[i]._type);
					p_list[i] = (*_paramSettingList)[i].defaultValue.ResolveArgument(nullptr);

					if (p_list[i].GetType() == ArgumentType::Number)
						logger::info("other number {} is {}", (int)p_list[i]._type, p_list[i].GetNumberParam());
					else
						logger::info("other type {}", (int)p_list[i]._type);
				}
				else
					logger::info("type {}", (int)p_list[i]._type);
			}

			logger::info("mouse 2");

			RoutineArgument args = RoutineArgument(this, target, &p_list);

			result = args.ProcessFunction();
		}

		void OnDeclareOwner(std::deque<ArthmeticObject*>& owner_stack) override
		{
			//This is apparently, pretty ambiguious because of the set up of IReadyArth. Go figure.

			logger::info("Coroutine encountered, items: {}, owner: {}", _code.size(), owner_stack.front() == this);

			for (auto& item : _code){
				if (item) item->DeclareOwner(owner_stack, this);
			}
		}

		LinkerFlags GetLinkerFlags() const override
		{
			if (!_paramSettingList)
				return LinkerFlags::None;

			//I sorta forgot that by nature this will have to set up with external objects. Whoops.
			//LinkerFlags result_flags = LinkerFlags::None;

			//constexpr LinkerFlags cond_flag = LinkerFlags::Object;

			for (auto& arg : *_paramSettingList)
			{
				if (arg.HasDefault() == arg.defaultValue.IsObject() == true)
					return LinkerFlags::Object;

			}

			return LinkerFlags::None;
		}

		Coroutine(ParameterSettingList parameters, std::vector<RoutineItem*> try_code)
		{
			_paramSettingList.reset(new ParameterSettingList(parameters));
			//_code = try_code;
			_code = std::vector<std::unique_ptr<RoutineItem>>(try_code.cbegin(), try_code.cend());
		}
	};
}