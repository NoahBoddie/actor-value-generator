#pragma once

//#include <string_view>



#define ARTH_OBJECT_TYPE RE::TESForm
#define ARTH_CONTEXT_TYPE RE::ExtraDataList
#define ARTH_ENUM_TYPE RE::FormType

#define ARITHMETIC_API_SOURCE "ActorValueExtension.dll"
#define ARITHMETIC_API_SOURCE_L L"ActorValueExtension.dll"

#include <API_Arithmetic.h>



namespace ActorValueGeneratorAPI
{
	struct ExportSetData
	{
		using StringVector = std::vector<std::string>;

		RE::Actor*					target{ nullptr };	//Targeted change of the AV.
		RE::Actor*					cause{ nullptr };	//Used when damage happens

		RE::ActorValue				av_index{ RE::ActorValue::kTotal };	//Index for the literal value
		std::string					av_name{};							//AV name for more accurate context

		RE::ACTOR_VALUE_MODIFIER	av_modifier;		//The Actor Value Modifier targeted, kTotal is base

		StringVector				export_context{};	//Context stored on values
		mutable StringVector		process_context{};	//Context passed through functions.

		float						to{ 0 };			//The pure value this was just set to.
		float						from{ NAN };	//If modded, its the result from the get function

		void PushBackContext(std::string context) const { process_context.push_back(context); }

#ifdef AVG_SOURCE

		ExportSetData(RE::Actor* tar, RE::Actor* cus,
			RE::ActorValue index, std::string name, RE::ACTOR_VALUE_MODIFIER mod,
			float to_val, float from_val) :
			target{ tar }, cause{ cus },
			av_index{ index }, av_name{ name }, av_modifier {mod},
			to{ to_val }, from{ from_val } {}

		ExportSetData(RE::Actor* tar, RE::Actor* cus, RE::ActorValue index, std::string name,
			RE::ACTOR_VALUE_MODIFIER mod, float to_val) :
			target{ tar }, cause(cus), av_index{ index }, av_name{ name }, av_modifier{ mod }, to{ to_val } {}


		ExportSetData(RE::Actor* tar, RE::ActorValue index, std::string name,
			RE::ACTOR_VALUE_MODIFIER mod, float to_val) :
			target{ tar }, av_index{ index }, av_name{ name }, av_modifier{ mod }, to{ to_val } {}
#else

	private:
		ExportSetData() = delete;
		ExportSetData(const ExportSetData&) = delete;
		ExportSetData(ExportSetData&&) = delete;

		ExportSetData& operator=(const ExportSetData&) = delete;
		ExportSetData& operator=(ExportSetData&&) = delete;
		~ExportSetData() = delete;
#endif
	};

	//deprecated.
	//RE::Actor* target, RE::Actor* aggressor, std::string av_name, ActorValueModifer mod(total = base), float value
	using ExportFunctionV1 = void(*)(RE::Actor*, RE::Actor*, std::string, RE::ACTOR_VALUE_MODIFIER, float);
	using ExportFunctionV2 = void(*)(RE::Actor*, RE::Actor*, std::string, std::vector<std::string>, RE::ACTOR_VALUE_MODIFIER, float);

	using ExportFunction = void(*)(const ExportSetData&);

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
		virtual Version GetVersion() = 0;


		//Target will have to be defined here if there's no source, just for the transfers sake. It doesn't nothing special anyhow.
		
		/// <summary>
		/// Registers a function to able to be called upon with it's set functions. Currently must be called in the event PostLoad
		/// </summary>
		/// <param name="export_name"></param>
		/// <param name="function"></param>
		virtual void RegisterExportFunction(std::string export_name, ExportFunction function) = 0;

		/// <summary>
		/// Checks an actor value, turning it into the actor value the name designated.
		/// </summary>
		/// <param name="av_ref">is actor value index to alter. Will be set to total upon failure.</param>
		/// <param name="av_name">is name of the ActorValue or ExtraValue requested.</param>
		virtual void CheckActorValue(RE::ActorValue& av_ref, const char* av_name) = 0;

		/// <summary>
		/// Sets the AVDelay of an actor value. Currently unimplement.
		/// </summary>
		/// <param name="target"></param>
		/// <param name="actor_value"></param>
		/// <param name="new_delay"></param>
		/// <returns></returns>
		virtual bool SetAVDelay(RE::Actor* target, RE::ActorValue actor_value, float new_delay) = 0;

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

		HINSTANCE API = GetModuleHandle(ARITHMETIC_API_SOURCE_L);

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
#ifndef AVG_SOURCE

namespace AVG
{
	
	template<size_t N>
	struct StringLiteral {
		static constexpr size_t Size = N;
		constexpr StringLiteral(const char(&str)[N]) {
			std::copy_n(str, N, value);
		}

		constexpr operator const char* () { return &value; }


		constexpr operator std::string_view() { return std::string_view(&value, Size); }


		constexpr std::string_view view() { return std::string_view(&value, Size); }

		char value[N];
	};

	/// <summary>
	/// A class used to store and represent an actor value that may or may not be an Extra Value. Set with a string to get its EV.
	/// </summary>
	struct DynamicValueID
	{
		RE::ActorValue actorValue = RE::ActorValue::kNone;

		DynamicValueID() = default;

		constexpr DynamicValueID(RE::ActorValue a_rhs) : actorValue(a_rhs) {}

		template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
			DynamicValueID(StringType a_rhs)
		{
			if (ActorValueGeneratorAPI::RequestInterface())
				ActorValueGeneratorAPI::Interface->CheckActorValue(actorValue, a_rhs);
		}


		constexpr DynamicValueID& operator=(RE::ActorValue a_rhs)
		{
			actorValue = a_rhs;
			return *this;
		}


		template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
			DynamicValueID& operator=(StringType a_rhs)
		{
			if (ActorValueGeneratorAPI::RequestInterface())
				ActorValueGeneratorAPI::Interface->CheckActorValue(actorValue, a_rhs);
			return *this;
		}
	};

	/// <summary>
	/// A constant Value ID that holds onto the ActorValue of it's associated string.
	/// </summary>
	/// <typeparam name="AV_Name">The string literal name of the AV. Case insensitive.</typeparam>
	template <StringLiteral AV_Name>
	struct ConstValueID
	{
		//

		static constexpr auto ActorValueName = AV_Name.value;

		operator RE::ActorValue() const
		{
			if (ActorValueGeneratorAPI::RequestInterface())
				ActorValueGeneratorAPI::Interface->CheckActorValue(_actorValueID, ActorValueName);

			return _actorValueID;
		}

	private:
		static inline RE::ActorValue _actorValueID = RE::ActorValue::kNone;
	};
}

#endif
