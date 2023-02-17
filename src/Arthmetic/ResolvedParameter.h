#pragma once

//#include "Arthmetic/ParameterType.h"
//#include "Arthmetic/DelegateParameter.h"

namespace Arthmetic
{

	//A resolved parameter is created from 3 different things, the default value from the setting,
	struct ResolvedParameter
	{
		//This is used in any place that wants to signal whether it's been filled or not.
		// So, when having these in a vector or something, you have _resolved to tell you if the value in question
		// is it's actual value.

		bool _resolved = false;


		//Also important to note, that resolved parameters effectively own nothing, and instead refer to data that is either
		// currently on the stack of calculated when applicable. This is in the case of strings.

		union
		{
			uint64_t _value{};
			char* _string;//When outputted, put into an std::string

			float _number;

			ARTH_OBJECT_TYPE* _object;
		};

		//Must be created from setting or delegate parameter
	};


#ifdef Custom_List_Class

	struct ParameterList
	{
		//static ParameterList default_list{nullptr};//A list with no length and thes nothing to it's name.

		//This is what's used externally, because if I introduce new types I think the different versions would break shit.
		//I think these will consist of 2 different types.

		//The first would be the dynamic targets, then the static targets. Would make it easier to handle I think. and would prevent an index
		// error. Will work on that when targeting becomes more important.

		//Should store default as well?

		const ResolvedParameter* params = nullptr;
		size_t length = 0;

		//Access should be arranged with getter functions, in case not enough has been fulfilled.
		// Also, delegate parameters are not what should be being used. a weak carrier object called ResolvedParameters
		// What should go in it are the fully resolved parameters of a function. Resolved parameters make function calls,
		// and resolve routines, reducing items down to just their values.

		ParameterList() = default;

		//This is supposed to take 2 lists.
		ParameterList(std::vector<DelegateParameter>* delegate_params)
		{
			if (delegate_params)
			{
				//targets = delegate_params->data();
				length = delegate_params->size();
			}
		}
	};

#else
	

#endif

}