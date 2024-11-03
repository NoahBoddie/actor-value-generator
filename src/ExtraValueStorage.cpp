#include "ExtraValueStorage.h"
#include "ExtraValueInfo.h"

namespace AVG
{
	ExtraValueStorage::ExtraValueStorage(RE::Actor* actor, bool create_default)
	{
		if (!actor)
			return;

		logger::info("Project {}", LEX::Formula<float>::Run("GetPlayer().ProjectTest()", commons));

		//if deserializing, we won't really do anything, just create it.

		auto size = ExtraValueInfo::GetCount(ExtraValueType::Adaptive);
		logger::debug("store size {} for {}", size, actor->GetName());
		if (size == ExtraValueInfo::FunctionalID) {
			
			return;  //print error, probbably crash
		}
		auto& value_data = _valueData.get();

		value_data = std::remove_reference_t<decltype(value_data)>(size, ExtraValueData());
		
		_recoveryData.get() = ExtraValueInfo::GetRecoverableValues(actor);



		if (!create_default)
			return;
		//*
		//auto adapt_list = ExtraValueInfo::GetAdaptiveList();

		//ArgTargetParams tar_params = MakeTargetParamList(actor);
		//Make this a function plz
		for (int i = 0; i < value_data.size(); i++) {
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByData(i);

			DefaultData* default_data = info->GetDefaultDataSafe();

			if (!info)
				continue;

			//No real implementation yet, but yeah.
			if (default_data)
				if (default_data->_type == DefaultData::Constant || default_data->_type == DefaultData::Explicit)
					continue;

			//At a later point this will not take place, and instead, base value is NaN while it has not been manually set, so
			// it shapes to whatever default value exists.
			float base_value = info->GetExtraValueDefault(actor);//update->updateFunction->RunImpl(actor);
			//should take the target actor.
			value_data[i]._base = base_value;
		}
		//*/
		//This is gonna need a neat organized list of info to perform h
	}


	ExtraValueStorage* ExtraValueStorage::GetStorage(RE::Actor* actor)
	{
		//This won't be used often, but it's used in situations where it would be ok to not create storage yet.
		if (!actor) {
			return nullptr;
		}
		
		if (actor->IsPlayerRef() == true)
			return PlayerStorage::GetAsPlayable(false);

		auto result = _valueTable->find(actor->formID);

		return result == _valueTable->end() ? nullptr : result->second;
	}


	ExtraValueStorage& ExtraValueStorage::GetCreateStorage(RE::Actor* actor)
	{
		//Should this be thead locked? I feel like this should be thread locked

		if (!actor) {
			logger::critical("No actor detected, terminating storage search.");
			throw nullptr;  //Just crash, this isn't supposed to be found.
		}

		if (actor->IsPlayerRef() == true)
			return *PlayerStorage::GetAsPlayable(true);


		ExtraValueStorage*& storage_spot = (*_valueTable)[actor->formID];

		if (storage_spot) {
			//logger::warn("Stor loc {}", (uintptr_t)storage_spot);
			return *storage_spot;
		}


		ExtraValueStorage* new_storage = new ExtraValueStorage(actor, false);

		storage_spot = new_storage;

		return *new_storage;
	}


	bool ExtraValueStorage::RemoveStorage(RE::FormID _id)
	{
		if (!_id || _valueTable->contains(_id) == false)
			return false;


		if (_id == 0x14) {
			logger::debug("PlayerStorage cannot be removed.");
			return false;
		
		}

		//needs an initializer part
		//It will need to search the left hand, the right hand
		logger::info("[Unregister {:08X} ]", _id);


		ExtraValueStorage* storage = _valueTable[_id];

		//LOCK while removing

		_valueTable->erase(_id);

		//delete *cData;
		delete storage;

		return true;
	}




	PlayerStorage& PlayerStorage::_singleton = SerializationHandler::CreatePrimarySerializer<PlayerStorage>(PrimaryRecordType::PlayerStorage);
}