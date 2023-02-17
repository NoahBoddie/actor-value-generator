#pragma once

#include "Arthmetic/TargetType.h"
#include "Arthmetic/ParameterSetting.h"
#include "Arthmetic/DelegateArgument.h"

namespace Arthmetic
{
	struct IDirective : public ArthmeticObject
	{
		//I formulas push no index, as they are self contained.


		std::unique_ptr<ParameterSettingList> _paramSettingList{ nullptr };

		//I've removed default value and the return value from run because that implementation can change between formulas.

		ParameterSetting* FindParamSettingByName(const char* name)
		{
			//int loc = -1;

			if (!_paramSettingList || !name)
				return nullptr;

			auto name_comp = [=](ParameterSetting& setting) { logger::info("{} vs {}", setting.name, name); return CStrCmpI(setting.name, name); };

			auto result = std::find_if(_paramSettingList->begin(), _paramSettingList->end(), name_comp);


			if (_paramSettingList->end() == result)
				return nullptr;

			return result._Ptr;
		}


		virtual void Run(Target target, ParameterList, float&) = 0;

		void RunImpl(Target target, ParameterList params, float& result)
		{

			//if (IsValid() == false) {
			//	result = 0;
			//	return;
			//}

			Run(target, params, result);
		}
	};
}