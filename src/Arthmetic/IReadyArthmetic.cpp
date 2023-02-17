#pragma once

#include "Arthmetic/IReadyArthmetic.h"
#include "Arthmetic/RoutineArgument.h"


namespace Arthmetic
{
	void IReadyArthmetic::RunImpl(RoutineArgument* arg, float& r, size_t&) 
	{ 
		Target target = arg ? arg->GetTarget() : Target();
		r = RunImpl(target);
	}
	float IReadyArthmetic::RunImpl(RoutineArgument* arg) 
	{
		Target target = arg ? arg->GetTarget() : Target();
		return RunImpl(target);
	}
}