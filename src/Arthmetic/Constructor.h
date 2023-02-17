#pragma once

#include "Arthmetic/Coroutine.h"
#include "Arthmetic/Subroutine.h"

//I know there's a file called constructor already. I'm deleting that and handing the means of production to routines. These are supposed
// to sort out the efficiency process of piece by piece assembly. Perhaps Assembler is a better name. Or factory, but that doesn't work
// because of the lack of objects.

//At this point, I'm unsure which is more beneath regard these naming conventions or my own desire to live. I'm just trying to get through
// the day, please leave me alone god damn it.


//tldr without the shit that makes me want to off myself, this is supposed to be the
// thing the user uses, not the internal stuff.
namespace Arthmetic
{

	//Will not be doing this just yet.
	//inline Subroutine CreateTemporarySubroutine() { return Subroutine({}); };

	//These should probably use records instead of direct strings like this

	//FIND AN APPROPRIATE PLACE FOR THIS

	//*

	//Should have another version that is basically create ready arthmetic..
	

	inline FunctionInterface* CreateFormula(ParsingArgumentList parameters)
	{
		FunctionInterface* func_intfc = nullptr;

		if (parameters.size() == 0) {
			func_intfc = new FunctionInterface();
		}
		else {
			auto parameter_list = CreateParameterSettings(parameters);
			func_intfc = new FunctionInterface(parameter_list);
		}

		 

		//routine->LoadFromViewImpl(it);

		func_intfc->DeclareOwner();

		logger::debug("Function Interface Finished Construction.");

		return func_intfc;
	}


	inline Coroutine* CreateFormula(std::string& code, ParsingArgumentList parameters)
	{
		Record record = InitialParse(code);


		RecordIterator it = record.begin();
		RecordIterator end = record.end();

		std::vector<RoutineItem*> function_code = CreateRoutineItems(it, end);

		auto parameter_list = CreateParameterSettings(parameters);

		Coroutine* routine = new Coroutine(parameter_list, function_code);

		//routine->LoadFromViewImpl(it);

		routine->DeclareOwner();

		logger::debug("Coroutine Finished Construction.");

		return routine;
	}


	template <class... StringClass> requires(all_true_v<std::is_convertible_v<StringClass, std::string>...>)
		inline Coroutine* CreateFormula(std::string& code, std::pair<StringClass, StringClass>... parameters)
	{
		ParsingArgumentList param_list{ parameters };
		return CreateFormula(code, param_list);
	}

	inline Subroutine* CreateReadyArthmetic(std::string& code)
	{
		//This should only ever return a ready arthmetic. Nothing really "Needs" a routine, and should it, one can specify, one way or the other.
		// But what this allows me to do is just store it as the result without targets, or if it's just a delegate call, you can just use that
		// delegate call.
		Record record = InitialParse(code);


		RecordIterator it = record.begin();
		RecordIterator end = record.end();

		std::vector<RoutineItem*> function_code = CreateRoutineItems(it, end);

		Subroutine* routine = new Subroutine(function_code);

		//routine->LoadFromViewImpl(it);

		routine->DeclareOwner();
		
		logger::debug("Subroutine Finished Construction.");

		return routine;
	}

	//When you specifically want subroutine as the return, and want it to not be
	// a sub routine. ReadyArth should be he one using this in the future.
	inline Subroutine* CreateSubroutine(std::string& code)
	{
		return CreateReadyArthmetic(code);
	}


	//*/
}