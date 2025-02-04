#include "API_ActorValueGenerator.h"

#include "ExtraValueInfo.h"


namespace AVG::API
{
	struct ActorValueGeneratorInterface : public CurrentInterface
	{
		Version GetVersion() override { return Version::Current; }


		RE::ActorValue ResolveExtraValue(RE::ActorValue av) override
		{
			if (av <= RE::ActorValue::kTotal || av ==  RE::ActorValue::kNone)
				return av;

			uint32_t id  = (uint32_t)av - (uint32_t)RE::ActorValue::kTotal;

			auto info = ExtraValueInfo::GetValueInfoByManifest(id);

			if (!info)
				return RE::ActorValue::kNone;

			return info->GetValueIDAsAV();
		}
	};

	[[nodiscard]] CurrentInterface* InferfaceSingleton()
	{
		static ActorValueGeneratorInterface intfc{};
		return &intfc;
	}
}