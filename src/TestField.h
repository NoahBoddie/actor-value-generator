#pragma once

namespace TestField
{
	//A name that implies the array can be off and on, something that tells the size of the array externally
	// If I have more need of safety, I can make the first object of the bunch a number that stores its current size.

	//It can be given a class that will give it what size it's allowed to have. It's currently just advised one keeps this number
	// relatively constant, but if I can, I'd like to have some assurance that it's not you know.

	// What
	template<size_t Size>
	class StandardSize
	{
	public: 
		template<class RequestClass>
		static size_t operator()(RequestClass& request) { return Size; }
	};
	
	//Think std::span sums what this effectively does.
	template <class EntryType>
	class TArrayBase
	{
	protected:
		static inline EntryType end;

	
	};

	template<class EntryType, class SizeClass>
	class ToggableArray : TArrayBase<EntryType>
	{
		using Base = TArrayBase<EntryArray>;

	public:
		size_t size() { return SizeClass(*this); }

		bool Create()
		{
			if (_data)
				return false;
			

			size_t r = size();

			if (!r) {
				//Size was 0, terminating code.
				return false;
			}

			_data = new EntryType[size];

			return true;
		}

		inline bool ForceCreate()
		{
			Destroy();
			Create();
		}

		bool Destroy()
		{
			if (!_data)
				return false;
			
			delete[] _data;
		}

		EntryType* _data;

		EntryType& operator[](size_t index)
		{
			if (_data)
				throw nullptr;//maybe like range? No idea.

			size_t r = size();

			if (r <= index)
				throw nullptr;//Out of range likely.
			
			return _data[index];
		}

		bool IsEnabled() { return _data; }
		bool IsDisabled() { return !_data; }

		~ToggableArray() { Destroy(); }
	};
	
	struct ValueData;
	struct CacheData;
	struct BaseData;

	struct NEW_ExtraValueStorage
	{
		//This is the thing that is used to do what we are currently doing, the ownership of instance data. When one stops existing they will
		// be offloaded. 
		//I think how I'll unload this is once when this has been slated for deletion, I will wait for all active effects to be emptied
		// or for the active effect list to be deleted, and once it has then this will create a cache data.
		ValueData* valueStuff;
		
		//If I can, I would really like to have ValueData and CacheData to be the same memory space, and have which is being used by the first 2
		// bits of the pointer.
		//The point of the CacheData is to provide list of values. While I'd like to use this in something like a togglable array,
		// I won't do that. First, because I need to know the size to traverse safely, and second because I need the ability to insert 
		// new entries. It's alright for this to basically own a vector to do it's business.
		CacheData* cacheStuff;

		//Basically, only one of these can exist at any one time.


		//Information about a characters base is stored here for convienant access, if no base changes are made, I'm unsure how
		// I'll conduct changes when someone doesn't want to directly edit the base. But I'll cross the bridge when reached.

		//This is what should be noted. Base is only really used for actors it's specified on, and as such, needs to be handled for the 
		// utmost of look up efficacy, but also so that adding stuff doesn't mess with the defaults, so vectors might be out.
		BaseData* baseStuff;//Whether this would even be needed is subject to question, for host of reasons.
	};



	enum class StoragePriority
	{
		Low,      //Lowest priority, EV Storage will be dumped into a cache. Actor mustn't have an active effect list for this
		Medium,   //Second lowest priority, EV Storage exists but no time functions do. Actor mustn't be 3d loaded for this
		MedHigh,  //Second highest normal priority. Time functions work, but function slower than normal. For this actor is loaded, but not in high
		High,     //Highest normal priority. Time functions work at their maximum possible speed. Actor must be in high for this to function.
		Player,   //The highest priority in total. Player can never lower into different categories.++
		Total = Player
	};

	inline std::map<RE::FormID, ValueData> main_storage;


	union PriorityEntry
	{
		std::pair<const RE::FormID, ValueData*>* storage;
		std::pair<const RE::FormID, CacheData*>* cache;
	};

	struct TEST_Priority
	{
		std::vector<PriorityEntry> priorityMap[StoragePriority::Total];
	};
	
}