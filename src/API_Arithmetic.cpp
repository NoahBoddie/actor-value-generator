#pragma once

#include "Arthmetic/IFormula.h"
#include "API_Arithmetic.h"
#include "Arthmetic/TargetType.h"
#include "Arthmetic/FunctionInterface.h"//REALLY don't want this to be a thing btw, kinda shit.
namespace ArithmeticAPI
{
	using namespace Arthmetic;

	struct ArithmeticInterface : public CurrentInterface
	{
		Version GetVersion() override { return Version::Current; }

		//These can have the interesting ability of switching between resolved argument and delegate argument depending on source
		// or not.


		//These should also send a catchable exception if null or if the argument was in line with what it expects it to be.

		//Target will have to be defined here if there's no source, just for the transfers sake. It doesn't nothing special anyhow.
		Arthmetic::Target ArgAsObject(SWITCH_ARG* arg) override
		{
			if (!arg)
				throw InvalidArgError("Null Arg: Obj");

			if (arg->IsObject() == false)
				throw InvalidArgError("Not Obj");

			return arg->GetObjectParam();


		}

		float ArgAsNumber(SWITCH_ARG* arg) override
		{

			if (!arg)
				throw InvalidArgError("Null Arg: Num");

			if (arg->IsNumber() == false)
				throw InvalidArgError("Not Num");

			return arg->GetNumberParam();
		}

		const char* ArgAsString(SWITCH_ARG* arg) override
		{
			if (!arg)
				throw InvalidArgError("Null Arg: Str");

			if (arg->IsString() == false)
				throw InvalidArgError("Not Str");

			return arg->GetCStringParam();
		}

		void LinkNativeFunction(std::string str, NativeFormula function, ParamConstraints) override
		{
			logger::info("nat func link called: '{}'", str);
			
			IFormula* formula = formulaMap[str];

			auto func_intfc = dynamic_cast<FunctionInterface*>(formula);

			if (!func_intfc)
				return;

			//Currently, I can't actually do this because the types aren't right. But I'd like to edit that overtime.
			func_intfc->_callback = function;


		}
	};


	CurrentInterface* InferfaceSingleton()
	{
		static ArithmeticInterface intfc{};
		return &intfc;
	}
}