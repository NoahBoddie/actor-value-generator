#pragma once

namespace Arthmetic
{
	//Should have a fuller name, to specify the inbetween of coroutines and methods.
	struct InvalidArgError : public std::exception
	{
		//If it could be so possible to say this once, and then never again.
		std::string what{};

		InvalidArgError(const std::string& what_arg) : what(what_arg) {}
	};

	//Don't want this, this is just to resolve this issue.
	using FunctionCallback = float(Target, const ArgumentList&);

	struct FunctionInterface : public IFormula
	{
		FunctionCallback* _callback = nullptr;

		void Run(Target target, ParameterList p_list, float& result) override
		{
			float default_value = p_list.defaultValue.IsResolved() ? p_list.defaultValue.GetNumberParam() : defaultValue;

			if (!_callback) {
				//Log error, do return value
				result = default_value;
			}

			try
			{
				ArgumentList list(p_list.size());
				
				std::transform(p_list.begin(), p_list.end(), list.begin(), 
					[](DelegateArgument& arg) { return &arg; });

				result = _callback(target, list);
			}
			catch (const InvalidArgError& error)//Not gonna use anything yet.
			{//Temporary, but this is the exception that helps an easy bail.
				//logger::error(error.what);//Don't actually care right now. No debugging data.
				result = default_value;
			}
		}

		void OnLinkage(LinkerFlags link_type, bool& success) override
		{
			/*
			if (link_type == LinkerFlags::Object)
				return;//I cannot stress how little I care about this rn.



			if (IsPartiallyValid() == false)
			{
				std::string code{ _formulaName };

				FreeData();

				if (formulaMap.contains(code) == true) {
					_formula = formulaMap[code];
					logger::info("Formula '{}' found and linked.", code);
				}
				else {
					logger::error("Formula '{}' not found", code);
					do_return(success = false);
				}
			}
			//*/
		}

		FunctionInterface() = default;

		FunctionInterface(ParameterSettingList parameters)
		{
			_paramSettingList.reset(new ParameterSettingList(parameters));
		}
	};

}