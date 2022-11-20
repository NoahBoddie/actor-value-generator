#include "ExtraValueStorage.h"
#include "ExtraValueInfo.h"

namespace AVG
{
	ExtraValueStorage::ExtraValueStorage(RE::Actor* actor, bool is_deserializing)
	{
		if (!actor)
			return;

		//if deserializing, we won't really do anything, just create it.

		auto size = ExtraValueInfo::GetAdaptiveCount();
		logger::info("store size {} for {}", size, actor->GetName());
		if (size == ExtraValueInfo::FunctionalID) {
			
			return;  //print error, probbably crash
		}
		_valueData = decltype(_valueData)(size, ExtraValueData());
		
		_recoveryData = ExtraValueInfo::GetRecoverableValues();



		if (is_deserializing)
			return;
		/*
		auto adapt_list = ExtraValueInfo::GetAdaptiveList();

		ArgTargetParams tar_params = MakeTargetParamList(actor);

		for (int i = 0; i < size; i++) {
			auto extra_info = adapt_list[i];

			if (!extra_info || !extra_info->updateCalc)
				continue;

			if (extra_info->dataID != i)
				continue;

			//should take the target actor.
			valueData[i]._base = extra_info->updateCalc->Run(tar_params);
		}
		//*/
		//This is gonna need a neat organized list of info to perform h
	}
}