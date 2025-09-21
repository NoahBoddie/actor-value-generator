#include "ExtraValueStorage.h"
#include "ExtraValueInfo.h"

namespace AVG
{
	std::map<RE::FormID, ExtraValueStorage>& ExtraValueStorage::_valueTable =
		TOME::SerialManager::CreateSerializer<ExtraValueStorage::SerialMapClass, PrimaryRecordType::ExtraValueStorage>();


	ExtraValueStorage::ExtraValueStorage(RE::Actor* actor, bool create_default)
	{
		ResetStorageImpl(actor, create_default);
		initialized = true;

		//*/
		//This is gonna need a neat organized list of info to perform h
	}


	StorageView ExtraValueStorage::GetStorage(RE::Actor* actor)
	{
		//This won't be used often, but it's used in situations where it would be ok to not create storage yet.
		if (!actor) {
			return nullptr;
		}
		
		if (actor->IsPlayerRef() == true)
		{
			auto test = PlayerStorage::GetAsPlayable(false);
			return test;
		}
		
		//ReadLock guard{ accessLock };
		StorageView view{ accessLock, true };

		auto result = _valueTable.find(actor->formID);

		ExtraValueStorage* storage = nullptr;
		
		if (_valueTable.end() != result && result->second.initialized) {
			storage = &result->second;
		}

		view.SetStorage(storage);
		
		//return result == _valueTable.end() ? nullptr : &result->second;
		return view;
	}


	StorageView ExtraValueStorage::ObtainStorage(RE::Actor* actor)
	{
		//Should this be thead locked? I feel like this should be thread locked

		if (!actor) {
			logger::critical("No actor detected, terminating storage search.");
			throw nullptr;  //Just crash, this isn't supposed to be found.
		}

		if (actor->IsPlayerRef() == true) {
			auto test = PlayerStorage::GetAsPlayable(true);
			return *test;
		}


		StorageView view{ accessLock, false };
		//WriteLock guard{ accessLock };
		
		ExtraValueStorage& storage = _valueTable[actor->formID];
		
		if (!storage.initialized) {
			storage = ExtraValueStorage{ actor, false };
		}

		view.SetStorage(storage);
		//return storage;
		return view;
	}


	bool ExtraValueStorage::RemoveStorage(RE::FormID _id)
	{
		WriteLock guard{ accessLock };

		if (!_id)
			return false;

		if (_id == 0x14) {
			logger::debug("PlayerStorage cannot be removed.");
			return false;
		
		}

		auto removes = _valueTable.erase(_id);
		if (removes)
			logger::info("[Unregister {:08X} ]", _id);
		

		return removes;
	}

	void ExtraValueStorage::RemoveAllStorages()
	{
		WriteLock guard{ accessLock };

		_valueTable.clear();
	}

	void ExtraValueStorage::ResetStorageImpl(RE::Actor* actor, bool init_default)
	{
		if (!actor)
			return;

		WriteLock guard;

		if (initialized) {
			guard = WriteLock{ accessLock };
		}

		//if deserializing, we won't really do anything, just create it.

		auto size = ExtraValueInfo::GetCountUpto(actor->IsPlayerRef() ? ExtraValueType::Exclusive : ExtraValueType::Adaptive);
		logger::debug("store size {} for {}", size, actor->GetName());
		if (size == ExtraValueInfo::FunctionalID) {
			return;  //print error, probbably crash
		}
		
		//_valueData = std::vector<ExtraValueData>(size, ExtraValueData());
		//_recoveryData = ExtraValueInfo::GetRecoverableValues(actor);

		_valueData.resize(size);
		std::fill(_valueData.begin(), _valueData.end(), ExtraValueData{});
		ExtraValueInfo::FillRecoverableValues(actor, _recoveryData);



		if (!init_default)
			return;
		//*
		//auto adapt_list = ExtraValueInfo::GetAdaptiveList();

		//ArgTargetParams tar_params = MakeTargetParamList(actor);
		//Make this a function plz
		for (int i = 0; i < _valueData.size(); i++) {
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByData(i);



			if (!info)
				continue;

			DefaultInfo* default_data = info->FetchDefaultInfo();

			//No real implementation yet, but yeah.
			if (default_data)
				if (default_data->_type == DefaultInfo::Constant || default_data->_type == DefaultInfo::Explicit)
					continue;

			//At a later point this will not take place, and instead, base value is NaN while it has not been manually set, so
			// it shapes to whatever default value exists.
			float base_value = info->GetExtraValueDefault(actor);//update->updateFunction->RunImpl(actor);
			//should take the target actor.
			_valueData[i]._base = base_value;
		}
	}



	PlayerStorage& PlayerStorage::_singleton = TOME::SerialManager::CreateSerializer<PlayerStorage, PrimaryRecordType::PlayerStorage>();
}