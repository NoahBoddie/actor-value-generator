#pragma once

#include "Arthmetic/IFormula.h"
#include "Arthmetic/RoutineItem.h"

namespace Arthmetic
{
	struct IRoutine : public IFormula
	{
		
		//This is supposed to hold code.

		//Instead of a vector, this should likely be a unique_ptr that has it's collection managed by a dynamically generated span.
		std::vector<std::unique_ptr<RoutineItem>> _code{};
		//std::vector<RoutineItem*> _code{};

		//I routine similarly needs to have an ownership over the set of data that's spawned it.
		// Whenever they change hands, the ownership will change. On reflection though, no such thing is really needed for routines.
		// Mainly because they're generally detacted, and you own pointers of them. But if anything is ever stored pure, you know what
		// to do.

		//NOTE, the vector could be unique ptrs, no idea how that will interact though. I'll have to change a bit for that too so
		// fat shrug for now. TLDR, this may be temp.
		~IRoutine() override
		{
			//*
			return;
			/*/
			for (auto& item : _code)
			{
				delete item;
			}
			//*/

		}
	};
}