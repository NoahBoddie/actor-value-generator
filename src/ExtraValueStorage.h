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
		
		void UpdateDelay(DataID id, float new_delay)
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

	inline ExtraValueInput GetInputFromCacheIndex(int i)
	{
		switch (i) {
		case 0b0001://1 << 0
			return ExtraValueInput::Base;

		case 0b0010://1 << 1
			return ExtraValueInput::Permanent;

		case 0b0100://1 << 2
			return ExtraValueInput::Damage;

		case 0b1000://Temporary gets dumped.
		default:
			return ExtraValueInput::None;
		}
	}

	struct ExtraValueCacheData
	{
		uint8_t _cacheSetting{};
		union
		{
			uint32_t _cacheInteger{};
			float _cacheFloat;
		};

		triplet<ExtraValueInput> GetOrder()
		{
			triplet<ExtraValueInput> return_triplet{ ExtraValueInput::None, ExtraValueInput::None, ExtraValueInput::None };

			//The order of placement is base, damage, perm startig from the first bit left going right.
			int entry = 0;

			uint8_t placements = _cacheSetting >> 4;
			logger::info("placements {:04B}", placements);
			if (!placements)
				return return_triplet;
			else if (placements == 0xb1111)
				return triplet<ExtraValueInput>{ ExtraValueInput::Base, ExtraValueInput::Permanent, ExtraValueInput::Damage };

			for (int i = 0b0001; i < 0b1000; i <<= 1) {
				if (placements & i) {
					return_triplet[entry] = GetInputFromCacheIndex(i);
					entry++;
				}
			}

			return return_triplet;
		}

		uint8_t GetJumpLength() { return _cacheSetting & 0b1111; }

		DataID GetDataID() { return !_cacheSetting ? 0xFFFFFFFF : _cacheInteger; }
		float GetValue() { return _cacheSetting ? 0 : _cacheFloat; }

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

	struct ExtraValueCache
	{
		//Important note with access to this, the headers are stored in order, so the moment you encounter an entry greater than
		// what you're looking for, you know it's not within.

		std::vector<ExtraValueCacheData> _cacheVector;
		/*
		void PrintCache()
		{
			for (auto& entry)
		}
		//*/
		void MakeCache(std::vector<ExtraValueData>& data_list)
		{
			_cacheVector.clear();

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
			auto header = _cacheVector.begin();

			while (_cacheVector.end() != header) {
				DataID id = header->GetDataID();

				ExtraValueData& data = data_list[id];

				//as it takes, it should remove, to make it simpler to just exit.
				auto triplet_order = header->GetOrder();

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

			_cacheVector.clear();
		}


		//Remake this function with a few different ideas at it's core. 
		// the ability to walk through iterations, the ability to get the next iteration as well.
		// main reason this is needed is in order to be able to insert entries into other entries.
		// Which is mainly how things like mod av will be doing it's work.
		float GetValue(DataID id, ExtraValueInput modifiers = ExtraValueInput::All)
		{
			modifiers = static_cast<ExtraValueInput>(modifiers & ExtraValueInput::ExceptTemporary);

			auto header = _cacheVector.begin();

			while (_cacheVector.end() != header) {
				if (header->GetDataID() == id) {
					auto jump_length = header->GetJumpLength();
					header += jump_length;
					continue;
				}

				float dam = 0;
				float base = 0;
				float perm = 0;

				//as it takes, it should remove, to make it simpler to just exit.
				auto triplet_order = header->GetOrder();

				header++;

				auto order_type = triplet_order.begin();

				while (*order_type != ExtraValueInput::None && modifiers != ExtraValueInput::None) {
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
					modifiers = static_cast<ExtraValueInput>(modifiers ^ *order_type);

					header++;
				}

				return base + perm + dam;
			}

			return 0;
		}
	};

	//Note, the player should never have cache. They are one of the actors treated as "always loaded".
	inline ExtraValueCache playerCache{};
}
