#include "API_ActorValueGenerator.h"

#include "ExtraValueInfo.h"


namespace AVG::API
{

	struct ActorValueGeneratorInterface : public CurrentInterface
	{
		

		Version GetVersion() override { return Version::Current; }


		RE::ActorValue ResolveExtraValue(RE::ActorValue av) override
		{
			if (av <= RE::ActorValue::kTotal || av == RE::ActorValue::kNone)
				return av;

			uint32_t id = (uint32_t)av - (uint32_t)RE::ActorValue::kTotal;

			auto info = ExtraValueInfo::GetValueInfoByManifest(id);

			if (!info)
				return RE::ActorValue::kNone;

			return info->GetValueIDAsAV();
		}

		void RegisterForActorValueChange(ActorValueChange func)
		{
			ExtraValueInfo::AddOnActorValueChanged(func);
		}

		DelegateResult RegisterAVDelegate(std::string_view name, GetAVDelegate get, SetAVDelegate set) override
		{
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(name);

			if (!info) {
				return DelegateResult::Nonexistent;
			}

			FunctionalData* data;
			switch (info->GetType())
			{
			case ExtraValueType::Functional:
				data = static_cast<FunctionalValueInfo*>(info)->function();
				break;
			case ExtraValueType::Exclusive:
				data = static_cast<ExclusiveValueInfo*>(info)->function();
				break;

			default:
				return DelegateResult::Nonfunctional;
			}

			if (data->IsDelegate() == false) {
				return DelegateResult::Nondelegate;
			}

			auto& delegate = data->ObtainDelegate(info);

			if (delegate.get || delegate.set) {
				return DelegateResult::AlreadyFilled;
			}

			delegate.get = get;
			delegate.set = set;

			return DelegateResult::Success;
		}

	};



	[[nodiscard]] CurrentInterface* InferfaceSingleton()
	{
		static ActorValueGeneratorInterface intfc{};
		return &intfc;
	}

	extern "C" __declspec(dllexport) void* AVG_RequestInterfaceImpl(Version version)
	{

		CurrentInterface* result = InferfaceSingleton();

		if (result && result->GetVersion() >= version)
			return result;

		return nullptr;
	}

}