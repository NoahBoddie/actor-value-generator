#pragma once

namespace RE
{
	using ActorValueModifier = RE::ACTOR_VALUE_MODIFIER;
}

namespace AVG
{

	using ActorProcessType = RE::PROCESS_TYPE;
	
	using ActorValueModifier = RE::ACTOR_VALUE_MODIFIER;

	//I'm thinking I don't just want this as an alias anymore, I might have situations where ids are invalidated based on what range you set
	// them to? or something? I dunno, they might not need to exist, but the point should be valueID should be able to be gotten ez from
	// actor value. Data values are sorta their own thing, being the index of the id.
	using DataID = uint32_t;//This is the code used to get the index of ExtraValueData
    using ValueID = uint32_t;//This is the code used to blend in with actor values. Should avoid using total and -1

    using EVTick = uint8_t;//Ticks used for recovery and EV updating.

	using FileView = toml::v3::node_view<const toml::v3::node>;
	using FileNode = toml::v3::node;

    //Move the enums, and include them in this file. Then, made Types included everywhere.
	enum ExtraValueInput
	{
		None = 0,
		Base = 1 << 0,
		Permanent = 1 << 1,
		Temporary = 1 << 2,
		Damage = 1 << 3,
		All = Base | Permanent | Temporary | Damage,

		//Desirable combinations
		Natural = Base | Permanent,
		Increase = Permanent | Temporary,
		Maximum = Base | Permanent | Temporary,
		Cached = Natural,             //Those values that are persistent
		Loaded = Temporary | Damage,  //Values that are only for loaded actors, barring cases withstanding.

		//Exception combinations
		ExceptBase = All ^ Base,
		ExceptPermanent = All ^ Permanent,
		ExceptTemporary = All ^ Temporary,
		ExceptDamage = All ^ Damage,

	};

	constexpr RE::ACTOR_VALUE_MODIFIER ValueInputToModifier(ExtraValueInput input)
	{
		switch (input)
		{
		case ExtraValueInput::Base:
			return RE::ACTOR_VALUE_MODIFIER::kTotal;

		case ExtraValueInput::Permanent:
			return RE::ACTOR_VALUE_MODIFIER::kPermanent;

		case ExtraValueInput::Temporary:
			return RE::ACTOR_VALUE_MODIFIER::kTemporary;
			break;

		case ExtraValueInput::Damage:
			return RE::ACTOR_VALUE_MODIFIER::kDamage;
			break;

		default:
			return RE::ACTOR_VALUE_MODIFIER::kTotal;
		}
	}

	constexpr ExtraValueInput ModifierToValueInput(RE::ACTOR_VALUE_MODIFIER modifier)
	{
		switch (modifier)
		{
		case RE::ACTOR_VALUE_MODIFIER::kTotal:
			return ExtraValueInput::Base;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kPermanent:
			return ExtraValueInput::Permanent;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kTemporary:
			return ExtraValueInput::Temporary;
			break;

		case RE::ACTOR_VALUE_MODIFIER::kDamage:
			return ExtraValueInput::Damage;
			break;

		default:
			return ExtraValueInput::None;
		}
	}




}