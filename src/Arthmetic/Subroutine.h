#pragma once

#include "Arthmetic/IRoutine.h"
#include "Arthmetic/IReadyArthmetic.h"

namespace Arthmetic
{
	struct Subroutine : public IRoutine, public IReadyArthmetic
	{
		//This is the type thats used purely in a statement.
		//This shit has some severe ambiguity with ready arthmetic and iroutine.
		

		// I would wager, no convience functions plz.


		float Run(Target target) override
		{
			logger::debug("Fire");
			RoutineArgument arg = RoutineArgument(this, target);

			return arg.ProcessFunction();
		}
		//This sticks Im sure, this should make a new routine based on the last one.
		void Run(RoutineArgument* argument, float& return_value, size_t& index)
		{
			//What this actually does instead of using this one, it a new one from this.
			RoutineArgument new_argument(argument);
			return_value = new_argument.ProcessFunction();
		}
		
		//If it ever did get a parameter list, it wouldn't do anything, subroutines cannot own parameters
		void Run(Target target, ParameterList list, float& result) override { result = Run(target); }


		void OnDeclareOwner(std::deque<ArthmeticObject*>& owner_stack) override
		{
			//This is apparently, pretty ambiguious because of the set up of IReadyArth. Go figure.
			IReadyArthmetic* self = this;

			logger::debug("subroutine encountered, items: {}, owner: {}", _code.size(), owner_stack.front() == this);

			for (auto& item : _code) {//This seems to have an erroneous entry in the end.
				if (item) item->DeclareOwner(owner_stack, this);
			}

			logger::debug("Subroutine finished declaring ownership");
		}


		Subroutine(std::vector<RoutineItem*> try_code)
		{
			//_code = try_code;
			_code = std::vector<std::unique_ptr<RoutineItem>>(try_code.cbegin(), try_code.cend());
		}

	};
}