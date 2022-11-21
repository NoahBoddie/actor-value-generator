#pragma once

#include "Types.h"

#include "ExtraValueInfo.h"//Put in cpp please pepeW

namespace AVG
{
	enum class StoragePriority
	{
		Low,		//Lowest priority, EV Storage will be dumped into a cache. Actor mustn't have an active effect list for this
		Medium,		//Second lowest priority, EV Storage exists but no time functions do. Actor mustn't be 3d loaded for this
		MedHigh,	//Second highest normal priority. Time functions work, but function slower than normal. For this actor is loaded, but not in high
		High,		//Highest normal priority. Time functions work at their maximum possible speed. Actor must be in high for this to function.
		Player		//The highest priority in total. Player can never lower into different categories.++ 
	};

	//just a holding cell for right now
	struct time_data
	{
		EVTick tilRecovery, tilUpdate = 0;
	};
	struct ExtraValueData
	{
		//A base value of NaN will dictate that it'd like to use the update function as
		// it's default value, indicating it cannot be changed in base value what so ever.
		//Note, I actually don't think I'll do this, const EVs are supposed to be for things like levels or one's item count
		// with a specific thing.
		// I may make levels to it.
		//^Ignore all of the above

		//Make operators for this like you did for combat value
		//private:

		//Here's an idea, base value starts as an nan value, but gets set
		float _base{0.f};

		union
		{
			struct
			{
				float _prmMod;
				float _tmpMod;
				float _dmgMod;
			};
			//Grouped like this it can stand in for modifiers.
			float _evMods[RE::ACTOR_VALUE_MODIFIER::kTotal]{0.f, 0.f, 0.f};
		};

		float GetValue(ExtraValueInput modifiers = ExtraValueInput::All)
		{
			if (modifiers == ExtraValueInput::None)
				return 0.f;
			//Can do a switch instead maybe.

			float base = modifiers & ExtraValueInput::Base ? _base : 0;
			float perm = modifiers & ExtraValueInput::Permanent ? _prmMod : 0;
			float temp = modifiers & ExtraValueInput::Temporary ? _tmpMod : 0;
			float dmg = modifiers & ExtraValueInput::Damage ? _dmgMod : 0;
			
			return base + temp + perm + dmg;
		}
		//This should likely not use These modifiers btw. Would be easier on me.
		void SetValue(float value, ExtraValueInput modifier = ExtraValueInput::Base)
		{
			switch (modifier)
			{
			case ExtraValueInput::Base:
				_base = value;
				break;

			case ExtraValueInput::Permanent:
				_prmMod = value;
				break;

			case ExtraValueInput::Temporary:
				_tmpMod = value;
				break;

			case ExtraValueInput::Damage:
				_dmgMod = value;
				break;
			default:
				logger::error("Modifier {} id isn't valid", modifier);
			}
		}

		void ModValue(float value, ExtraValueInput modifier = ExtraValueInput::Base)
		{
			switch (modifier) {
			case ExtraValueInput::Base:
				_base += value;
				break;

			case ExtraValueInput::Permanent:
				_prmMod += value;
				break;

			case ExtraValueInput::Temporary:
				_tmpMod += value;
				break;

			case ExtraValueInput::Damage:
				//This would need a setting to allow overcharge maybe?
				_dmgMod = fmin(_dmgMod + value, 0);
				//_dmgMod += value;
				//logger::info("modifying damage by {} to {}", value, GetValue());
			}
		}

		bool operator==(ExtraValueData& a_rhs) 
		{ 
			return a_rhs._base == _base &&
			       a_rhs._prmMod == _prmMod &&
			       a_rhs._tmpMod == _tmpMod &&
			       a_rhs._dmgMod == _dmgMod;
		}

	};


	class ExtraValueStorage
	{
	//static
	private:
		inline static std::map<RE::FormID, ExtraValueStorage*> _valueTable{};

	public:
		
		static ExtraValueStorage* GetStorage(RE::Actor* actor)
		{
			//This won't be used often, but it's used in situations where it would be ok to not create storage yet.
			if (!actor) {
				return nullptr;
			}

			auto result = _valueTable.find(actor->formID);

			return result == _valueTable.end() ? nullptr : result->second;
		}

		
		static ExtraValueStorage& GetCreateStorage(RE::Actor* actor)
		{
			//Should this be thead locked? I feel like this should be thread locked

			if (!actor) {
				logger::critical("No actor detected, terminating storage search.");
				throw nullptr;  //Just crash, this isn't supposed to be found.
			}
			
			ExtraValueStorage*& storage_spot = _valueTable[actor->formID];

			if (storage_spot) {
				//logger::warn("Stor loc {}", (uintptr_t)storage_spot);
				return *storage_spot;
			}


			ExtraValueStorage* new_storage = new ExtraValueStorage(actor, false);

			storage_spot = new_storage;

			return *new_storage;
		}


	//membered
		//This should delete it's copy assignment and intializers.
	public:
		std::vector<ExtraValueData> _valueData;

		//This serves as both a map and manifest. And while, yes, this is a vector, ideally the data is concurent, never to be deleted
		// until it no longer exists.
		std::vector<std::pair<DataID, float>> _recoveryData; 
		
		float tickValue = 0;


		ExtraValueStorage(RE::Actor* actor, bool is_deserializing);

	public:
		float* GetExtraValueDelay(DataID id)
		{//switch to binary search if it's sorted
			auto result = std::find_if(_recoveryData.begin(), _recoveryData.end(), [=](auto it) { return it.first == id; });

			if (_recoveryData.end() != result) {
				return &result->second;
			}

			return nullptr;
		}
		
		void SetDelay(DataID id, float new_delay)
		{//Sets the delay forcibly, if a presence for it exists.
			float* curr_delay = GetExtraValueDelay(id);

			if (!curr_delay)
				return;

			if (new_delay > *curr_delay)
				*curr_delay = new_delay; 
		}
		
		void UpdateDelay(DataID id, ExtraValueInfo* info)
		{//Sets the delay based on info.
			if (!info)
				return;

			RecoveryData* rec_data = info->GetRecoveryData();
	
			if (!rec_data)
				return;

			float* curr_delay = GetExtraValueDelay(id);

			if (!curr_delay)
				return;

			float new_delay = rec_data->tmp_recDelay;

			if (new_delay > *curr_delay) {
				*curr_delay = new_delay;
				logger::info("new delay");
			}
		}


		//Make versions of these that accept the other modifier set up too.
		float GetValue(DataID id, ExtraValueInput modifiers = ExtraValueInput::All, ExtraValueInfo* info = nullptr)
		{
			//Here, if info is loaded an update will be performed if it's on constant update.
			return _valueData[id].GetValue(modifiers); 
		}

		void SetValue(DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr) 
		{ 
			_valueData[id].SetValue(value, modifier); 
		}

		void ModValue(DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr) 
		{
			//If used with info, it will attempt to update the recovery data.
			//No info should actually mean something, meaning don't run needed stuff.
			if (info && value < 0)
				UpdateDelay(id, info); 
			
			
			_valueData[id].ModValue(value, modifier); 
		}

	};


	
	template <class Type>
	using triplet = std::array<Type, 3>;

	
	template <class Type>
	using quad = std::array<Type, 4>;

	constexpr ExtraValueInput GetInputFromCacheIndex(int i)
	{
		switch (i) {
		case 0b0001://1 << 0 OR 1
			return ExtraValueInput::Base;

		case 0b0010://1 << 1 OR 2
			return ExtraValueInput::Permanent;

		case 0b0100://1 << 2 OR 4
			return ExtraValueInput::Damage;

		case 0b1000://1 << 3 OR 8
			//While temporary (as the name suggests) isn't stored, this value is used to denote the stored recovery value.
			return ExtraValueInput::Temporary;
		default:
			return ExtraValueInput::None;
		}
	}

	constexpr int GetCacheIndexFromInput(ExtraValueInput i)
	{
		switch (i) {
		case ExtraValueInput::Base://1 << 0 OR 1
			return 0b0001;

		case ExtraValueInput::Permanent://1 << 1 OR 2
			return 0b0010;

		case ExtraValueInput::Damage://1 << 2 OR 4
			return 0b0100;

		case ExtraValueInput::Temporary://1 << 3 OR 8
			//While temporary (as the name suggests) isn't stored, this value is used to denote the stored recovery value.
			return 0b1000;
		default:
			return 0b0000;
		}
	}


	#pragma pack(1)
	struct ExtraValueCacheData
	{
		uint8_t _cacheSetting{};
		union
		{
			uint32_t _cacheInteger{};
			float _cacheFloat;
		};

		triplet<ExtraValueInput> GetOrder(bool raw)
		{
			//Raw will mind the spaces between things. Unsure of where I'll do it, but I anticipate it.

			triplet<ExtraValueInput> return_triplet{ ExtraValueInput::None, ExtraValueInput::None, ExtraValueInput::None };

			//The order of placement is base, damage, perm startig from the first bit left going right.
			int entry = 0;

			uint8_t placements = _cacheSetting >> 4;
			logger::info("placements {:04B}", placements);
			if (!placements)
				return return_triplet;
			else if (placements == 0xb1111 || placements == 0xb0111)
				return triplet<ExtraValueInput>{ ExtraValueInput::Damage, ExtraValueInput::Permanent, ExtraValueInput::Base };
			//Thinking about it, I could just make switch statements for the combinations
			constexpr int max_index = 0b0100;

			for (int i = max_index; i > 0; i >>= 1) {
				if (placements & i) {
					return_triplet[entry] = GetInputFromCacheIndex(i);
					
					if (!raw)
						entry++;
				}

				if (raw)
					entry++;
			}

			return return_triplet;
		}
		//Notice, jump length even with later expansion will only ever need 3 bits, so the last is allowed to have a flag for a value.
		// The question is what...

		constexpr uint8_t GetJumpLength() { return _cacheSetting & 0b1111; }//I fear jump length may need to increase by one.

		constexpr uint8_t GetPlacement() { return _cacheSetting >> 4; }

		DataID GetDataID() { return !_cacheSetting ? ExtraValueInfo::FunctionalID : _cacheInteger; }
		float GetValue() { return _cacheSetting ? 0 : _cacheFloat; }//Use nan

		bool HasValue(ExtraValueInput input) { int i = GetCacheIndexFromInput(input); return i | GetPlacement(); }
		
		int GetPlacementsBeforeInput(ExtraValueInput input)
		{
			//Should use negative to convey error

			int i = GetCacheIndexFromInput(input) << 1;
			2;
			0b10;

			if (i == 0)
				return -1;

			int placement = GetPlacement();

			int count = 0;

			while (i <= 0b1000) 
			{
				if (i & placement)
					count++;

				i <<= 1;
			}

			logger::info("new insert length: {}", count);

			return count;

		}


		
		void SetID(DataID id)
		{
			if (!_cacheSetting)
				return;//This isn't a setting, do not edit.
		}

		//The value mods should probably just take a modifier enum
		float SetValue(float value, bool is_damage)
		{
			if (_cacheSetting)
				return NAN;  //This is a setting, do not edit.

			return _cacheFloat = is_damage ? fmin(0, value) : value;
		}
		
		
		float ModValue(float value, bool is_damage) 
		{ 
			value += _cacheFloat; 
			return SetValue(value, is_damage); 
		}
		
		//Uses bitwise OR/XOR to add/remove placements, if no more places remain, returns false so it can be removed.
		bool ModPlacement(uint8_t placements, bool add)
		{
			if (!_cacheSetting)
				return true;  //This isn't a setting, do not edit. Must return true for obvious reasons.

			//logger::info("placements {:04B}", placements);

			//placements &= 0b11110000;
			//Check if the first 4 bits are clear, ie if the placement was already adjusted. Not important though
			placements <<= 4;

			auto old = _cacheSetting;

			if (add)
				_cacheSetting |= placements;
			else
				_cacheSetting ^= placements;

			if (!add && GetPlacement() == 0) {
				return false;
			}

			if (_cacheSetting != old)
				UpdateJumpLength();

			return true;
		}

		void UpdateJumpLength()
		{
			if (!_cacheSetting)
				return;  //This isn't a setting, do not edit.

			auto placements = GetPlacement();
			auto n = placements;

			int count = 0;

			while (n)
			{
				count += n & 1;
				n >>= 1;
			}

			_cacheSetting = (placements << 4) | count;

			logger::info("new count {}", count);
		}


		//Create ways to adjust the flags, which should re do the jump length.
		// Also, something to redo the id
		ExtraValueCacheData() = default;
		ExtraValueCacheData(uint8_t positions, uint8_t jump_length, DataID id)
		{
			_cacheSetting = (positions << 4) | jump_length;
			_cacheInteger = id;
		}

		ExtraValueCacheData(float value)
		{
			_cacheFloat = value;
		}
	};
	static_assert(sizeof(ExtraValueCacheData) == 0x5);

	struct ExtraValueCache
	{
		//Important note with access to this, the headers are stored in order, so the moment you encounter an entry greater than
		// what you're looking for, you know it's not within.

		//Some more, notes, more than anything THIS needs a read and write lock because of the constant use of iterators.


		//To expand on this, this actually has 2 headers. 1 is a time float, used to count ticks until it will update recovery, the 
		// other is to assist simple checking for "I don't currently contain this value" or I can ignore half of my stored
		// values.
		std::vector<ExtraValueCacheData> _cacheVector{ ExtraValueCacheData() };


		//NOT TO BE SERIALIZED but I think it would be smart to have a list of all the places that need updating for recovery.


		/*
		void PrintCache()
		{
			for (auto& entry)
		}
		//*/
		void MakeCache(std::vector<ExtraValueData>& data_list)
		{
			//_cacheVector.clear();

			//_cacheVector.push_back(ExtraValueCacheData());

			ExtraValueData empty{};

			//There's a psuedo constant for this.
			for (int i = 0; i < data_list.size(); i++) {
				ExtraValueData& entry = data_list[i];

				if (entry == empty)
					continue;

				logger::info("DataID {} processing...", i);

				uint8_t pos_bits = 0;
				uint8_t size_bits = 0;

				triplet<float> cache_entries{ 0, 0, 0 };

				for (int j = 0b0001; j < 0b1000; j <<= 1) 
				{
					//if (j == 0b1000)
					//	continue;

					ExtraValueInput input = GetInputFromCacheIndex(j);

					float value = entry.GetValue(input);

					if (input == ExtraValueInput::Damage) 
					{
						logger::warn("FFFFFFFFFFFFFFFFFF {}", value);
					}
					//Or the more proper way, did change flags get set? Data can be larger, as it's instance data.
					//Also this ignores if the value in question started at zero
					if (value) {
						logger::info("value for 0b{:04B}(or {:04B}) is {}", input, j, value);
						cache_entries[size_bits] = value;
						pos_bits |= j;
						size_bits++;
					}
				}

				_cacheVector.push_back(ExtraValueCacheData(pos_bits, size_bits, i));

				for (int j = 0; j < size_bits; j++) {
					_cacheVector.push_back(ExtraValueCacheData(cache_entries[j]));
				}
			}
		}

		
		void DumpCache(std::vector<ExtraValueData>& data_list)
		{

			auto header = _cacheVector.begin() + 1;

			while (_cacheVector.end() != header) {
				DataID id = header->GetDataID();

				if (id == ExtraValueInfo::FunctionalID) {
					logger::info("trigger(in cache dump) {}", header->GetValue());
					header++;
					continue;
				}
				ExtraValueData& data = data_list[id];

				//as it takes, it should remove, to make it simpler to just exit.
				auto triplet_order = header->GetOrder(false);

				header++;

				auto order_type = triplet_order.begin();
				//Improve this please.
				while (_cacheVector.end() != header && order_type != triplet_order.end() && *order_type != ExtraValueInput::None) {
					switch (*order_type) {
					case ExtraValueInput::Base:
						logger::info("Cached value for ID{}::Base is {}", id, header->GetValue());
						data.SetValue(header->GetValue(), ExtraValueInput::Base);
						break;

					case ExtraValueInput::Permanent:
						logger::info("Cached value for ID{}::Permanent is {}", id, header->GetValue());
						data.SetValue(header->GetValue(), ExtraValueInput::Permanent);
						break;

					case ExtraValueInput::Damage:
						logger::info("Cached value for ID{}::Damage is {}", id, header->GetValue());
						data.SetValue(header->GetValue(), ExtraValueInput::Damage);
						break;
					}
					order_type++;
					header++;
				}
			}

			//_cacheVector.clear();
			//_cacheVector.push_back(ExtraValueCacheData());
		}


		void DumpCache(ExtraValueStorage* storage)
		{
			std::vector<ExtraValueData>& data_list = storage->_valueData;
			std::vector<std::pair<DataID, float>>& rec_list = storage->_recoveryData;

			auto header = _cacheVector.begin();

			while (_cacheVector.end() != header) {
				DataID id = header->GetDataID();

				ExtraValueData& data = data_list[id];

				//as it takes, it should remove, to make it simpler to just exit.
				auto triplet_order = header->GetOrder(false);

				header++;

				auto order_type = triplet_order.begin();
				//Improve this please.
				while (_cacheVector.end() != header && order_type != triplet_order.end() && *order_type != ExtraValueInput::None) {
					switch (*order_type) {
					case ExtraValueInput::Base:
						logger::info("Cached value for ID{}::Base is {}", id, header->GetValue());
						data.SetValue(header->GetValue(), ExtraValueInput::Base);
						break;

					case ExtraValueInput::Permanent:
						logger::info("Cached value for ID{}::Permanent is {}", id, header->GetValue());
						data.SetValue(header->GetValue(), ExtraValueInput::Permanent);
						break;

					case ExtraValueInput::Damage:
						logger::info("Cached value for ID{}::Damage is {}", id, header->GetValue());
						data.SetValue(header->GetValue(), ExtraValueInput::Damage);
						break;

					case ExtraValueInput::Temporary://Note, this is actually recovery. Easier this way though.
						logger::info("Cached value for ID{}::Recovery is {}", id, header->GetValue());
						storage->SetDelay(id, header->GetValue());
						break;
					}
					order_type++;
					header++;
				}
			}

			_cacheVector.clear();
		}

		using CacheIterator = decltype(_cacheVector)::iterator;

		CacheIterator GetHeaderFromID(DataID id) 
		{ 
			CacheIterator it = _cacheVector.begin();
			CacheIterator end = _cacheVector.end();
			//It's here where ordinarily we'd want to check the midways and the maximum of this value.
			// For now, I will not be doing that however. But I will later, because it makes it easier to traverse.
			
			it++;
			
			while (end != it) {
				auto prev_id = it->GetDataID();

				if (prev_id == ExtraValueInfo::FunctionalID) {
					//This isn't being hit off of anything erroneous, its just where the jump length goes. do + 1.
					logger::info("Trigger, {}", it->GetValue());

					it++;
					continue;
				}
				if (prev_id == id) {
					return it;
				}

				auto jump_length = it->GetJumpLength() + 1;
				it += jump_length;
			}

			return _cacheVector.end(); 
		}

		
		CacheIterator GetHeaderClosestFromID(DataID id)
		{
			CacheIterator it = _cacheVector.begin();
			CacheIterator end = _cacheVector.end();
			//It's here where ordinarily we'd want to check the midways and the maximum of this value.
			// For now, I will not be doing that however. But I will later, because it makes it easier to traverse.
			//static_assert(false, "remember, get closest header from id doesn't currently work as intended.");
			it++;

			//logger::info("AAAAAAAAAAAAAA");

			auto closest_id = it->GetDataID();
			auto closest_it = it;
			while (end != it) {
				auto prev_id = it->GetDataID();
				
				if (prev_id == ExtraValueInfo::FunctionalID) {
					//This isn't being hit off of anything erroneous, its just where the jump length goes. do + 1.
					logger::info("Trigger, {}", it->GetValue());
					
					it++;
					continue;
				}
				else if (prev_id == id) {
					return it;
				} else if (prev_id > closest_id && prev_id < id) {
					closest_id = prev_id;
					closest_it = it;
				}
				auto jump_length = it->GetJumpLength() + 1;
				it += jump_length;
			}


			//logger::info("EEEEEEEEE");
			return closest_it;
		}


		CacheIterator GetIteratorFromIndex(int i) { CacheIterator it = _cacheVector.begin(); return i <= 0 ? it : it + i; }


		//It will return functional if it's reached the end or is the end.
		// The way it's to be used is submit begin (the information header) and it will iterate from there.
		// I would optionally like this to designate an end, so I can have one for traversal.
		DataID GetNextHeader(CacheIterator& it, CacheIterator end)
		{
			//Kinda feel like it should I should do this manually because of the jump length stuff.
			int inc = _cacheVector.begin() == it ? 1 : it->GetJumpLength();

			

			it += inc;//We want to increment past the current header, 1 for the primary, variable for value headers

			auto eval_header = [=](ExtraValueCacheData data) { return data.GetDataID() != ExtraValueInfo::FunctionalID; };


			auto result = std::find_if(it, end, eval_header);

			return result != end ? result->GetDataID() : ExtraValueInfo::FunctionalID;
		}
		
		/* 
		//I 'm feeling lazy, I' m just gonna make it with one, since that 's the time you' d make something like this.
		//If you need to create the header too, not just edit it, this is the thing you'd use.
		std::vector<ExtraValueCacheData> InsertHeaderIterator(DataID id, triplet<float> value_triplet)
		{
			const int valid_items = std::count_if(value_triplet.cbegin(), value_triplet.cend(), [](auto value) { return std::isnan(value); }) - 3;

			//This returns the values that you should insert
			std::vector<ExtraValueCacheData> return_vector = std::vector<ExtraValueCacheData>(valid_items + 1);

			int entry = 1;

			//Maybe use transform
			for (int i = 0; i <= valid_items; i++) 
			{
				return_vector[i] = ExtraValueCache()
			}
		}
		//*/


		
		bool RemoveValueIterator(DataID id, ExtraValueInput modifier)
		{
			//Doesn't need to return success, but it will just incase.
			CacheIterator header_it = GetHeaderClosestFromID(id);

			
			//Create
			if (_cacheVector.end() == header_it) {
				//No process required, it's the end.
				return false;
			}

			auto position = GetCacheIndexFromInput(modifier);

			if (!position) {
				//Still not sure what to put here.
				logger::error("A error");
				throw nullptr;
			}


			logger::info("test A");

			CacheIterator value_it = header_it;

			//Get
			if (GetIterator(value_it, modifier) == false) {
				//Modifier not found
				return false;
			}

			logger::info("test B");

			if (header_it->ModPlacement(position, false) == false)
				_cacheVector.erase(header_it, value_it);
			else
				_cacheVector.erase(value_it);
			
			logger::info("test C");
			
			return true;
		}

		bool RemoveValueIteratorFromIterator(CacheIterator header_it, CacheIterator value_it, ExtraValueInput modifier)
		{
			//For now, I'm just assuming each on is as they say they are. Excepting end checks

			if (_cacheVector.end() == header_it || _cacheVector.end() == header_it)
				return false;


			auto position = GetCacheIndexFromInput(modifier);

			logger::info("test A");

			if (header_it->ModPlacement(position, false) == false)
				_cacheVector.erase(header_it, value_it);
			else
				_cacheVector.erase(value_it);

			logger::info("test B");

			return true;
		}


		bool InsertValueIterator(DataID id, ExtraValueInput modifier, float value, CacheIterator& it_value, CacheIterator& it_header)
		{
			//Returns true if inserted, false if an iterator is going to be sent back
			// as an iterator didn't need to be created

			CacheIterator it = GetHeaderClosestFromID(id);
			
			auto position = GetCacheIndexFromInput(modifier);

			if (!position) {
				//I'm not sure what to throw or do in this instance.
				logger::error("A error");
				throw nullptr;
			}

			//Note, should only be getting a modifier, and I will treat it as such.

			//From here you should get order


			//Create
			if (_cacheVector.end() == it) {
				//No entries found, Insert at end, move on.
				_cacheVector.insert(_cacheVector.end(), { ExtraValueCacheData(position, 1, id), ExtraValueCacheData(value) });
				return true;
			}
			//Create
			if (it->GetDataID() != id) {
				//Specific entry not found, inserting entry.
				it += it->GetJumpLength();
				_cacheVector.insert(it + 1, { ExtraValueCacheData(position, 1, id), ExtraValueCacheData(value) });
				return true;
			}

			logger::info("test A");

			//We set the header now that we know that it's valid
			it_header = it;

			//Get
			if (GetIterator(it, modifier) == true) {
				it_value = it;
				return false;
			}
			logger::info("test B");

			int add = it->GetPlacementsBeforeInput(modifier);

			logger::info("test C");
			if (add < 0) {
				logger::error("B error");
				throw nullptr;//Again, no idea how to error handle this.
			}

			//Create
			it->ModPlacement(position, true);
			
			logger::info("test D");

			_cacheVector.insert(it + add + 1, ExtraValueCacheData(value));
				
			return true;
		}

		bool InsertValueIterator(DataID id, ExtraValueInput modifiers, float value)
		{
			CacheIterator it = _cacheVector.begin();//Don't really need this but you know.
			
			return InsertValueIterator(id, modifiers, value, it, it);
		}

		//Make a version of the above that uses std::memcpy on a location to copy a set of objects. Will be used for deserial.


		//Remake this function with a few different ideas at it's core. 
		// the ability to walk through iterations, the ability to get the next iteration as well.
		// main reason this is needed is in order to be able to insert entries into other entries.
		// Which is mainly how things like mod av will be doing it's work.
		float GetValue(DataID id, ExtraValueInput modifiers = ExtraValueInput::All)
		{
			//Not using here yet, but check below for an explanation.
			//if (modifiers & ExtraValueInput::Temporary)
			//	modifiers = ExtraValueInput::Temporary;


			//Note, this will likely need to send 2 values. A float and a bool to say if base value ever existed, and that if not
			// it gets the update value, or the default value.

			modifiers = static_cast<ExtraValueInput>(modifiers & ExtraValueInput::ExceptTemporary);

			auto header = _cacheVector.begin();

			while (_cacheVector.end() != header) {
				if (header->GetDataID() != id) {
					auto jump_length = header->GetJumpLength();
					header += jump_length;
					continue;
				}

				float dam = 0;//NAN;
				float base = 0;//NAN;
				float perm = 0;//NAN;

				//as it takes, it should remove, to make it simpler to just exit.
				auto triplet_order = header->GetOrder(false);

				header++;

				auto order_type = triplet_order.begin();

				while (header != _cacheVector.end() && *order_type != ExtraValueInput::None && modifiers != ExtraValueInput::None) {
					switch (*order_type) {
					case ExtraValueInput::Base:
						base = header->GetValue();
						break;

					case ExtraValueInput::Permanent:
						perm = header->GetValue();
						break;

					case ExtraValueInput::Damage:
						dam = header->GetValue();
						break;
					}
					//Watch this, I've had bad experences with XOR
					modifiers = static_cast<ExtraValueInput>(modifiers ^ *order_type);//I think this is to guard check?
					order_type++;//I think both of these have to move for it to work? Could be wrong.
					header++;
				}

				//I'm doing this so this can report back an error when accessing, so that it can get the default values. Might not work.
				//if ( !isnan(dam) || !isnan(base) || !isnan(perm) ) 
				//{dam = isnan(dam) ? 0 : dam;}

				return base + perm + dam;
			}

			return 0;
		}

		//I can improve this but I'm honestly too fucking lazy right now. Sadge
		float GetValueIterator(CacheIterator it, ExtraValueInput modifiers = ExtraValueInput::All)
		{
			//Instead, if it possesses temporary, which is recovery delta, it will ONLY do that, 
			// since that cannot actually be grouped with the rest and never should be. That way it reads easy.
			if (modifiers & ExtraValueInput::Temporary)
				modifiers = ExtraValueInput::Temporary;

			float dam = 0;
			float base = 0;
			float perm = 0;

			//as it takes, it should remove, to make it simpler to just exit.
			auto triplet_order = it->GetOrder(false);

			it++;

			auto order_type = triplet_order.begin();

			while (it != _cacheVector.end() && *order_type != ExtraValueInput::None && modifiers != ExtraValueInput::None) {
				switch (*order_type) {
				case ExtraValueInput::Base:
					base = it->GetValue();
					break;

				case ExtraValueInput::Permanent:
					perm = it->GetValue();
					break;

				case ExtraValueInput::Damage:
					dam = it->GetValue();
					break;
				}
				//Watch this, I've had bad experences with XOR
				modifiers = static_cast<ExtraValueInput>(modifiers ^ *order_type);
				order_type++;
				it++;
			}

			return base + perm + dam;

		}
		

		
		//I can improve this but I'm honestly too fucking lazy right now. Sadge
		bool GetIterator(CacheIterator& iterator, ExtraValueInput modifier = ExtraValueInput::All)
		{//Real name, GetIteratorFromIterator


			//returns if the value exists or not. If true it will be changed to that value.

			//as it takes, it should remove, to make it simpler to just exit.
			auto it = iterator;
			
			auto triplet_order = it->GetOrder(false);

			if (it->HasValue(modifier) == false)
				return false;

			it++;

			auto order_type = triplet_order.begin();

			while (it != _cacheVector.end() && *order_type != modifier) { it++; order_type++; }
			
			//Iterator should be functional value, this searches for values not headers.
			bool result = it != _cacheVector.end();// && it->GetDataID() != ExtraValueInfo::FunctionalID;

			if (result)
				iterator = it;

			return result;
		}
		



		//Mod and set are a bit of odd ducklings. They will need to insert and edit headers, as well as the cache header.
		// Important to note, that the value of get shouldn't work if there's a constant update on the value, it will
		// just give the update function.
		void SetValue(DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr)
		{
			bool updates = info && info->GetUpdateData();

			//the lack of update data is a temp thing
			if (value == 0 && (modifier != ExtraValueInput::Base || !updates)) {
				//Remove, do not insert.
				RemoveValueIterator(id, modifier);
				return;
			}

			CacheIterator value_it{};
			CacheIterator header_it{};

			if (InsertValueIterator(id, modifier, value, value_it, header_it) == true)
				return;//Value was inserted, no need to go on from here.

			value_it->SetValue(value, modifier == ExtraValueInput::Damage);		
		}

		void ModValue(DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr)
		{
			CacheIterator value_it{};
			CacheIterator header_it{};
			if (InsertValueIterator(id, modifier, value, value_it, header_it) == true)
				return;  //Value was inserted, no need to go on from here.


			
			bool updates = info && info->GetUpdateData();

			//Since base can be a value other than zero at default, this won't apply here.
			if (value_it->ModValue(value, modifier == ExtraValueInput::Damage) == 0 && (modifier != ExtraValueInput::Base || !updates)) {
				//it's time to destroy.
				RemoveValueIteratorFromIterator(header_it, value_it, modifier);
			}
		}

		//FOR THE ABOVE MAKE SURE TO EXCLUDE TEMP FROM REMOVAL AS WELL, IE RECOVERY RATE TIME.
	};

	//Note, the player should never have cache. They are one of the actors treated as "always loaded".
	inline ExtraValueCache playerCache{};
}
