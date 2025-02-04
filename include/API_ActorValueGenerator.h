#pragma once

#ifndef _STRING_VIEW_
#include <string_view>
#endif


#define AVG_API_SOURCE "ActorValueGenerator.dll"
#define AVG_API_SOURCE_L L"ActorValueGenerator.dll"


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
			REL::Relocation<func_t> func{ REL::RelocationID(26570, 27203) };
			return func(av_name);
		}
	}


	inline RE::ActorValue GetActorValueIDFromName(std::string_view av_name)
	{
		//Please make sure that you are using a null terminated string view. Wouldn't want it to be completely incorrect.
		return detail::GetActorValueIDFromName(av_name.data());
	}
	
}

namespace AVG
{
	namespace API
	{
		enum Version
		{
			Version1,

			Current = Version1
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

		using CurrentInterface = InterfaceVersion1;


		inline CurrentInterface* Interface{ nullptr };

#ifdef AVG_SOURCE 

		CurrentInterface* InferfaceSingleton();

		namespace detail
		{
			extern "C" __declspec(dllexport) void* AVG_RequestInterfaceImpl(Version version)
			{
				CurrentInterface* result = InferfaceSingleton();

				switch (version)
				{
				case Version::Version1:
					return dynamic_cast<InterfaceVersion1*>(result);

				}

				return nullptr;
			}
		}

#endif

		/// <summary>
		/// Accesses the ActorValueGenerator Interface. Using the template version is advised. Safe to call PostLoad
		/// </summary>
		/// <param name="version"> to request.</param>
		/// <returns>Returns void* of the interface, cast to the respective version.</returns>
		inline void* RequestInterface(Version version)
		{
			typedef void* (__stdcall* RequestFunction)(Version);

			static RequestFunction request_interface = nullptr;

			HINSTANCE API = GetModuleHandle(AVG_API_SOURCE_L);

			if (API == nullptr) {
				logger::critical("ActorValueExtension.dll not found, API will remain non functional.");
				return nullptr;
			}

			request_interface = (RequestFunction)GetProcAddress(API, "AVG_RequestInterfaceImpl");


			if (request_interface) {
				if (static unsigned int once = 0; once++)
					logger::info("Successful module and request, AVG");
			}
			else {
				logger::critical("Unsuccessful module and request, AVG");
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

				if constexpr (std::is_same_v<InterfaceClass, CurrentInterface>)
					Interface = intfc;
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