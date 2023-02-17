#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"

namespace AVG
{
	float AdaptiveValueInfo::GetExtraValue(RE::Actor* target, ExtraValueInput value_types)
	{
		if (!target)
			return 0;//I'd like to return NaN, with some additional information.

		ExtraValueStorage& ev_store = ExtraValueStorage::GetCreateStorage(target);

		return ev_store.GetValue(target, _dataID, value_types, this);


		//Needs to look up EVS, so this shit is just gonna wait for implementation.
	}

	bool AdaptiveValueInfo::SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier)
	{
		if (!target)
			return false;

		ExtraValueStorage& ev_store = ExtraValueStorage::GetCreateStorage(target);

		ExtraValueInput ev_mod = ExtraValueInput::None;

		switch (modifier) 
		{
		case RE::ACTOR_VALUE_MODIFIER::kTotal:
			ev_mod = ExtraValueInput::Base;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kPermanent:
			ev_mod = ExtraValueInput::Permanent;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kTemporary:
			ev_mod = ExtraValueInput::Temporary;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kDamage:
			ev_mod = ExtraValueInput::Damage;
			break;

		default:
			return false;
		}
		
		ev_store.SetValue(target, _dataID, value, ev_mod, this);

		return true;
	}

	bool AdaptiveValueInfo::ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier)
	{
		if (!target)
			return false;

		ExtraValueStorage& ev_store = ExtraValueStorage::GetCreateStorage(target);

		ExtraValueInput ev_mod = ExtraValueInput::None;

		switch (modifier) {
		case RE::ACTOR_VALUE_MODIFIER::kTotal:
			ev_mod = ExtraValueInput::Base;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kPermanent:
			ev_mod = ExtraValueInput::Permanent;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kTemporary:
			ev_mod = ExtraValueInput::Temporary;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kDamage:
			ev_mod = ExtraValueInput::Damage;
			break;

		default:
			return false;
		}

		ev_store.ModValue(target, aggressor, _dataID, value, ev_mod, this);

		return true;
	}
}