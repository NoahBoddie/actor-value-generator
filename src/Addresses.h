#pragma once

namespace AVG
{
	struct Addresses
	{
		//This is the condition function one, I think I want the papyrus version. Too early to tell.
		//REL::VariantID TESObjectREFR__GetItemCount = REL::VariantID(20985, 0x0, 0x0);
		inline static REL::RelocationID TESObjectREFR__GetItemCount{ 20985, 21435 };//SE: 0x2D5750, AE: 0x2EB3B0, VR: ?? 
	};

	
}