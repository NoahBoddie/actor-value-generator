#pragma once

#include "Arthmetic/IDirective.h"

namespace Arthmetic
{
	struct IFormula : public IDirective
	{
		//If the function gets an error thrown by a missing parameter or something like that, this is the default value used.
		//Namely, if your choosen value is null
		//I think if the result is NAN it will also use this as well.
		float defaultValue = 0;



		inline float Run(Target target, ParameterList formula_params)
		{
			//Not really needed, but symbolizes a change in function.
			float result = 0;

			RunImpl(target, formula_params, result);

			return result;
		}
	};

	//would like something other than string, probably should use string view because the formula 
	// holds its name?
	inline std::map<std::string, IFormula*> formulaMap;

}