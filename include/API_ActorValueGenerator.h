#pragma once

#include <string_view>
#include "RE/A/Actor.h"
#include "RE/A/ActorValues.h"

#ifdef UNICODE
#define AVG_API_SOURCE L"ActorValueGenerator.dll"
#else
#define AVG_API_SOURCE "ActorValueGenerator.dll"
#endif

namespace RE
{
	//If there is ever some detail of general AV usage that needs access (GetDelay, SetDelay, etc) I will place them here.
	// The prefered should be whatever clib uses, but since they might not always be ported I'll put them here.
	namespace detail
	{
		inline RE::ActorValue GetActorValueIDFromName(const char* av_name)
		{
			//SE: 0x3E1450, AE: 0x3FC5A0, VR: ---
			using func_t = decltype(&GetActorValueIDFromName);
			REL::Relocation<func_t> func{ RELOCATION_ID(26570, 27203) };
			return func(av_name);
		}
	}



	inline const char* GetActorValueName(RE::ActorValue av)
	{
		//SE: 3E1230, AE: 3FC360, VR: ---
		using func_t = decltype(&GetActorValueName);
		REL::Relocation<func_t> func{ RELOCATION_ID(26563, 27195) };
		return func(av);
	}

	inline const char* GetActorValueScriptName(RE::ActorValue av)
	{
		//SE: 3E1130, AE: 3FC250, VR: ---
		using func_t = decltype(&GetActorValueScriptName);
		REL::Relocation<func_t> func{ RELOCATION_ID(26561, 27192) };
		return func(av);
	}

	inline RE::ActorValue GetActorValueIDFromName(std::string_view av_name)
	{
		//Please make sure that you are using a null terminated string view. Wouldn't want it to be completely incorrect.
		return detail::GetActorValueIDFromName(av_name.data());
	}
	
}

namespace AVG
{
#define ACTOR_VALUE_CHANGE_PARAMS RE::Actor* target, RE::Actor* cause, RE::ActorValue av, RE::ACTOR_VALUE_MODIFIER modifier, float before, float after
	
	using ActorValueChange = void(*)(ACTOR_VALUE_CHANGE_PARAMS);



	namespace API
	{
		enum Version
		{
			Version1,
			Version2,

			Current = Version2
		};

		struct InterfaceVersion1
		{
			inline static constexpr auto VERSION = Version::Version1;

			virtual ~InterfaceVersion1() = default;

			/// <summary>
			/// Gets the current version of the interface.
			/// </summary>
			/// <returns></returns>
			[[nodiscard]] virtual Version GetVersion() = 0;


			/// <summary>
			/// Resolves an ExtraValue after a save game has been loaded.
			/// </summary>
			/// <param name="av_ref">ExtraValue to resolve, treated as an actor value.</param>
			/// <returns></returns>
			[[nodiscard]] virtual RE::ActorValue ResolveExtraValue(RE::ActorValue av_ref) = 0;


		};

		struct InterfaceVersion2 : public InterfaceVersion1
		{
			inline static constexpr auto VERSION = Version::Version2;

			/// <summary>
			/// Registers a function to fire after an ActorValue changes it's base or modifier values. Function requires parameters:
			/// <para>  RE::Actor* target: the target of the change</para> 
			/// <para>	RE::Actor* cause: cause of the change (available only on damage)</para> 
			/// <para>	RE::ActorValue av: The Changing actor value</para> 
			/// <para>	RE::ACTOR_VALUE_MODIFIER modifier: The modifier impacted (kTotal is the base value)</para> 
			/// <para>	float prev_value: The value before the change happened</para> 
			/// <para>	float new_value: The value before the change happened</para> 
			/// <para>	bool recursive: A check if this event is firing off within another event</para> 
			/// <para>	*Use macro ACTOR_VALUE_CHANGE_PARAMS to set up parameters as above automatically </para> 
			/// </summary>
			virtual void RegisterForActorValueChange(ActorValueChange func) = 0;


		};


		using CurrentInterface = InterfaceVersion2;




		/// <summary>
		/// Accesses the ActorValueGenerator Interface. Using the template version is advised. Safe to call PostLoad
		/// </summary>
		/// <param name="version"> to request.</param>
		/// <returns>Returns void* of the interface, cast to the respective version.</returns>
		inline void* RequestInterface(Version version)
		{
			typedef void* (__stdcall* RequestFunction)(Version);

			static RequestFunction request_interface = nullptr;

			HINSTANCE API = GetModuleHandle(AVG_API_SOURCE);

			if (API == nullptr) {
				spdlog::critical("ActorValueGenerator.dll not found, API will remain non functional.");
				return nullptr;
			}

			request_interface = (RequestFunction)GetProcAddress(API, "AVG_RequestInterfaceImpl");


			if (request_interface) {
				if (static unsigned int once = 0; once++)
					spdlog::info("Successful module and request, AVG");
			}
			else {
				spdlog::critical("Unsuccessful module and request, AVG");
				return nullptr;
			}

			auto intfc = (CurrentInterface*)request_interface(version);

			return intfc;
		}

		/// <summary>
		/// Accesses the ActorValueGenerator Interface, safe to call PostLoad
		/// </summary>
		/// <typeparam name="InterfaceClass">is the class derived from the interface to use.</typeparam>
		/// <returns>Casts to and returns a specific version of the interface.</returns>
		template <class InterfaceClass = CurrentInterface>
		inline  InterfaceClass* RequestInterface()
		{
			static InterfaceClass* intfc = nullptr;

			if (!intfc) {
				intfc = reinterpret_cast<InterfaceClass*>(RequestInterface(InterfaceClass::VERSION));
				assert(intfc);
			}

			return intfc;
		}

	}

	struct ExtraValue
	{
		//It is in one's best interest to NEVER statically create these. Needless to say it wouldn't wait to see if AVG exists, and would get none

		constexpr ExtraValue() = default;

		constexpr ExtraValue(RE::ActorValue a) : _av{ a } {}
		
		ExtraValue(std::string_view name) : _av{ RE::GetActorValueIDFromName(name) } {}
		
		RE::ActorValue Resolve() const
		{
			if (auto api = API::RequestInterface(); api) {
				_av = api->ResolveExtraValue(_av);
			}
			else if (_av > RE::ActorValue::kTotal) {
				_av = RE::ActorValue::kNone;
			}

			return _av;
		}


		constexpr operator RE::ActorValue()
		{
			return _av;
		}

		
	private:
		mutable RE::ActorValue _av = RE::ActorValue::kNone;

	};


}

