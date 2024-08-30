#pragma once

#include "API_ActorValueGenerator.h"

#include "ExtraValueInfo.h"
#include "Arthmetic/ArthmeticUtility.h"

namespace ActorValueGeneratorAPI
{
	using namespace AVG;

	struct ActorValueGeneratorInterface : public CurrentInterface
	{
		Version GetVersion() override { return Version::Current; }


		void RegisterExportFunction(std::string_view name, ExportFunction func) override
		{
			//For now I don't really care about making this check for null.
			logger::info("export function {} registered.", name);
			exportMap[std::string(name)] = func;
		}

		void CheckActorValue(RE::ActorValue& av, const char* c_str) override
		{
			if (av != RE::ActorValue::kNone && av != RE::ActorValue::kTotal)
				return;

			std::string str = c_str;

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

		void ResolveActorValue(RE::ActorValue& av)
		{
			auto info = ExtraValueInfo::GetValueInfoByManifest((uint32_t)av);

			if (!info)
				return;

			av = info->GetValueIDAsAV();
		}

		bool SetAVDelay(RE::Actor*, RE::ActorValue, float) override
		{
			return false;
		}
	};

	[[nodiscard]] CurrentInterface* InferfaceSingleton()
	{
		static ActorValueGeneratorInterface intfc{};
		return &intfc;
	}
}