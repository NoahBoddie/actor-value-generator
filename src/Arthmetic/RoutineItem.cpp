#pragma once

#include "Arthmetic/RoutineItem.h"


namespace Arthmetic
{
	RoutineItem* RoutineItem::GetBack(RoutineArgument* argument) { return argument ? argument->GetItem(_position - 1) : nullptr; }
	RoutineItem* RoutineItem::GetFront(RoutineArgument* argument) { return argument ? argument->GetItem(_position + 1) : nullptr; }


	float RoutineItem::GetBackValue(RoutineArgument* argument) { return argument ? argument->GetItemValue(_position - 1) : 0; }
	float RoutineItem::GetFrontValue(RoutineArgument* argument) { return argument ? argument->GetItemValue(_position + 1) : 0; }
}