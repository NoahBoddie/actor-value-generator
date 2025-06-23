#include "ExtraValueStorage.h"
#include "ExtraValueInfo.h"

namespace AVG
{
	ExtraValueStorage::ExtraValueStorage(RE::Actor* actor, bool create_default)
	{
		ResetStorage(actor, create_default);
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
		{
			auto test = PlayerStorage::GetAsPlayable(false);
			return test;
		}
			
		ReadLock guard{ accessLock };

		auto result = _valueTable->find(actor->formID);

		return result == _valueTable->end() ? nullptr : result->second;
	}


	ExtraValueStorage& ExtraValueStorage::ObtainStorage(RE::Actor* actor)
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

		WriteLock guard{ accessLock };
		
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
		WriteLock guard{ accessLock };

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

	void ExtraValueStorage::ResetStorageImpl(RE::Actor* actor, bool init_default)
	{
		if (!actor)
			return;

		//if deserializing, we won't really do anything, just create it.

		auto size = ExtraValueInfo::GetCountUpto(actor->IsPlayerRef() ? ExtraValueType::Exclusive : ExtraValueType::Adaptive);
		logger::debug("store size {} for {}", size, actor->GetName());
		if (size == ExtraValueInfo::FunctionalID) {

			return;  //print error, probbably crash
		}
		auto& value_data = _valueData.get();

		value_data = std::remove_reference_t<decltype(value_data)>(size, ExtraValueData());

		_recoveryData.get() = ExtraValueInfo::GetRecoverableValues(actor);



		if (!init_default)
			return;
		//*
		//auto adapt_list = ExtraValueInfo::GetAdaptiveList();

		//ArgTargetParams tar_params = MakeTargetParamList(actor);
		//Make this a function plz
		for (int i = 0; i < value_data.size(); i++) {
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
			value_data[i]._base = base_value;
		}
	}


	void ExtraValueStorage::ResetStorage(RE::Actor* owner, bool init_default)
	{
		if (auto singleton = PlayerStorage::GetSingleton(); singleton == this)
		{
			singleton->ResetStorageImpl(RE::PlayerCharacter::GetSingleton(), init_default);
		}
		else
		{
			ResetStorageImpl(owner, init_default);
		}
	}




	PlayerStorage& PlayerStorage::_singleton = SerializationHandler::CreatePrimarySerializer<PlayerStorage>(PrimaryRecordType::PlayerStorage);
}