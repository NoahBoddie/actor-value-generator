#pragma once

#include "RoguesGallery/Utility.h"
#include "Serialization/ISerializer.h"
#include "Serialization/SerialConstructor.h"
#include "Serialization/SerializableObject.h"
//#include "SerializingWrapper.h"
//free_rul::serializing





namespace AVG
{
	class ExtraValueData;
}


namespace RGL
{
	//TestZone, move me some place.

	constexpr string_hash serial_hash_integer(uint32_t i) noexcept
	{

		uint8_t lsb = i & 0xFF;

		i &= ~lsb;

		i |= (lsb & 0x0F) << 8;
		i |= (lsb & 0xF0) << 16;

		return i;
	}

	template<bool Insensitive = false>
	constexpr string_hash serial_hash(const std::string_view view) noexcept
	{
		constexpr HashFlags flag = Insensitive ? HashFlags::Insensitive : HashFlags::None;

		string_hash raw_hash = Hash<flag>(view.data(), view.size());

		return serial_hash_integer(raw_hash);
	}



	struct TypeHash
	{
		constexpr TypeHash() = default;

		constexpr TypeHash(const uint32_t arg) : code(serial_hash_integer(arg)) {}//arg == 0 ? 0xFFFFFFFF : serial_hash_integer(arg)) {}
		constexpr TypeHash(const char* arg) : code(serial_hash(arg)) {}//!arg || *arg == '\0' ? 0xFFFFFFFF : serial_hash(arg)) {}
		constexpr TypeHash(const std::string_view& arg) : code(serial_hash(arg)) {}//arg == "" ? 0xFFFFFFFF : serial_hash(arg)) {}



		constexpr static TypeHash RawHash(uint32_t arg)
		{
			TypeHash _hash{};

			_hash.code = arg;

			return _hash;
		}

		constexpr static TypeHash RawHash(std::string_view arg)
		{
			TypeHash _hash{};

			_hash.code = Hash(arg);

			return _hash;
		}

		constexpr static TypeHash Create(const char* arg)
		{
			TypeHash _hash{};


			_hash.code = serial_hash(arg);

			return _hash;
		}

		constexpr static TypeHash Create(uint32_t arg)
		{
			TypeHash _hash{};

			_hash.code = serial_hash_integer(arg);

			return _hash;
		}


		constexpr static TypeHash Create(std::string_view arg)
		{
			TypeHash _hash{};

			_hash.code = serial_hash(arg);

			return _hash;
		}

		uint32_t code{};


		operator uint32_t() const noexcept{ return code; }

		constexpr bool operator==(TypeHash& a_rhs) noexcept { return code == a_rhs.code; }

		constexpr bool operator!=(TypeHash& a_rhs) noexcept { return code != a_rhs.code; }
	};


	template <typename T> constexpr std::string_view type_name();

	template <>
	constexpr std::string_view type_name<void>()
	{
		return "void";
	}

	namespace detail {

		using type_name_prober = void;

		template <typename T>
		constexpr std::string_view wrapped_type_name()
		{
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
			#error "Unsupported compiler"
#endif
		}

		constexpr std::size_t wrapped_type_name_prefix_length() {
			return wrapped_type_name<type_name_prober>().find(type_name<type_name_prober>());
		}

		constexpr std::size_t wrapped_type_name_suffix_length() {
			return wrapped_type_name<type_name_prober>().length()
				- wrapped_type_name_prefix_length()
				- type_name<type_name_prober>().length();
		}

	} // namespace detail

	template <typename T>
	constexpr std::string_view type_name() {
		constexpr auto wrapped_name = RGL::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = RGL::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = RGL::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}
	

	struct DefaultSerialize;
	
	template <class A, class B>
	struct SerialVector;

	constexpr std::string_view testNAME = TypeName<std::vector<char>>::value;

	//static_assert(testNAME == "std::vector<char, std::allocator<char>>");
	
	struct TestStructForCereal
	{
		constexpr virtual TypeHash GetTypeHash()
		{
			//Returning a string will hash the string for a serialization hash,
			// returning a number will hash the number to be viable for use
			return "Test";
		}
	};


	template<class Type>requires(&Type::GetTypeHash != nullptr)
		constexpr TypeHash GetHashFromType(Type& target)
	{
		if constexpr (std::derived_from<Type, ISerializer>) {
			return target.GetTypeHash();
		}
		else {
			return Type::GetTypeHash();
		}
	}

	template<class Type>
	constexpr TypeHash GetHashFromType(Type& target)
	{
		return TypeName<std::remove_const_t<Type>>::value;
	}

	template<class T>
	bool operator==(T& a_this, TypeHash& a_rhs)
	{
		TypeHash type_hash = GetHashFromType(a_this);
		return type_hash.code == a_rhs.code;
	}


	template<class T>
	inline bool operator!=(T& a_this, TypeHash& a_rhs)
	{
		return (a_this == a_rhs) == false;
	}
	//This isn't really even going to be used, the check type hash is basically just the operator.
	template <class T>
	bool CheckTypeHash(T& target, TypeHash type_hash)
	{
		return target == type_hash;
	}
///End of test field




	enum class SerializingState
	{
		None,
		Serializing,
		Deserializing,
		//Dumping = 1 << 2, //Because I don't need to run through the objects, I don't actually need this.
		Finished = 1 << 3,	//In the event one has finished, this will be ticked. This will prevent any further deserializations, and return attempts false.
	};


	
	enum class SerializingFlag
	{
		None = 0,
		SerializeAsData = 1 << 0,	//Instead of opening a new record it will serialize the object as data to whatever record is currently open.
		OpenSerializing = 1 << 1	//When serializing a type that can't expose the OpenSerializer flag, this value will handle it for it.
	};


	class SerialArgument;
	class ISerializable;
	class ISerializer;
	

	//This needs to work in a reference to a pointer that is null, so calling it can possibly fill the value.
	template<typename T>
	concept null_serialize = requires(T v, SerialArgument& a, bool& b)
	{
		{ HandleSerializeNullptr(v, a, b) } -> std::same_as<void>;
	} && !requires(T v, int a) {
		{ HandleSerializeNullptr(v, a, DefaultType::value) };
	} && !requires(T v, bool b) {
		{ HandleSerializeNullptr(v, DefaultType::value, b) };
	};
	//std::pointer_traits<T>::pointer
	//template<typename T>
	//concept pointer_type = !std::is_same_v<void, typename std::pointer_traits<T>::pointer>;
	
	/*
	template<typename T>
	concept pointer_type = requires(T v)
	{
		std::is_same_v<typename std::remove_pointer<T>::pointer, T>;
	}
	//*/

	template<typename T>
	concept is_pointer = std::is_pointer_v<T>;
	/*
	//find out if the type overloads the arrow operator
	template<class T>
	concept pointer_type = requires (T t)
	{
		{ std::is_pointer_v<typename std::pointer_traits<T>::pointer> };
		//{ is_pointer<T> };
	} || requires (T t)
	{
		{ is_pointer<T> };
	};
	//*/
	template <typename T>
	concept pointer_type = std::is_pointer_v<T> ||
		(requires(T pointer) {
			{ pointer.operator->() } -> std::same_as<typename std::remove_reference_t<T>::pointer > ;
	}
	&& (requires(T pointer) {
		{ pointer.operator*() } -> std::same_as<typename std::add_lvalue_reference<typename std::remove_reference_t<T>::element_type>::type>;
	})
	);

	static_assert(pointer_type<std::unique_ptr<int>&>, "TEST");

	//Main take aways.
	// Bit messy (obvs)
	// needs more organization
	// try using constaints to contain what types can and cannot be used. perhaps have different functions
	//  for serializable object and serial constructors and regular stuff?
	// Make the effects of using the wrong version more transparent, possibly have a logging system for 
	//  errors so people can submit them to me.

	//This isn't super proper for here, but I would like to store plugin headers as the first thing. If there is no plugin
	// header, it will load nothing treating it as a new game. Plugin header will be the thing that loads the current version
	// (because I need that info, might as well use it).


	struct SerializingID
	{

		//These are special reserved id's for functionality. 
		//The first type can only be used at depth zero, as the moment this object is created it (later) be the first thing
		// serialized/deserialized.
		//The second type is used to denote that the data seen has been dumped and thrown out.

		//static constexpr inline uint32_t BufferHeader = 'HEAD';
		//static constexpr inline uint32_t DataDumped = 'DUMP';
		static constexpr inline TypeHash BufferHeader = TypeHash::RawHash('HEAD');
		static constexpr inline TypeHash DataDumped = TypeHash::RawHash('DUMP');
		static constexpr inline TypeHash acceptAll = TypeHash::RawHash(0xFFFFFFFF);
	};

	
	//Make a get hash code function maybe?
	// Hash codes can change, you need to make a manual serialize hash code, but I sorely want the hash to be something
	// that doesn't change, but also something that sticks based on intent. So probably something quite manually
	// assigned, so I don't have serialization issues. Perhaps a hash made based on the name of the type?


	//Hash code idea number 2
	// the first four bits are mine, they denote pointing locations. IE
	//  is this a combat data I'm supposed to be creating, or is this supposed to go to the thingy majig, whatever
	// To determine the value that I get for the hash, I'll use the type name. Then I'll compare the types.
	// NOTICE, this might not be super great. Template types don't go down, and std::location won't work well for 
	//  ints or other types without functions.
	// Idea complete, consult keep notes.
	//inline std::int32_t REPLACE_TEST_VAR = 0;

	//This is the object that largely makes the whole serialization bit, it's a sorta singleton, though
	// that's not enforced just in principle. This thing gets made, and it should 
	//This is basically created at the start of the core function for serializing, and this pointer is passed around
	// so long as this main function is being used.

	//The core serialization elementmade with a state and the interface pointer, it drives the functions that interpret, reinterpret,
	// save, and load data.
	class SerialArgument
	{
		//These should probably be uint16_t's but eh.
		static uint32_t split_v_process(uint16_t size, uint8_t depth)
		{
			uint32_t whole_number = static_cast<uint32_t>(size) << 16;

			whole_number |= depth;

			return whole_number;

		}

		//Will likely become a tuple
		static std::pair<uint16_t, uint8_t> split_v_process(uint32_t version)
		{
			std::pair<uint16_t, uint8_t> split{};

			split.first = static_cast<uint16_t>(version >> 16);
			split.second = static_cast<uint8_t>(version & 0xFF);

			return split;
		}

		struct CurrentProcessData
		{
			//std::uint32_t type = 0;//This is represented by a string hash
			TypeHash type = TypeHash();//This is represented by a string hash
			std::uint32_t version = 0;//I think I was gonna use version for something else, if not change to be the new version object later.
			std::uint32_t length = 0;//This is the data length, but not explicitly the classes size.

			//This value is ignored when a serializer is encountered, as the size of one is not important
			//If the size of some raw data or serializable has changed, it will fail to serialize.
			uint16_t classSize = 0;
			uint8_t serialDepth = 0;

			bool readyForNext = false;

			uint32_t GetID() { return _processID; }

		private:

			uint32_t _processID = 0;
		public:
			void operator()(uint32_t t, uint32_t v, uint32_t l, bool ready)
			{
				

				type = TypeHash::RawHash(t);
				length = l;
				version = v;//For now.
				auto split = split_v_process(v);
				
				classSize = split.first;
				serialDepth = split.second;

				readyForNext = ready;

				_processID += 1;
			}
		};


		struct DepthDelegate
		{
			inline void operator()(bool& is_valid, SerialArgument& focus, uint8_t& original_value, uint8_t set_value)
			{
				original_value = focus._depth;
				logger::info("Start: {} -> {}", original_value, set_value + original_value);

				focus.ModDepth(set_value);
			}

			inline void operator()(SerialArgument& focus, uint8_t& original_value, uint8_t set_value)
			{
				logger::info("Finish: {} -> {}", focus._depth, focus._depth - set_value);//Log these.

				focus.ModDepth(-set_value);
			}
		};

		//I keep these.
		static void DepthFlagStart(bool& is_valid, SerialArgument& focus, uint8_t& original_value, uint8_t set_value)
		{
			original_value = focus._depth;
			logger::info("Start: {} -> {}", original_value, set_value + original_value);

			focus.ModDepth(set_value);
		}

		static void DepthFlagFinish(SerialArgument& focus, uint8_t& original_value, uint8_t set_value)
		{
			logger::info("Finish: {} -> {}", focus._depth, focus._depth - set_value);//Log these.

			focus.ModDepth(-set_value);
		}



		//Note, the true name should be, SerialBuffer, short for serialization buffer.


		//For the existence of handling primary, when a session opens, it will keep track of the amount of bytes
		// it handles, and the amount it successfully handles, to determine how much data was properly handled.





	private:
	public:
		//Primarily only really to be used in deserialization.
		CurrentProcessData _processData;
		
		
		using SerializableLockFlag = TemporaryFlag::Variable<bool>;
		using SerializationDepthFlag = TempFlag<SerialArgument, uint8_t, DepthDelegate>;
		using OpenRecordFlag = TemporaryFlag::Variable<bool>;



		uint8_t _depth = 0;
		uint8_t _dumpDepth = 0;

		void SetDepth(uint8_t new_depth)
		{
			auto old_depth = _depth;

			_depth = new_depth;

			if (_dumpDepth != 0 && _depth >= _dumpDepth) {//End dump order
				FinishDumpOrder();
			}

		}


		uint8_t ModDepth(uint8_t mod_depth)
		{
			uint8_t new_depth = _depth + mod_depth;

			SetDepth(new_depth);

			return new_depth;
		}


		uint8_t GetDepth()
		{
			return _depth;
		}
		

		//I actually don't need these anymore, as this current system means I no longer actually need to handle it like this.
		/// <summary>
		/// A function that is used to prevent data from being serialized, or toss out data that is being deserialized
		/// </summary>
		void SendDumpOrder(uint8_t new_order = 0)
		{
			new_order = !new_order ? _depth : new_order;

			if (new_order <= _dumpDepth)
				return;
			
			//serialState |= SerializingState::Dumping;

			_dumpDepth = new_order;
		}

		void FinishDumpOrder()
		{
			//serialState &= ~SerializingState::Dumping;
			_dumpDepth = 0;
		}

		bool _handlingPrimary = false;

		bool _serializableLock = false;


		void Finish() { logger::info("Finished."); serialState |= SerializingState::Finished; }
	public:
		constexpr bool IsFinished() noexcept { return Any(serialState, SerializingState::Finished); }

		SKSE::SerializationInterface* serialInterface;
		//std::uint32_t interfaceVersion = GetProjectVersion();
		std::uint32_t interfaceVersion = SKSE::PluginDeclaration::GetSingleton()->GetVersion().pack();
		SerializingState serialState;

		SerialArgument(SKSE::SerializationInterface* enter_face, SerializingState state)
		{
			serialInterface = enter_face;
			serialState = state;
			
			switch (serialState)
			{
			case SerializingState::Serializing:
				//temporarily doesn't use version.
				serialInterface->WriteRecord(SerializingID::BufferHeader, interfaceVersion, 'H');
				//This needs to open a new record, to seperate all data from the header.
				break;
			case SerializingState::Deserializing:
			{
				uint32_t value_dmp1 = 0;
				uint32_t value_dmp2 = 0;
				if (serialInterface->GetNextRecordInfo(value_dmp1, interfaceVersion, value_dmp2) == false){
					//Finish, log error.
					logger::warn("No values stored");
				}
				else {
					logger::debug("{} length", value_dmp2);
				}
			}
			break;

			default:
				//Invalid, crash probably.
				logger::error("Invalid SerialBuffer state, {}", (int)state);
				break;
			}

			//I would set depth to 1, but 0 is the primary serializing object.
		}


		static inline std::int32_t temp_Version = 1;



		//The requirements to use this would be, if the object is not a serializable, and if the serializable lock is not going on right now.
		SerializationDepthFlag GetDepthHandle(bool active)
		{
			SerializationDepthFlag flag{};
			if (active)//For now it will force a new one to open when used in a viable manner. This will have to change however (for serial iterators)
				flag(this, 1);

			return flag;
		}

		//Additional information I would like to store
		// How many times this function has been called. 1 for non-primary, and then how many times it's
		// been called for non-primary targets. Perhaps records of how many each record type had,
		// so I can walk through the values.



		//This is for the special types that are wrappers. Needs to be repurposed as a seperate function for serialization
		// instead of a GetSerialized version of it.
		//You also might be wondering why there's a before and after setting. Its mainly because I don't want more functions.
		// This handles The Post serialization stuff for serializable object too.
		template <class Type> requires(std::derived_from<Type, ISerializable>)
		Type GetSerialized(Type& target, bool& success, Type* pre_deserialize = nullptr)
		{
			static_assert(
				&Type::HandleSerialize != &ISerializable::HandleSerialize ||
				&Type::OnSerialize != &ISerializable::OnSerialize ||
				&Type::OnDeserialize != &ISerializable::OnDeserialize,
				"At least one serialization function of ISerializable must be overloaded (not overriden).");
			
			static_assert(!std::is_const_v<Type>, "Const not allowed here.");

			SerialArgument& a_this = *this;
			//A note for this, if it's flagged false for success, it shouldn't flag be able to be flagged positive again.
			// Might make a type for that.

			bool original_lock = _serializableLock;
			
			_serializableLock = true;
			
			if (!pre_deserialize)
			{
				if (!original_lock)	//If this is the first serializable object.
				{
					//Serializable Bytes are decreed no longer need.
					//SerializableBytes bytes = target.GetSerialized_EXTERNAL<Type>(target, success);
					//Type serial_ready = bytes.GetSerializableObject(target, success);

					Type serial_obj(target);

					serial_obj.HandleSerialize(a_this, success, &target);
					serial_obj.OnSerialize(a_this, success, &target);
					

					if (!success) {
						RE::DebugMessageBox("FAILR");
					}

					return serial_obj;
				}
				else
				{
					//Doing this on target in the middle of serialization looks weird, but note, this part should only happen
					// when a copied serializable is serializing something.
					//RE::DebugMessageBox("Serializing internal object.");
					target.HandleSerialize(a_this, success, &target);
					target.OnSerialize(a_this, success, &target);
					
					return target;
				}
			}
			
			target.HandleSerialize(a_this, success, pre_deserialize);
			target.OnDeserialize(a_this, success, pre_deserialize);

			return target;
		}
		
		template <class Type> requires(!std::derived_from<Type, ISerializable>)
		Type GetSerialized(Type& target, bool& success ,Type* pre_deserialize = nullptr) { return target; }



		template <std::derived_from<ISerializer> Type>// requires(std::derived_from<Type, ISerializer>)
		bool HandleSerialization(Type& target)
		{
			bool success = true;

			if (std::is_const<Type>::value)
			{
				auto& non_const_target = const_cast<Type&>(target);

				non_const_target.HandleSerialize(*this, success);
			}
			else
			{
				target.HandleSerialize(*this, success);
			}
			
			return success;

		}
		
		template<class Type>requires(&Type::IsRelevant != nullptr)
		inline bool CheckSkipSerialization(Type& target)
		{
			//The things that should come equipped with this are serialization handlers primarily. It can set it up as a virtual function, and as far
			// as base classes that use IsSerialRelevant, it will be the one of only one that will do so (given it's one of the only with virtual funcs in mind.)
			// SerialComponents will have this function as well, but it will remain largely unimplemented unless the derived one wished it to.
			//Serializable and ISerializer classes can implement these at their leisure, as without a vtable there's no way for those ones to be encountered anyways.

			//A note however, I'm unsure how to deal with the prospect of  the require constraint dealing with vtabled functions.
			//This has some good info on that possibility.
			//https://stackoverflow.com/questions/4741271/ways-to-detect-whether-a-c-virtual-function-has-been-redefined-in-a-derived-cl

			return target.IsRelevant();
		}

		template<class Type>
		inline bool CheckSkipSerialization(Type& target)
		{
			return false;
		}



		bool IsHandlingPrimary() { return _handlingPrimary; }

		constexpr bool IsSerializing() { return Any(serialState, SerializingState::Serializing); }
		constexpr bool IsDeserializing() { return Any(serialState, SerializingState::Deserializing); }



		//This function should be done at the bottom of serialize, that way I really don't need to worry about whether it's been done or not, and I can access
		// the process data at my leisure.
		bool GetNextRecordInfo(bool readyForNext)
		{
			if (IsFinished() == true)
				return false;

			std::uint32_t type = 0;
			std::uint32_t version = 0;
			std::uint32_t length = 0;
			logger::debug("hit");
			if (serialInterface->GetNextRecordInfo(type, version, length) == false) {
				//Submit error, no records available.
				logger::info("No more records");
				_finished = true;
				Finish();
				return false;
			}

			_processData(type, version, length, readyForNext);
			
			return true;
		}
		

		bool GetNextRecordIfNotReady()
		{
			if (_processData.readyForNext)
				return true;

			return GetNextRecordInfo(true);
		}


		void SetCurrentRecordReadiness(bool value) { _processData.readyForNext = value; }


		
		void DumpDepth(uint8_t release_depth, bool serialize = false)
		{
			//NOTE!, this shouldn't be the publically available version. That version should have introduction checking, and shouldn't allow custom release depths

			//Zero should not be a viable answer.
			//What should happen if the starting depth doesn't meet the process depth?0

			//If this is serializing, it should save DUMP data instead.

			if (IsFinished() == true)
				return;

			if (IsSerializing() == true)
			{
				if (serialize)//This is used to not mess with the flow of the data, however I want to have some stronger type safety on this.
					serialInterface->WriteRecord(SerializingID::DataDumped, release_depth, release_depth);

				return;
			}

			std::uint32_t type = 0;
			std::uint32_t version = 0;
			std::uint32_t length = 0;



			uint8_t depth = 0;
			
			logger::info("Release depth: {}", release_depth);

			do
			{
				if (serialInterface->GetNextRecordInfo(type, version, length) == false) {
					_finished = true;
					Finish();
					return;//Say final	
				}

				depth = static_cast<uint8_t>(version & 0xFF);

				logger::info("Dump depth: {}", depth);

			} 
			while (depth > release_depth);
			
			//If the process data did not end, and the depth was greater, set the read record to the process data.
			_processData(type, version, length, true);

		}

		//While this seems to be what I initially desired, anything past seems to be a struggle
		inline void DumpDepth() { return DumpDepth(_depth + 1, false); }

		//Before we increment depth
		//If we are deserializing, and it's a type that wishes to make records, and isn't flagged to not pull records, pull records.
		//If the depth is wrong, or the length is different than it is now, it will dump data until it gets to the previous depth.

		//Addition serialization tidbit, the first object deserialized is one of it's own.
		bool _finished = false;

		bool _temp_hasGottenFirst = false;
		//bool _temp_mustPushFirst = false;
		//Make alias functions for only intro version and only serializing flag.
		// but I don;t think I actually need that amymore..
		//Also incompatible version is important. This is if a version does something different with the information, and is no longer compatible.
		
		//When you move this, really research this shit.
		template<typename Test, template<typename...> class Ref>
		struct is_specialization : std::false_type {};

		template<template<typename...> class Ref, typename... Args>
		struct is_specialization<Ref<Args...>, Ref> : std::true_type {};


		//*
		template <class Type>requires(!pointer_type<Type>)//requires(!std::is_pointer_v<Type>)//
		bool Serialize(Type& target, std::uint32_t intro_version = 0, std::uint32_t incompatible_version = 0, SerializingFlag flags = SerializingFlag::None)
		{

			constexpr uint32_t acceptAll = 0xFFFFFFFF;
			TypeHash type_code = GetHashFromType(target);;
			//type_code = SerializingID::acceptAll;

			//static_assert(is_specialization<target, std::unique_ptr>::value, "ERROR, unique_ptr found");
			//I would like to rely on these in order to handle most of the serializing. But for not, don't rock the boat.
			bool is_data = _serializableLock || Any(flags, SerializingFlag::SerializeAsData);

			if (!is_data && IsDeserializing() == true) {
				GetNextRecordIfNotReady();
			}

			//if (!_temp_hasGottenFirst && IsDeserializing() == true) {
			//	GetNextRecordInfo(true);
			//	_temp_hasGottenFirst = true;
			//}

			if (IsFinished() == true)
			{
				//no more data ey
				logger::warn("Last Object \"{}\" won't de/serialize. Buffer finished.", typeid(Type).name());
				return false;
			}
			
			
			//if (is_header) do header stuff.

			//I would like some length and depth (version) checks at some point later. Now, while depth sits empty here, version can also be used to check
			// true types, which would be useful for dump commands and such, so I can know the true types.
			//I would also like to use temporary flags for my own convience.

			logger::info("handling: {}", typeid(Type).name());

			//If it's serializing, opposite this there should be something that checks for the function serialization relevant.
			if (IsDeserializing() == true)
			{
				if (intro_version > interfaceVersion) {//No data exists, keep going.
					//LOGTHIS
					return true;//I'm unsure if I want to return true or false.
				}
				// is \/ this right?
				//Data exists but is invalid, dump OR data exists but only maintains order.
				else if (incompatible_version >= interfaceVersion || _processData.type == SerializingID::DataDumped) {
					//Data is incompatible, but still exists. Dump it, then return.
					//LOGTHIS
					DumpDepth(_depth);
					return true;
				}
				//There should be a switch statement for _processData.type == SerializingID::DataDumped instead, there are other special types.
			}


			if (IsSerializing() == true && CheckSkipSerialization(target) == true) {
				logger::info("Object \"{}\" has irrelevant data. Dumping records.", typeid(Type).name());

				DumpDepth(_depth);
				return true;
			}
			//auto original_depth = _depth;
			bool original_lock = _serializableLock;
			
			constexpr static bool is_serializable = std::derived_from<Type, ISerializable> == true;

			auto original_depth = _depth;


			auto depth_handle = GetDepthHandle(!is_serializable || !original_lock);

			//SerializableLockFlag serial_lock_handle{};//Doesn't seem to need atm. But still should use.

			bool return_value = true;


			//I think type code checking can actually go some where around here.



			//I would like to fix this bit
			if constexpr (std::derived_from<Type, ISerializer> == true)
			{
				bool is_open_serializing = is_open_serializer<Type> || Any(flags, SerializingFlag::OpenSerializing);//(flags & SerializingFlag::OpenSerializing) != SerializingFlag::None;//
				//Make 2 versions of this, 1 for serialization handler, and another for if it's not. 
				// If it's not, I can just use open serializer, and a constexpr if.
				// If it is, I need to shake it down, but it will only be for the crossroad types.
				//Rather, so it works with newer types that are also more crossroadsy, make it check if it's abstract.

				//Important to note, this doesn't fucking work either.
				if (is_open_serializing)
				{
					if (IsSerializing() == true) {
						serialInterface->OpenRecord(type_code, split_v_process(sizeof(Type), original_depth));
					}
					else if (IsDeserializing() == true) {//But if it's deserializing I want to make sure the types match.
						//Clean this up.
						//if (type_code != _processData.type && _processData.type != acceptAll) {
						//I REALLY would like the regular operator to fucking work please.
						if (!CheckTypeHash(target, _processData.type) && _processData.type.code != SerializingID::acceptAll.code) {
							logger::error("Process typecode {:X} does not equal current type code {:X} for {}.", _processData.type, type_code, TypeName<Type>::value);
							DumpDepth(original_depth);
							return false;
						}
						else if (_processData.serialDepth != original_depth)
						{
							logger::error("ERROR: process depth {} does not equal current depth {}.", _processData.serialDepth, original_depth);
							DumpDepth(original_depth);
							return false;
						}


						//If the next thing does something, it will undo this flag//This didn't work, it did not work.
						SetCurrentRecordReadiness(false);
					}
					logger::debug("<!> Open encounter {}", _processData.GetID());

				}

				uint32_t last_id = _processData.GetID();
				
				return_value = HandleSerialization(target);

				if (is_open_serializing)
				{

					//This should only do something if handle serialization did nothing, which is possible.
					if (last_id == _processData.GetID() && IsDeserializing() == true) {
						logger::info("nothing happened?, {} | {}", last_id, _processData.GetID());
						GetNextRecordInfo(true);
					}
				}
			}
			else
			{
				if (IsSerializing() == true)
				{
					using NonConstType = std::remove_const_t<Type>;
					//CLEAN ME, this thing is a nightmare
					NonConstType& safe_target = const_cast<NonConstType&>(target);//const_cast<Type*>(&target);

					NonConstType serial_object = GetSerialized(safe_target, return_value);

					//auto serial_ptr = std::bit_cast<void*>(&target);

					if (return_value && !original_lock)
					{
						//If not forced to write as data, write it as the whole record
						if ((flags & SerializingFlag::SerializeAsData) != SerializingFlag::None) {
							logger::debug("Saved as Data");

							return_value = serialInterface->WriteRecordData(serial_object);
							//serialInterface->WriteRecord(REPLACE_TEST_VAR, temp_Version, &target, sizeof(Type));
							//result = serialInterface->WriteRecord(REPLACE_TEST_VAR, temp_Version, serial_ptr);
						}
						else {
							logger::debug("Saved as Record");
							return_value = serialInterface->WriteRecord(type_code, split_v_process(sizeof(Type), original_depth), serial_object);
							//result = serialInterface->WriteRecordData(serial_ptr);
						}
						if (!return_value) {
							//RE::DebugMessageBox(std::format("Value Serialize Failure in {}", typeid(serialObject).name()));
							logger::error("Value Serialize Failure in {}", typeid(serial_object).name());
						}
					}
				}
				else if (IsDeserializing() == true)//Getting rid of this, temporarily just to see if it'll work without it
				{
					using NonConstType = std::remove_const_t<Type>;

					//CLEAN ME, this thing is a nightmare
					NonConstType* test = const_cast<NonConstType*>(&target);//const_cast<Type*>(&target);
					//auto setTo = target;//Can't work because of const variables in the pairs. Fix first.

					//When deserializing, create a new value, then use equals to set it at the reference.
					// if reading record data doesn't use equals, I'm going to want to force it to.
					// by using equals like this, what I can effectively do is remove the need to have a specific way to handle
					// stuff like function pointers, instead just using it's equals function.
					//That, or make dedicated function, cause stuff like serializable objects ARE the ones using this.

					//Might be better to implement a copy constructor.
					//Testing remove const.
					NonConstType deserialized;

					//Might not be needed?


					//GetNextRecordIfNotReady();


					//If the serializable was locked when this function started, OR the reading was successful, continue.

					//I was going to make it so it's only if it's the first, but I also just realized, its better if it inside checks too.
					//  This would actually be checking the wrong one though wouldn't it?
					//It would seem I cannot size check internal data smartly. All I could do is remove the size as the internals is being processed.
					if (!original_lock && sizeof(Type) != _processData.classSize)
					{
						logger::info("Value Deserialize Failure for {}, size {} doesn't equal process size {}.", typeid(deserialized).name(), sizeof(Type), _processData.classSize);
						DumpDepth(_depth);
						_serializableLock = original_lock;
						return false;
					}
					else if (!original_lock && _processData.serialDepth != original_depth)
					{
						logger::info("ERROR: process depth {} does not equal current depth {}.", _processData.serialDepth, original_depth);
						DumpDepth(_depth);
						_serializableLock = original_lock;
						return false;
					}
					//NEW!
					else if (!CheckTypeHash(target, _processData.type) && _processData.type.code != SerializingID::acceptAll.code) {
						logger::error("Process typecode {:X} does not equal current type code {:X} for {}.", _processData.type, type_code, TypeName<Type>::value);
						DumpDepth(_depth);
						_serializableLock = original_lock;
						return false;
					}

					//if (serialInterface->ReadRecordData(test, sizeof(Type)) == false) {
					if (!original_lock && serialInterface->ReadRecordData(deserialized) == false) {
						//if (!original_lock && true == false) {
							//if (serialInterface->ReadRecordData(setTo) == false) {
							//RE::DebugMessageBox(std::format("Value Deserialize Failure for {}.", typeid(deserialized).name()));
						logger::critical("Value Deserialize Failure for {}.", typeid(deserialized).name());
						throw nullptr;//temporary
						//return_value = false;//Whether this is allowed to escape is a thing that has to be questioned a bit more.

						_serializableLock = original_lock;


						return false;
					}
					else {
						//RE::DebugMessageBox("Value Deserialize Success");
						//if the original state of the lock was in true, we don't want to use th derserialized object, instead test is already
						// derserialized. If it's false however, this is the first object and is seeking to be deserialized.
						auto& focus = original_lock ? *test : deserialized;

						//if (original_lock)
						//	RE::DebugMessageBox(std::format("using go to loc for {}.", typeid(deserialized).name()));

						GetSerialized(focus, return_value, test);

						//Check for original lock may not be required
						if (return_value && !original_lock)
							*test = deserialized;
					}
					//This should NOT happen if the thing getting deserialized is not going to be be reading data.
					// IE original lock, or serialized as data explicitly. In the last instance, I think I'll need a wait and see or something.
					//SO for example, this would close too early when deserializing timers.
					if (!original_lock && (flags & SerializingFlag::SerializeAsData) == SerializingFlag::None)
						GetNextRecordInfo(true);
					else
						SetCurrentRecordReadiness(false);
					//target = setTo;
				}
			}
			//Would like to expire this at some point.
			_serializableLock = original_lock;


			return return_value;
		}
		/*/
		template <class Type>
		bool Serialize(Type& target, std::uint32_t intro_version = 0, std::uint32_t incompatible_version = 0, bool deprecated = false)
		{
			//If deprecated skip. This check should happen post info load, so it can toss it if it matches
			//If the introduced version is less than the serialized version and deserial, skip pre info load, as it should have nothing
			//If the if the current version is under the incompatible version, it will skip post load, as the data should be tossed.

			bool isPrimaryProcess = !_handlingPrimary;

			if (isPrimaryProcess) {
				_handlingPrimary = true;

				std::uint32_t type;
				std::uint32_t version;
				std::uint32_t length;

				if (IsDeserializing() == true) {//Want this to go through even if constructor
					if (serialInterface->GetNextRecordInfo(type, version, length) == false) {
						//Submit error, no records available.
						RE::DebugMessageBox("No Records");
						_handlingPrimary = false;
						return false;
					}

					if (1 != 1)
					{
						if (intro_version > version) {
							//Is new object, don't deserialize, it will throw off the count.
							RE::DebugMessageBox("des A");
							_handlingPrimary = false;
							return false;
						}
						//While both of these do seek to go next, unlike the target version they don't leave it open,
						// as it's changed so the data gets thrown out.
						//else if (typeid(Type)..hash_code() != type){ }//This is useless as is, cause the types may move.
						// Need a more definite hash if I'm going to do this.
						else if (incompatible_version < version) {
							//The version is beneath the compatible version, don't deserialize, it won't end well.
							RE::DebugMessageBox("des B");
							_handlingPrimary = false;
							return false;
						}
					}
				}
			}

			bool return_value = true;

			bool original_lock = _serializableLock;


			//I would like to fix this bit
			if constexpr (std::derived_from<Type, ISerializer> == true)
			{
				if (isPrimaryProcess && IsSerializing() == true)//If the primary record and is serializing, open the record.
					serialInterface->OpenRecord(REPLACE_TEST_VAR, interfaceVersion);

				//auto handler = (SerializationHandler*)(&target);//CONFUTE
				//if (handler)
				//	handler->HandleSerialize(*this);


				return_value = HandleSerialization(target);
			}
			else if (IsSerializing() == true)
			{
				auto serialObject = GetSerialized(target, return_value);

				//auto serial_ptr = std::bit_cast<void*>(&target);

				if (return_value && !original_lock)
				{
					bool result = false;

					//We wanna write record the entire record if this is a primary process, wight data if not.
					if (isPrimaryProcess)
						//serialInterface->WriteRecord(REPLACE_TEST_VAR, temp_Version, &target, sizeof(Type));
						result = serialInterface->WriteRecord(REPLACE_TEST_VAR, temp_Version, serialObject);
					//result = serialInterface->WriteRecord(REPLACE_TEST_VAR, temp_Version, serial_ptr);
					else
						result = serialInterface->WriteRecordData(serialObject);
					//result = serialInterface->WriteRecordData(serial_ptr);

					if (!result) {
						RE::DebugMessageBox(std::format("Value Serialize Failure in {} process", isPrimaryProcess ? "primary" : "secondary"));
					}
				}
			}
			else if (IsDeserializing() == true)//Getting rid of this, temporarily just to see if it'll work without it
			{
				//CONFUTE?
				//auto test = std::bit_cast<void*>(&target);
				Type* test = const_cast<Type*>(&target);
				//auto setTo = target;//Can't work because of const variables in the pairs. Fix first.

				//When deserializing, create a new value, then use equals to set it at the reference.
				// if reading record data doesn't use equals, I'm going to want to force it to.
				// by using equals like this, what I can effectively do is remove the need to have a specific way to handle
				// stuff like function pointers, instead just using it's equals function.
				//That, or make dedicated function, cause stuff like serializable objects ARE the ones using this.

				//Might be better to implement a copy constructor.
				Type deserialized;

				//If the serializable was locked when this function started, OR the reading was successful, continue.

				//if (serialInterface->ReadRecordData(test, sizeof(Type)) == false) {
				if (!original_lock && serialInterface->ReadRecordData(deserialized) == false) {
					//if (serialInterface->ReadRecordData(setTo) == false) {
					RE::DebugMessageBox(std::format("Value Deserialize Failure for {}.", typeid(deserialized).name()));

					//I don't like this, there's a lot of boiler plate that needs to be done at the end of this thing.

					if (isPrimaryProcess)
						_handlingPrimary = false;

					_serializableLock = original_lock;

					return false;
				}
				else {
					//RE::DebugMessageBox("Value Deserialize Success");
					//if the original state of the lock was in true, we don't want to use th derserialized object, instead test is already
					// derserialized. If it's false however, this is the first object and is seeking to be deserialized.
					auto& focus = original_lock ? *test : deserialized;

					//if (original_lock)
					//	RE::DebugMessageBox(std::format("using go to loc for {}.", typeid(deserialized).name()));

					GetSerialized(focus, return_value, test);

					//Check for original lock may not be required
					if (return_value && !original_lock)
						*test = deserialized;
				}


				//target = setTo;
			}

			_serializableLock = original_lock;



			if (isPrimaryProcess)
				_handlingPrimary = false;

			return return_value;
		}

		//*/
		//template <class PointerType>requires(pointer_type<PointerType>)//requires(std::is_pointer_v<PointerType>)//
		template <class PointerType>requires(pointer_type<PointerType>)//requires(std::is_pointer_v<PointerType>)//
		bool Serialize(PointerType& target, std::uint32_t intro_version = 0, std::uint32_t incompatible_version = 0, SerializingFlag flags = SerializingFlag::None)
		{
			///Ramble Block
			//An interesting functionality for this could be that if there's a flag available for it, one can just have
			// this create it if the serialization is successful.

			//The main issue is thus, if something is a pointer, I'll need something to use so I can call serialize on it, open up
			// the record to begin dumping information. Largely, this goes into the other one, but it's basically to say that the above
			// needs a "heading" version of serialize, that both the above and this one will use, but something that allows me to use
			// one but not the other.
			///Ramble over~

			//I think this is currently unused.
			using ElementType = std::pointer_traits<PointerType>::element_type;//std::remove_pointer_t<PointerType>;//
			//using ElementType = std::conditional_t<std::is_pointer_v<PointerType>, std::remove_pointer_t<PointerType>, PointerType::element_type>;

			using Ptr = ElementType*;

			//struct block;
			//using block_ptr_t = typename std::pointer_traits<Ptr>::template rebind<block>;
			//struct block { std::size_t size{}; block_ptr_t next_block{}; };
			//block_ptr_t ptr_trait;
			
			Ptr _target = std::to_address(target);
			//Ptr _target = std::pointer_traits<PointerType>::to_address(target);

			//static_assert(!std::is_same_v<Ptr, PointerType>, "ERROR NOT SAME TYPE");

			if (!_target) {
				if constexpr (null_serialize<ElementType>)
				{
					logger::warn("Nullptr encountered during serialization. Using Nullptr serialize method");
					bool success = true;
					ElementType::HandleSerializeNullptr(*this, success);
					return success;
				}
				else
				{
					logger::info("Target nullptr, Dumping.");
					DumpDepth();
					return false;
				}
			}
			logger::info("Target exists, Serializing.");

			// This should actually throw an error when serializing, to symbolize, while you can compile that, it will not allow you to serialize
			// a pointer, and instead it needs to be handled in some way.
			return Serialize(*_target, intro_version, incompatible_version, flags);
		}
		
		//This function needs a few different versions. One should take a type code, one should take a string, and another should be an inlined template
		// function. These are all safeguards from getting the wrong type.
		//*For now, should remain unused.
		void DumpRecord(uint32_t intro_version) 
		{
			if (intro_version > interfaceVersion)
				return;
		}

		template <class Type>//Need this to be aliased or something.
		bool DumpIfFailure(Type& target, std::uint32_t intro_version = 0, std::uint32_t incompatible_version = 0, bool success = false) 
		{
			if (success) {
				logger::info("Is Successful: true! Serializing.");
				return Serialize(target, intro_version, incompatible_version);
			}
			
			logger::info("Is Successful: false! dumping.");


			DumpDepth();

			return false;
		}

		template <class Type>//Need this to be aliased or something.
		inline bool DumpIfFailure(Type& target, bool success) { return DumpIfFailure(target, 0, 0, success); }



		template<class Form>requires(std::derived_from<Form, RE::TESForm>)
		bool ResolveFormID(RE::FormID& form_id, Form*& form)
		{
			if (!form_id) {//If it's a null form it's looking form, it gets a null form.
				form = nullptr;//Just incase whatever has a default value.
				return true;
			}

			RE::FormID new_id = form_id;

			if (ResolveFormID(new_id, false) == false) {
				form = nullptr;
				return false;
			}

			form = RE::TESForm::LookupByID<Form>(form_id);

			return form != nullptr;

		}

		bool ResolveFormID(RE::FormID& form_id, bool check = true)
		{
			if (!form_id) {
				return true;
			}

			bool result = serialInterface->ResolveFormID(form_id, form_id);

			if (!check || !result)
				return result;

			auto* form = RE::TESForm::LookupByID(form_id);

			if (!form)
				form_id = 0x0;

			return form_id != 0x0;
		}

		constexpr SKSE::SerializationInterface* operator->() noexcept
		{
			return serialInterface;
		}
	};


	struct DefaultSerialize
	{
		template <class SerializeTarget>
		void operator()(SerializeTarget& target, SerialArgument& buffer, bool& success)
		{
			success = buffer.Serialize(target);
		}


		DefaultSerialize() = default;

		//template <class SerializeTarget>
		//DefaultSerialize(SerializeTarget& target, SerialArgument& buffer, bool& success)
		//{
		//	this(target, buffer, success);
		//}
	};



}

