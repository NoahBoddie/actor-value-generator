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


	//Make a dynamic extent version of this, where it will be created and managed with a single
	// dynamic size.
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
	
	struct ExtraValueStorage{};
	struct CacheValueStorage{};

	struct ExtraValueContainer
	{
		const StoragePriority priority;
		//Somewhere right here, I want an access counter, the idea would be that if someone keeps accessing a value, once it maxes out
		// it will promote the storage with a marked exception to being unloaded, maybe when dead, but not whe disabled.
		float tickUpdate;

		bool UpdatePriority(RE::Actor* a_this)
		{

		}

		//returns false if it's not allowed to demote, returns true if either aleady demoted or successfully demoted.
		bool DemoteStorage(RE::Actor* a_this) const
		{
			if (priority == StoragePriority::Low)
				return true;
		}
		
		void PromoteStorage(RE::Actor* a_this, const StoragePriority new_priority) const
		{
			//Similar to the above, will not allow the promotion of the following actors
			// Dead
			// Deleted
			// Completely unloaded
			if (priority != StoragePriority::Low)
				return true;


		}


		union
		{
			ExtraValueStorage* value_str = nullptr;
			CacheValueStorage* cache_str = nullptr;
		};

	};


	struct CachedValueNode {};
	struct CachedValueData {};
	
	struct CompressedValueNode {};
	struct CompressedValueData {};

	enum struct StorageType
	{
		Cached = 0b01,
		CompressWaiting = 0b11,
		Compressed = 0b10,
		Player = 0b111,
	};


	std::mutex _temp_ReadWriteLock;

	struct ExtraValueStorage
	{
		StorageType _type = StorageType::Cached;

		//For now, no caching. HOWEVER, when saving, I will be caching exclusively.

		union
		{
			CompressedValueData* _compressData;
			CachedValueData* _compressData;
		};
	};

	struct
	{
		
		//Not the primary storage of actors, but should an actor be loaded, they're in here.
		// so made for ease of look up.
		//This exists not only for ease of access, but also so that cached data can be 
		// cleaned up when it's been around too long.
		std::unordered_map<RE::FormID, ExtraValueStorage*> priority_map;

	};



	//The focus I'm going to go toward, instead of making it so functions are made first,
	// the actual reason why functions are made first will be handled last. 
	//Let me explain. ExtraValueInfos store the function data for a call, this much exists,
	// BUT it's this thing that will leave it's pointer empty, and have some extra data
	// that keeps track of the FullName that's supposed to be used. once everything's loaded,
	// we make connections to all created functions.

	// if a connection fails, we report that a connection could not be made.
	//Associated with every extra data should be an Full name of the Extra Value that
	// spawned it. This is mainly just for error reporting.

	//Also, I like that, extra data.





	


	//Here's a though, functional values are ordinarily high priority, for stuff like get level and such, that makes sense after all. 
	// One can make them lower priority, but this won't affect the data id. Adaptive will always have less priority over functional, who's
	// value ids start the earilest.


	//To this end, I think when making extra value data, we're likely going to want to use a sectioned set of vectors, and push back
	// contents onto each set, then combine them.

	//Here's what I'm gonna do, I'm not gonna care about any of that sort of shit for now, but I'll push functional being a head of adaptive.


	//accounted gaps in data id start from highPriorityFunctional, to HPA. Then,

	enum ExtraValueSection
	{
		Functional, //By default, functional is high priority, but you can assert it being high priority to itself.
		
	};


	enum AliasSettingFlag
	{
		None = 0,

		LastAppliesNonDerived = 1 << 0//This is kinda the only one worth doing right now.

	};

	struct AliasSetting
	{
		ExtraValueInfo* extraValue;
		AliasSettingFlag flag;

		operator bool() { return extraValue != nullptr; }

		AliasSetting(ExtraValueInfo* ev, AliasSettingFlag f) : extraValue(ev), flag(f) {}
	}

	struct PluginAliasNode
	{
		//don't really need that bit do I?
		//std::string plugin_name;

		//uint8_t filledAliases[sizeof(RE::ActorValue)];

		//ExtraValueInfo* avAliasArray[RE::ActorValue::kTotal];
		std::map<RE::ActorValue, AliasSetting> actorValueAliases;
	};

	std::map<std::string, PluginAliasNode> aliasMap;

}