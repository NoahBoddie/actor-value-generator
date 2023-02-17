#pragma once

#include "RE/B/BGSEntryPointFunctionData.h"

namespace RE
{
	class BGSEntryPointFunctionDataTwoValue : public BGSEntryPointFunctionData
	{
	public:
		inline static constexpr auto VTABLE = VTABLE_BGSEntryPointFunctionDataTwoValue;
		inline static constexpr auto RTTI = RTTI_BGSEntryPointFunctionDataTwoValue;

		//~BGSEntryPointFunctionDataTwoValue() override;  // 00

		// override (BGSEntryPointFunctionData)
		//FunctionType GetType() const override;               // 01 - { return kOneValue; }
		//bool LoadFunctionData(RE::TESFile* a_mod) override;  // 02

		// members
		float data1;  // 08 - DATA
		float data2;  // 0C
	};
	static_assert(sizeof(BGSEntryPointFunctionDataTwoValue) == 0x10);
}

