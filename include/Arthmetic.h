#pragma once

//This is supposed to include all the arthmetic stuff so they don't have to do the including
// and so there's no need to include more than one needs too.
//#include <>




//#include <NEBUtil.h>

//All of this stuff will be included in a header that can be passed around, something to make it easy if I'm using skyrim
// but still leave room for other projects, or other targets.

#ifndef ARTH_OBJECT_TYPE
#define ARTH_OBJECT_TYPE void;
#endif


#ifndef ARTHMETIC_LOGGER
#define ARTHMETIC_LOGGER(mc_level, mc_text, ...) static_assert(false, "Arthmetic logger not set.")
#endif


#include <API_Arithmetic.h>

#include <Arthmetic/ArthmeticObject.h>
#include <Arthmetic/IReadyArthmetic.h>
#include <Arthmetic/ArthmeticValue.h>
#include <Arthmetic/IRoutine.h>
#include <Arthmetic/ArthmeticParser.h>
#include <Arthmetic/ArthmeticConstructor.h>
#include <Arthmetic/ArthmeticUtility.h>
#include <Arthmetic/TargetType.h>
#include <Arthmetic/DelegateArgument.h>
//#include <Arthmetic/DualOperator.h>
//#include <Arthmetic/FunctionDelegate.h>
//#include <Arthmetic/OpenOperator.h>
#include <Arthmetic/IOperator.h>
#include <Arthmetic/OperatorType.h>
#include <Arthmetic/RoutineDelegate.h>
#include <Arthmetic/RoutineItemType.h>
#include <Arthmetic/Subroutine.h>
#include <Arthmetic/ArgumentType.h>
#include <Arthmetic/Record.h>
#include <Arthmetic/IDirective.h>
#include <Arthmetic/IFormula.h>
#include <Arthmetic/FunctionInterface.h>
#include <Arthmetic/FormulaDelegate.h>
#include <Arthmetic/ParameterSetting.h>
#include <Arthmetic/IReadyArthmetic.h>
#include <Arthmetic/Constructor.h>


//#include <Arthmetic/ExtraData.h>

//Would like to make something like this
// TOML_IMPL_NAMESPACE_START which would be toml::impl

//The internal headers
#ifdef ARITHMETIC_SOURCE

#endif


//Temp space. Need more room.
	//FIND AN APPROPRIATE PLACE FOR THIS

	//*
