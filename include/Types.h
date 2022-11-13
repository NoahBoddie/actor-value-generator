#pragma once

namespace AVG
{
	using DataID = uint32_t;//This is the code used to get the index of ExtraValueData
    using ValueID = uint32_t;//This is the code used to blend in with actor values. Should avoid using total and -1

    using EVTick = uint8_t;//Ticks used for recovery and EV updating.

    //This is made
	// so the variable demand of modifiers is easier.
	//RELOCATE to include.
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
}