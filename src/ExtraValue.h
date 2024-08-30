#pragma once

#include "ExtraValueInfo.h"
#include "Arthmetic/ArthmeticUtility.h"
/*
namespace AVG
{
	template <class StringType>
	static void CheckActorValue(RE::ActorValue& av, StringType str)
	{
		if (av != RE::ActorValue::kNone && av != RE::ActorValue::kTotal)
			return;

		//I'd like to put a lock here.

		av = Utility::StringToActorValue(str);

		if (Utility::IsValidValue(av) == true)
			return;

		std::string av_name = std::string(str);

		ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(av_name);

		if (!info) {
			logger::error("Attempts to find Actor Value for '{}' have failed.", str);
			av = RE::ActorValue::kTotal;
			return;
		}

		av = static_cast<RE::ActorValue>(info->GetValueID());
	}
	
	struct DynamicValueID
	{
		RE::ActorValue actorValue = RE::ActorValue::kNone;

		DynamicValueID() = default;

		constexpr DynamicValueID(RE::ActorValue a_rhs) : actorValue(a_rhs){}

		template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
			DynamicValueID(StringType a_rhs)
		{
			CheckActorValue(actorValue, a_rhs);
		}


		constexpr DynamicValueID& operator=(RE::ActorValue a_rhs)
		{
			actorValue = a_rhs;
			return *this;
		}


		template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
		DynamicValueID& operator=(StringType a_rhs)
		{
			CheckActorValue(actorValue, a_rhs);
			return *this;
		}
	};


	template <StringLiteral Av_Name>
	struct ConstValueID
	{
		static constexpr auto ActorValueName = Av_Name.value;

		operator RE::ActorValue() const
		{
			CheckActorValue(_actorValueID, ActorValueName);
			return _actorValueID;
		}

	private:
		static inline RE::ActorValue _actorValueID = RE::ActorValue::kNone;
	};
}
//*/