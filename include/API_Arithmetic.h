#pragma once

#ifndef ARITHMETIC_API_SOURCE
static_assert(false, "Macro 'ARITHMETIC_API_SOURCE' isn't defined.");
#endif

template <class T>
struct ABIContainer
{
	T* _data = nullptr;
	std::uint64_t _size{};
		

	T& operator[] (std::uint64_t index) const
	{
		assert(index < _size);

		return _data[index];
	}

	uint64_t size() const
	{
		return _size;
	}
	ABIContainer() = default;
	ABIContainer(T* d, uint64_t s) : _data{ d }, _size{ s } {}
	ABIContainer(std::vector<T>& v) : _data{ v.data() }, _size{ v.size() } {}
};


namespace Arthmetic
{

	struct Target;
	struct ResolvedArg;
	struct DelegateArgument;

	//Currently unimplemented.
	using ParamConstraints = uint64_t;

#ifdef ARITHMETIC_SOURCE
	using SWITCH_ARG = DelegateArgument;
#else
	using SWITCH_ARG = ResolvedArg;
#endif


	using ArgumentList = ABIContainer<SWITCH_ARG*>;


#ifdef ARTH_OBJECT_TYPE
	using BaseObject = ARTH_OBJECT_TYPE;
#else 
	using BaseObject = void;
#endif


#ifdef ARTH_CONTEXT_TYPE
	using TargetContext = ARTH_CONTEXT_TYPE;
#else 
	using TargetContext = void;
#endif


#ifdef ARTH_ENUM_TYPE
	using TargetType = ARTH_ENUM_TYPE;
#else 
	using TargetType = std::uint32_t;
#endif

}


namespace ArithmeticAPI
{

	using NativeFormula = float(*)(Arthmetic::Target, const Arthmetic::ArgumentList);



	using LinkNatFunc = int(*)(std::string_view, int, NativeFormula);



	using SWITCH_ARG = Arthmetic::SWITCH_ARG;

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
		/// Should not be using manually. Returns a TargetObject from an Argument, throwing an exception back to ARTH if invalid.
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		[[nodiscard]] virtual Arthmetic::Target ArgAsObject(SWITCH_ARG* arg) = 0;


		/// <summary>
		/// Should not be using manually. Returns a Number from an Argument, throwing an exception back to ARTH if invalid.
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		[[nodiscard]] virtual float ArgAsNumber(SWITCH_ARG* arg) = 0;


		/// <summary>
		/// Should not be using manually. Returns a String from an Argument, throwing an exception back to ARTH if invalid.
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		[[nodiscard]] virtual const char* ArgAsString(SWITCH_ARG* arg) = 0;

		/// <summary>
		/// When a routine is flagged native this will be used to link a C++ function with it. Safe to call after PostPostLoad.
		/// </summary>
		/// <param name="routine_name">is the name of the routine you'd like this to define.</param>
		/// <param name="function">is the function you'd like the routine to call when executed.</param>
		/// <param name="constaints">are expected parameter constraints. Currently unimplemented, set to zero.</param>
		[[nodiscard]] virtual void LinkNativeFunction(std::string_view routine_name, NativeFormula function, Arthmetic::ParamConstraints constaints = 0) = 0;
	};

	using CurrentInterface = InterfaceVersion1;

	inline CurrentInterface* Interface = nullptr;



#ifdef ARITHMETIC_SOURCE 


	CurrentInterface* InferfaceSingleton();

	namespace detail
	{
		extern "C" __declspec(dllexport) void* ARTH_RequestInterfaceImpl(Version version)
		{
			CurrentInterface* result = InferfaceSingleton();

			switch (version)
			{
			case Version::Version1:
				return dynamic_cast<InterfaceVersion1*>(result);
			default:
				//Should show an error or something like that.
				return nullptr;
			}

			return nullptr;
		}
	}

#endif


	/// <summary>
	/// Accesses the Arithmetic Interface, safe to call PostLoad
	/// </summary>
	/// <param name="version"> to request.</param>
	/// <returns>Returns void* of the interface, cast to the respective version.</returns>
	inline void* RequestInterface(Version version)
	{
		typedef void* (__stdcall* RequestFunction)(Version);

		static RequestFunction request_interface = nullptr;

		HINSTANCE API = GetModuleHandle(ARITHMETIC_API_SOURCE_L);

		if (API == nullptr) {
			logger::critical("{} not found, API will remain non functional.", ARITHMETIC_API_SOURCE);
			return nullptr;
		}

		request_interface = (RequestFunction)GetProcAddress(API, "ARTH_RequestInterfaceImpl");

		if (request_interface) {
			if (static unsigned int once = 0; once++)
				logger::info("Successful module and request, ARTH");
			
		}
		else {
			logger::critical("Unsuccessful module and request, ARTH");
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

#ifndef ARITHMETIC_SOURCE

namespace Arthmetic
{
	/// <summary>
	/// The object that the routine was called on. A combinate of Context and a BaseObject.
	/// </summary>
	struct Target
	{
		
		BaseObject* focus = nullptr;
		TargetContext* context = nullptr;


		Target() = default;
		constexpr Target(BaseObject* _f) : focus(_f) {}
		constexpr Target(BaseObject* _f, TargetContext* _c) : focus(_f) {}
		Target& operator=(BaseObject* base) { focus = base; context = nullptr; return *this; }

		operator BaseObject* () const { return focus; }

		BaseObject* operator->() const
		{
			return focus;
		}
	};

	/// <summary>
	/// The arguments sent via routine. Cannot be created and are solely accessed via pointer.
	/// </summary>
	struct ResolvedArg
	{
		/// <summary>
		/// Treats the Argument like a TargetObject. Throws back to ARTH if invalid.
		/// </summary>
		/// <returns></returns>
		Target AsObject()
		{
			return ArithmeticAPI::RequestInterface()->ArgAsObject(this);
		}

		/// <summary>
		/// Treats the Argument like a Number. Throws back to ARTH if invalid.
		/// </summary>
		/// <returns></returns>
		float AsNumber()
		{
			return ArithmeticAPI::RequestInterface()->ArgAsNumber(this);
		}

		/// <summary>
		/// Treats the Argument like a string. Throws back to ARTH if invalid.
		/// </summary>
		/// <returns></returns>
		std::string AsString()
		{
			return ArithmeticAPI::RequestInterface()->ArgAsString(this);
		}

	private:


		
		ResolvedArg() = delete;
		ResolvedArg(const ResolvedArg&) = delete;
		ResolvedArg(ResolvedArg&&) = delete;

		ResolvedArg& operator=(const ResolvedArg&) = delete;
		ResolvedArg& operator=(ResolvedArg&&) = delete;
		~ResolvedArg() = delete;
	};
}
#endif

#ifdef ARITHMETIC_API_SOURCE
#undef ARITHMETIC_API_SOURCE
#endif
