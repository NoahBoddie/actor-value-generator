#pragma once

#include "Serialization/ISerializer.h"

#include "Serialization/SerialArgument.h"
#include "Serialization/SerialConstructor.h"

namespace RGL
{

	namespace old_version
	{
#ifdef old_component
	//This is the old set up for how serial component used to work. If there's ever a use for it, or it's functional structure, I will make it.
	// its a beautiful set up, and one that resolves my ambiguity issues. It's just likely not one I will ever need.

		class IComponentBase;



		struct Empty {};


		template<class ManualData, class AutomaticData>
		class SerialComponent;


		template<class DataType>
		class ISerialData;

		enum class SerialDataFlag
		{
			Manual = 1 << 0,
			Automatic = 1 << 1,
			Paired = Manual | Automatic

		};

		enum AutomaticSerialState : std::uint8_t
		{
			Disabled,
			Enabled,
			Handled
		};


		//Should carry no data, but instead is the thing that the base stores.
		class DataBase
		{


		public:
			friend class IComponentBase;

			//TailSerializer header;
			bool _autoHandled = false;

			virtual void SerializeData(SerialArgument& serializer, bool& success) = 0;
		};

		template <class ManualData, class AutomaticData>
		class SerializableData : DataBase
		{
		private:

			using uSerialData = SerialComponent<ManualData, AutomaticData>;

		public:
			ManualData _manual;
			AutomaticData _auto;

			constexpr ManualData* GetManualData() { return &_manual; }

			constexpr AutomaticData* GetAutomaticData() { return &_auto; }


			constexpr ManualData& GetManualDataRef() { return _manual; }

			constexpr AutomaticData& GetAutomaticDataRef() { return _auto; }


			//void GetManualData(ManualData*& ) { return &_manual; }

			//void GetAutomaticData(AutomaticData*&) { return &_auto; }


			SerializableData()
			{
				auto* parent = GetParent();

				if (!parent)
					return;

				parent->AddDataBase(this);

				//auto* temp_base = dynamic_cast<TempBase*>(the_self);
			}

			uSerialData* GetParent()
			{
				auto offset = offsetof(uSerialData, uSerialData::_data);
				auto address = (std::uintptr_t)this;

				auto ptr = reinterpret_cast<uSerialData*>(address - offset);

				return ptr;
			}

			bool AutoSerialize(SerialArgument& serializer, std::uint32_t intro_version = 0, std::uint32_t incompatible_version = 0, bool deprecated = false)
			{
				if (std::is_same<ManualData, Empty>::value || _autoHandled)
					return;

				auto result = serializer.Serialize(_auto, intro_version, incompatible_version, deprecated);

				_autoHandled = true;

				return result;
			}


			void SerializeData(SerialArgument& serializer, bool& success) override
			{
				success = AutoSerialize(serializer);
				_autoHandled = false;
			}

		};






		//Stores all the actual things we want to serialize. Needs to derive from a serial handler, so it can be used generically.
		class IComponentBase : public ISerializer
		{
		protected:
			template<class Data>
			using DataSet = ISerialData<Data>;

		public:
			//may replace with offsets instead. For something like that, I could hold these mostly statically
			//

			//If I could replace something like this with statics it would be really cool.
			//Replace this with the header note system, it stores easier and is designed for linear calling like this one is.
			std::vector<DataBase*> serialize_list;

			void HandleSerialize(SerialArgument& serializer, bool& success)
			{
				SerializeData(serializer, success);

				//if ()
				for (auto& serializable_data : serialize_list) {
					serializable_data->SerializeData(serializer, success);
				}
			}

			void AddDataBase(DataBase* caller)
			{
				if (!caller)
					return;

				serialize_list.push_back(caller);
			}

			template<class DataType>
			DataType* GetDataPtr()
			{
				if (std::is_same<Empty, DataType>::value == true)
					return nullptr;

				auto* i_data = dynamic_cast<ISerialData<DataType>*>(this);

				if (!i_data)
					return nullptr;

				DataType* stored_data = nullptr;

				//if (i_data->GetData(stored_data) == false)
				//	return nullptr;

				i_data->GetData(stored_data);

				return stored_data;

				//Should get minor or major data later.
				//The purpose of this function is to get what is currently named Temp,
				// which will later be auto/manual data serializable data.

				//For this, it turns itself into a pickle. Greatest code ever written.
				// But in seriousness it turns itself into Temp<Type> and then returns that.
				// This def derives from temp, but no way to know which so I should make a safe and unsafe version.



			}


			virtual void SerializeData(SerialArgument& serializer, bool& success) { RE::DebugMessageBox("unimplemented component function"); }


			//template<class ManualData, class AutomaticData>
			//std::pair<ManualData*, AutomaticData*> GetPairedData(){//auto* serializer = dynamic_cast<Temp<ManualData, AutomaticData>*>(this);}


			//This can be combined for usage.
			//virtual bool IsRelevant() { return true; }


			//While manual serialization DEFINITELY will need it's own function for this sort of thing, auto will need to have this sort of thing too.
			// it should have a function that's used to determine how the values for it's auto serialization is percieved. Given it's nature,
			// it's for the entire piece rather than a portion.
		};


		template<class DataType>
		class ISerialData : public virtual IComponentBase
		{
		protected:
			using Type = DataType;

		public:
			//Look, I know it looks ugly, I know, but it solves the abiguity and allows me to set this from 2 different methods.
			// I tried just having the one, didnt compile, so this is what we're doing now.
			virtual DataType& GetData(DataType** data_set = nullptr) = 0;

			DataType& GetData(DataType*& data_set) { return GetData(&data_set); }
		};

		//Is the actual functionality for serialization
		template<class ManualData = Empty, class AutomaticData = Empty>
		class SerialComponent : public ISerialData<ManualData>, public ISerialData<AutomaticData>//public virtual Base//TempBase
		{
			static_assert(!std::is_same<ManualData, Empty>::value || !std::is_same<AutomaticData, Empty>::value, "At least one data type must be non-empty.");
		protected:
			using ManualSet = ISerialData<ManualData>;
			using AutomaticSet = ISerialData<AutomaticData>;

			template<class Data>
			using DataSet = ISerialData<ManualData>;

		private:
			friend class SerializableData<ManualData, AutomaticData>;

			SerializableData<ManualData, AutomaticData> _data;

		public:
			//Test value;
			//void Detester() {}
			//void SerializeTester() {}
			//{
			//	RE::DebugNotification(std::format("show: {}", value));
			//}

			ManualData& GetData(ManualData** data_set = nullptr) override
			{
				if (data_set)
					*data_set = _data.GetManualData();

				return _data.GetManualDataRef();
			}

			AutomaticData& GetData(AutomaticData** data_set = nullptr) override
			{
				if (data_set)
					*data_set = _data.GetAutomaticData();

				return _data.GetAutomaticDataRef();
			}

			//Simplifies the process of getting data then getting AutoSerialize.
			// Really it just makes Foo::_data.AutoSerialize into Foo::AutoSerialize. Solves an ugly look is all.
			void AutoSerialize(SerialArgument& serializer, std::uint32_t intro = 0, std::uint32_t comp = 0, bool dep = false) { _data.AutoSerialize(serializer, intro, comp, dep); }
		};
#endif
	}

	class IComponentBase;


	template<class DataType>
	class SerialComponent;


	//I'm planning a reinvent. This will take the header note style of doing things, and repurpose it to my own designs.
	// The DataBase will get it's parent, getting the offset between it's data and itself, it won't matter what at all what type it is, the spacing is the same.
	// Then, what you'll do from there is use get the serializableData parent, whom it won't really matter what it truly is, and use the ComponentBase's register function
	//Actually, you could probably just reinterpret cast it into component data by checking the distance between the concept of it and that. Actually, even to that,
	// I can just fucking guess. it's 8 bytes cause of the vtable.
	//After that you're familar with the header note system. The head stores the lowest, while the notes store the object that is above it memory wise.
	// loop until the address is that of the header.
	//This system is preferable because it takes up the least amount of space per head, and costs the least to produce.

	//I think I can cut it down even further. I can make it so the Component base inherits the data base, and the component implements a handle serialize function.

	//^ The above wouldn't work actually, see overriding would work, only one can win in that situation.


	//Should carry no data, but instead is the thing that the base stores.
	class DataBase
	{


	public:
		friend class IComponentBase;

		//TailSerializer header;
		bool _autoHandled = false;

		virtual void SerializeData(SerialArgument& serializer, bool& success) = 0;
	};

	template <class Data>
	class SerializableData : DataBase
	{
	private:
		using SerialComp = SerialComponent<Data>;

	public:
		Data _core;

		constexpr Data* GetData() { return &_core; }

		constexpr Data& GetDataRef() { return _core; }



		SerializableData()
		{
			auto* parent = GetParent();

			if (!parent)
				return;

			parent->AddDataBase(this);

			//auto* temp_base = dynamic_cast<TempBase*>(the_self);
		}

		SerialComp* GetParent()
		{
			auto offset = offsetof(SerialComp, SerialComp::_data);
			auto address = (std::uintptr_t)this;

			auto ptr = reinterpret_cast<SerialComp*>(address - offset);

			return ptr;
		}

		bool AutoSerialize(SerialArgument& serializer, std::uint32_t intro_version = 0, std::uint32_t incompatible_version = 0)
		{
			if (_autoHandled)
				return true;

			logger::info("Auto Serializing {}...", typeid(Data).name());

			auto result = serializer.Serialize(_core, intro_version, incompatible_version);

			_autoHandled = true;

			return result;
		}


		void SerializeData(SerialArgument& serializer, bool& success) override
		{
			success = AutoSerialize(serializer);
			_autoHandled = false;
		}

		Data* operator->() { return &_core; }
		Data& operator*() { return _core; }

	};






	//Stores all the actual things we want to serialize. Needs to derive from a serial handler, so it can be used generically.
	class IComponentBase : public ISerializer
	{
	protected:
		template<class Data>
		using DataSet = SerialComponent<Data>;

	public:
		//may replace with offsets instead. For something like that, I could hold these mostly statically
		//

		//If I could replace something like this with statics it would be really cool.
		//Replace this with the header note system, it stores easier and is designed for linear calling like this one is.
		std::vector<DataBase*> serialize_list;
		
		void HandleSerialize(SerialArgument& serializer, bool& success)
		{
			SerializeData(serializer, success);

			//If the core deserialization fails, its "SUPPOSED" to throw out all data.
			
			for (auto& serializable_data : serialize_list) {
				serializable_data->SerializeData(serializer, success);
			}
		}

		void AddDataBase(DataBase* caller)
		{
			if (!caller)
				return;

			serialize_list.push_back(caller);
		}

		template<class DataType>
		DataType* GetDataPtr()
		{
			//if (std::is_same<Empty, DataType>::value == true)
			//	return nullptr;

			auto* i_data = dynamic_cast<SerialComponent<DataType>*>(this);

			if (!i_data)
				return nullptr;

			DataType* stored_data = nullptr;

			//if (i_data->GetData(stored_data) == false)
			//	return nullptr;
			//Should this need to get address of first?
			i_data->GetData(stored_data);

			return stored_data;

			//Should get minor or major data later.
			//The purpose of this function is to get what is currently named Temp,
			// which will later be auto/manual data serializable data.

			//For this, it turns itself into a pickle. Greatest code ever written.
			// But in seriousness it turns itself into Temp<Type> and then returns that.
			// This def derives from temp, but no way to know which so I should make a safe and unsafe version.



		}

		//template <class T> constexpr T& data = *GetDataPtr<T>();

		//Make a type_trait like value called data that gets the above.

		virtual void SerializeData(SerialArgument& serializer, bool& success) { RE::DebugMessageBox("unimplemented component function"); }


		//template<class ManualData, class AutomaticData>
		//std::pair<ManualData*, AutomaticData*> GetPairedData(){//auto* serializer = dynamic_cast<Temp<ManualData, AutomaticData>*>(this);}


		//This can be combined for usage.
		//virtual bool IsRelevant() { return true; }


		//While manual serialization DEFINITELY will need it's own function for this sort of thing, auto will need to have this sort of thing too.
		// it should have a function that's used to determine how the values for it's auto serialization is percieved. Given it's nature,
		// it's for the entire piece rather than a portion.
	};

	//Wanted to make alt functions, but I see no reason to, or what I would even give.
	//template<class DataType>
	//using DataHandleFunc = void

	template<class DataType>
	class SerialComponent : public virtual IComponentBase
	{
	private:
		friend class SerializableData<DataType>;

		SerializableData<DataType> _data;

	protected:
		DataType& data = _data._core;
		//Simplifies the process of getting data then getting AutoSerialize.
		// Really it just makes Foo::_data.AutoSerialize into Foo::AutoSerialize. Solves an ugly look is all.
		bool AutoSerialize(SerialArgument& serializer, std::uint32_t intro = 0, std::uint32_t comp = 0) { return _data.AutoSerialize(serializer, intro, comp); }
	public:
		
		DataType& GetData(DataType** data_set = nullptr)
		{
			if (data_set)
				*data_set = _data.GetData();

			return _data.GetDataRef();
		}

		inline DataType& GetData(DataType*& data_set = nullptr) { return GetData(&data_set); }


	};




	//For the header note system to work,
	// All I would need is there to be an object within IData that handles that part.
	// This object can be in data rather, servering as the serializable data class, without it's serializable functions.
	//The set up for this seems to exist, but now, it will be more about setting the distances between the last offset the core was pointing to
	// and then setting the head to point to the last entry (serializes back to front.
	class C
	{
		
	};

	template <class Data>
	class NewData
	{


	public:
		using DataType = Data;
		NewData()
		{

		}
	};

	//This set up is confirmed to take up less space, it is ready to go.
	class IComp : public ISerializer
	{
	public:


		virtual void a() {}
	};

	class IData : public virtual IComp
	{
		size_t offset;
		bool _autoHandled = false;

	public:
		virtual void b() {}
	};
	template <class T>
	class Data : public IData///Data as a class doesn't seem to be needed.
	{
	public:
		T data;
		T& data_t = data;
		//D d;
	};

	template <class T>
	class Comp : public Data<T>
	{
		//D d;
	public:
		void b() override {}
	};

	class Derived : public Comp<C>, public Comp<float>, public Comp<int>
	{
		void test()
		{
			Comp<float>::b();
		}
	};

	static_assert(sizeof(Comp<float>) < sizeof(SerialComponent<float>), "EEEEEEEEEE");

	inline Derived new_comp_test;
}