#pragma once

#include "Arthmetic/ArgumentType.h"
#include "Arthmetic/DelegateArgument.h"

namespace Arthmetic
{
	struct ParameterSetting
	{
		//ArgumentType type;//Do I really need this?
		std::string name;

		int index = -1;

		inline ArgumentType GetType() { return defaultValue.GetType(); }

		DelegateArgument defaultValue;

		bool HasDefault()
		{
			return defaultValue.IsResolved();
		}
	};

	using ParameterSettingList = std::vector<ParameterSetting>;
}