#pragma once

#include "Types.h"

#include "ExtraValueInfo.h"//Put in cpp please pepeW

namespace AVG
{
	namespace Legacy {
		struct  Handler;
	}
	enum class StoragePriority
	{
		Low,		//Lowest priority, EV Storage will be dumped into a cache. Actor mustn't have an active effect list for this
		Medium,		//Second lowest priority, EV Storage exists but no time functions do. Actor mustn't be 3d loaded for this
		MedHigh,	//Second highest normal priority. Time functions work, but function slower than normal. For this actor is loaded, but not in high
		High,		//Highest normal priority. Time functions work at their maximum possible speed. Actor must be in high for this to function.
		Player		//The highest priority in total. Player can never lower into different categories.++ 
	};

	//MOVE ME
	template <std::derived_from<RE::TESForm> Object, class... Args>
	static bool SendEvent(RE::BSFixedString event_name, Object* object, Args... args)
	{
		//I will move this to an event manager, a seperate managing object that carelessly stores information of when
		// events go off, and how long ago. Basically, all forms would have "an event object, that adheres to some global settings.
		// I can then create a class that helps specific events by storing extra data for it, independent of the storage, for those
		// it would never interact upon.

		//A neat interaction though is if the storage had a pointer to where that information is.

		auto event_args = RE::MakeFunctionArguments((Args)args...);

		auto vm = RE::SkyrimVM::GetSingleton();

		RE::VMTypeID form_Type = static_cast<RE::VMTypeID>(object->GetFormType());

		const auto handle = vm->handlePolicy.GetHandleForObject(static_cast<RE::VMTypeID>(form_Type), object);

		if (handle){// && vm->handlePolicy.EmptyHandle() != handle) {
			vm->SendAndRelayEvent(handle, &event_name, event_args, nullptr);
		}
		else {
			logger::debug("{} has no handle.", object->GetName());
		}
		return handle;
	}

	constexpr float tickTime = 0.25f;
	
	
	template<class T>
	using duo = std::pair<T, T>;

	struct ExtraValueData
	{
		inline static std::mutex av_lock;

		//A base value of NaN will dictate that it'd like to use the update function as
		// it's default value, indicating it cannot be changed in base value what so ever.
		//Note, I actually don't think I'll do this, const EVs are supposed to be for things like levels or one's item count
		// with a specific thing.
		// I may make levels to it.
		//^Ignore all of the above

		//Make operators for this like you did for combat value
		//private:

		//Here's an idea, base value starts as an nan value, but gets set
		float _base{ NAN };

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


		duo<float> GetValue(ExtraValueInput modifiers = ExtraValueInput::All)
		{
			std::lock_guard<std::mutex> behaviour_guard(av_lock);

			if (modifiers == ExtraValueInput::None)
				return {};
			//Can do a switch instead maybe.

			float base = modifiers & ExtraValueInput::Base ? _base : 0;
			float perm = modifiers & ExtraValueInput::Permanent ? _prmMod : 0;
			float temp = modifiers & ExtraValueInput::Temporary ? _tmpMod : 0;
			float dmg = modifiers & ExtraValueInput::Damage ? _dmgMod : 0;
			
			auto result = duo<float>(base, temp + perm + dmg);

			//logger::debug("{}/{}", result.first, result.second);
			return result;
		}

		float GetValueUnsafe(ExtraValueInput modifiers = ExtraValueInput::All)
		{
			//These are completely reversed you idiota
			auto result = GetValue(modifiers);

			if (isnan(result.first) == true)
				return result.second;

			return result.first + result.second;
		}


		//This should likely not use These modifiers btw. Would be easier on me.
		duo<float> SetValue(float value, ExtraValueInput modifier = ExtraValueInput::Base)
		{
			std::lock_guard<std::mutex> behaviour_guard(av_lock);
			float old = 0;
			switch (modifier)
			{
			case ExtraValueInput::Base:
				old = _base;
				_base = value;

				break;

			case ExtraValueInput::Permanent:
				old = _prmMod;
				_prmMod = value;
				break;

			case ExtraValueInput::Temporary:
				old = _tmpMod;
				_tmpMod = value;
				break;

			case ExtraValueInput::Damage:
				old = _dmgMod;
				_dmgMod = value;
				break;
			default:
				logger::error("Modifier {} id isn't valid", modifier);
			}

			return { old, value };
		}

		duo<float> ModValue(float value, ExtraValueInput modifier = ExtraValueInput::Base)
		{
			std::lock_guard<std::mutex> behaviour_guard(av_lock);
			float before;
			float after;

			switch (modifier) {
			case ExtraValueInput::Base:
				before = _base;
				after = _base += value;
				break;

			case ExtraValueInput::Permanent:
				before = _prmMod;
				after = _prmMod += value;
				break;

			case ExtraValueInput::Temporary:
				before = _tmpMod;
				after = _tmpMod += value;
				break;

			case ExtraValueInput::Damage:
				before = _dmgMod;
				after = _dmgMod = fmin(_dmgMod + value, 0);
				break;
			default:
				logger::error("Modifier {} id isn't valid", magic_enum::enum_name(modifier));
				return {};

			}
			
			return { before, after };
			
		}

		bool operator==(ExtraValueData& a_rhs) 
		{ 
			return a_rhs._base == _base &&
			       a_rhs._prmMod == _prmMod &&
			       a_rhs._tmpMod == _tmpMod &&
			       a_rhs._dmgMod == _dmgMod;
		}

	};
	
	struct PlayerStorage;

	class ExtraValueStorage
	{
		friend Legacy::Handler;

	private:
		using Mutex = std::shared_mutex;
		using ReadLock = std::shared_lock<Mutex>;
		using WriteLock = std::unique_lock<Mutex>;


		//using Mutex = std::mutex;
		//using ReadLock = std::lock_guard<Mutex>;
		//using WriteLock = std::lock_guard<Mutex>;


		inline static Mutex accessLock{};



	public:

		struct StorageView
		{
			StorageView() = default;
			StorageView(ExtraValueStorage* ptr) : storage{ ptr } {}
			StorageView(ExtraValueStorage& ref) : StorageView{ &ref } {}

			StorageView(Mutex& mtx, bool readOnly) 
			{
				if (readOnly)
					ReadLockAccess(mtx);
				else
					WriteLockAccess(mtx);
			}


			ExtraValueStorage* storage = nullptr;
			std::variant<std::monostate, WriteLock, ReadLock> lock;

			operator ExtraValueStorage* () { return storage; }
			operator bool () const noexcept { return storage; }

			ExtraValueStorage* operator->() { assert(storage); return storage; }


			ExtraValueStorage& operator*() { assert(storage); return *storage; }


			void ReadLockAccess(Mutex& mtx)
			{
				if (lock.index() == 0)
				{
					lock = WriteLock{ mtx };
				}
			}

			void WriteLockAccess(Mutex& mtx)
			{
				if (lock.index() == 0)
				{
					lock = WriteLock{ mtx };
				}
			}

			void SetStorage(ExtraValueStorage* ptr)
			{
				storage = ptr;
				if (!ptr)//if no value is found this has no reason to maintain a lock
				{
					switch (lock.index())
					{
					case 1:
						std::get<1>(lock).unlock();
						break;
					case 2:
						std::get<2>(lock).unlock();
						break;
					}
				}
			}

			void SetStorage(ExtraValueStorage& ref)
			{
				return SetStorage(&ref);
			}
		};


		virtual void Serialize(TOME::SerialBuffer& buffer, bool& result)
		{

			using T = ExtraValueData;
			constexpr bool consttest1 = TOME::serial_data<ExtraValueData>;
			constexpr bool consttest2 = std::is_trivially_move_assignable_v<T> && std::is_trivially_move_constructible_v<T> && !pointer_type<T> && !std::is_polymorphic_v<T>;
			
			//TODO: List the actual version dude.
			buffer.Serialize(_tickValue, 1 << 8 * 3);

			if (buffer.IsSaving() == true) {
				int vect_size = _valueData.size();
				int mani_size = ExtraValueInfo::GetCountUpto(ExtraValueType::Exclusive);

				logger::debug("SAVING: {} vs {}", vect_size, mani_size);

				buffer.Serialize(_valueData);
				buffer.Serialize(_recoveryData);
			}
			else
			{
				std::vector<ExtraValueData> value_dump;
				std::vector<std::pair<DataID, RegenData>> recover_dump;



				buffer.Serialize(value_dump);
				buffer.Serialize(recover_dump);

				//Should I algorithm for this?


				int vect_size = value_dump.size();
				int mani_size = ExtraValueInfo::GetManifestCount();

				logger::debug("LOADING: {} vs {}", vect_size, mani_size);

				//Removing a check for manifest size because then it will won't init
				// new actor values when loading an actor in.
				for (uint32_t i = 0; i < vect_size && i < mani_size; i++)
				{
					//TODO: Turn this into a function I can use.
					//If I can only show these once per cycle, that would be great.
					ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByManifest(i);


					if (!info) {
						if (i < mani_size)
							logger::warn("ExtraValueInfo at {} not found. Tossing data.", i);

						continue;
					}

					DataID id = info->GetDataID();

					if (id == ExtraValueInfo::FunctionalID) {
						logger::warn("ExtraValueInfo {}({}) is now functional. Tossing data.", info->GetName(), id);
						continue;
					}

					//If this is an npc storage && info is exclusive, continue
					logger::debug("Deserializing '{}'. Value: [{}/{}/{}/{}], {} -> {}",
						info->GetName(),
						value_dump[i]._base,
						value_dump[i]._evMods[0],
						value_dump[i]._evMods[1],
						value_dump[i]._evMods[2],
						i, id);

					_valueData[id] = value_dump[i];

				}


				for (uint32_t i = 0; i < recover_dump.size(); i++)
				{
					std::pair<DataID, RegenData>& rec_data = recover_dump[i];

					//If it's zero, who gives a shit, these don't have a default value of anything.
					if ((isnan(rec_data.second._pool) || rec_data.second._pool <= 0) && rec_data.second._time <= 0) {
						logger::debug("Regen data {} trivial, skipping deserialization.", i);
						continue;

					}

					ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByManifest(rec_data.first);

					if (!info || info->GetRecoverInfo() == nullptr) {
						logger::warn("ExtraValueInfo at {} not found. Tossing regen data.", i);
						continue;
					}
					DataID id = info->GetDataID();

					if (id == ExtraValueInfo::FunctionalID) {
						logger::warn("ExtraValueInfo at {} has become functional. Tossing regen data.", i);
						continue;
					}


					auto predicate = [=](std::pair<DataID, RegenData> pair) {
						return pair.first == id;
						};

					//If this is an npc storage && info is exclusive, continue
					auto result = std::find_if(_recoveryData.begin(), _recoveryData.end(), predicate);
					;
					if (_recoveryData.end() != result) {
						logger::debug("Deserializing '{}' Recovery Data. : [{}/{}], {} -> {}",
							info->GetName(),
							rec_data.second._time,
							rec_data.second._pool,
							rec_data.first, id);

						result->second = rec_data.second;
					}
					else {
						logger::debug("'{}''s Recovery Data failed to Deserialize.", info->GetName());
					}
				}
			}
		}

		static void HandleSerialize(TOME::SerialBuffer& buffer, bool& result, ExtraValueStorage& storage)
		{
			storage.Serialize(buffer, result);
		}


		struct SerializeClass
		{

			void operator()(TOME::SerialBuffer& buffer, bool& result, std::pair<RE::FormID, ExtraValueStorage>& entry)
			{

				WriteLock guard{ accessLock };

				result = buffer.SerializeFormID(entry.first);//Needs to be a particular type of object, serializable formID


				if (result && buffer.IsLoading() == true) {
					 RE::Actor* actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

					if (!actor) {
						logger::error("Actor FormID {:08X} invalid, dumping.", static_cast<RE::FormID>(entry.first));
						result = false;
					}
					else {
						logger::info("Actor {}(FormID:{:08X}) successful, creating and deserializing.", actor->GetName(), static_cast<RE::FormID>(entry.first));
						entry.second = ExtraValueStorage{ actor, true };
					}
				}


				//If the pointer is null or the success is false, it will dump the data, and return unsuccessful.
				result = buffer.DiscardFailure(entry.second, result);

				if (result)
					logger::info("serialized: {:08X}", static_cast<RE::FormID>(entry.first));
				else
					logger::error("failed to de/serialize");


				

			}
		};


		//using EVStorageMap = SerializableMap<SerialFormID, ExtraValueStorage*, SerializeClass, SerializeClass>;
		//This should NOT be using the LogHandler, instead it should be using something that loads the default data of the second part properly.
		using SerialMapClass2 = TOME::map_entry_handler<TOME::LogHandler>::type<std::map<TOME::FormID, ExtraValueStorage>>;

		using SerialMapClass = TOME::SerialHandler<std::map<RE::FormID, ExtraValueStorage>, SerializeClass>;

		static std::map<RE::FormID, ExtraValueStorage>& _valueTable;


	//static
	private:
	



	protected:


		void ResetStorageImpl(RE::Actor* actor, bool init_default);




	public:



		virtual void ResetStorage(RE::Actor* actor, bool init_default = false)
		{
			return ResetStorageImpl(actor, init_default);
		}

		static StorageView GetStorage(RE::Actor* actor);

		
		static StorageView ObtainStorage(RE::Actor* actor);


		static bool RemoveStorage(RE::FormID _id);

		static void RemoveAllStorages();

	//membered
		//This should delete it's copy assignment and intializers.

	protected:

	public:
		std::vector<ExtraValueData> _valueData;

		//This serves as both a map and manifest. And while, yes, this is a vector, ideally the data is concurent, never to be deleted
		// until it no longer exists.
		//I think I MAY, want to introduce pooling into this.

		
		std::vector<std::pair<DataID, RegenData>> _recoveryData;
		
		float _tickValue = 0;
		
		float lastGameTime = 0;
		
		bool initialized = false;

		ExtraValueStorage() = default;

		ExtraValueStorage(RE::Actor* actor, bool uses_default);
		
	public:
		float* GetExtraValueDelay(DataID id)
		{//switch to binary search if it's sorted
			auto result = std::find_if(_recoveryData.begin(), _recoveryData.end(), [=](auto it) { return it.first == id; });

			if (_recoveryData.end() != result) {
				return &result->second._time;
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
		
		void UpdateDelay(RE::Actor* owner, DataID id, ExtraValueInfo* info)
		{//Sets the delay based on info.
			if (!info)
				return;

			RecoverInfo* rec_data = info->GetRecoverInfo();
	
			if (!rec_data || !rec_data->recoveryDelay)
				return;

			float* curr_delay = GetExtraValueDelay(id);

			if (!curr_delay)
				return;

			float new_delay = rec_data->recoveryDelay(owner)->Call();

			if (new_delay > *curr_delay) {
				*curr_delay = new_delay;
				logger::debug("new delay {}", new_delay);
			}
		}


		//Make versions of these that accept the other modifier set up too.
		float GetValue(RE::Actor* owner, DataID id, ExtraValueInput modifiers = ExtraValueInput::All, ExtraValueInfo* info = nullptr)
		{
			if (_valueData.size() <= id) {
				logger::critical("id given was larger than valueData's size. id: {}, size: {}, initialized: {}", id, _valueData.size(), initialized);
			}
			//Here, if info is loaded an update will be performed if it's on constant update.
			duo<float> result = _valueData.at(id).GetValue(modifiers);
			
			if (isnan(result.first) == true) {
				info = info ? info : ExtraValueInfo::GetValueInfoByData(id);
				result.first = info->GetExtraValueDefault(owner);
			}

			return result.first + result.second;
		}

		duo<float> SetValue(RE::Actor* owner, DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr)
		{ 
			//If flag is constant, then it will need to check if its actually allowed to set that.

			if (modifier == ExtraValueInput::Base) {
				info = info ? info : ExtraValueInfo::GetValueInfoByData(id);

				auto default_data = info->FetchDefaultInfo();

				if (default_data && default_data->_type == DefaultInfo::Constant) {
					//Cant set the base of default data.
					logger::warn("Cannot set the base value of constant Extra Value '{}'.", info->GetName());
					value = _valueData[id].GetValueUnsafe(modifier);
					return { value, value };
				}
			}

			return _valueData[id].SetValue(value, modifier); 


		}

		//Add in the accuser, no idea if that might come in handly
		duo<float> ModValue(RE::Actor* owner, RE::Actor* aggressor, DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr)
		{
			//If used with info, it will attempt to update the recovery data.
			//No info should actually mean something, meaning don't run needed stuff.

			if (modifier == ExtraValueInput::Base) {
				info = info ? info : ExtraValueInfo::GetValueInfoByData(id);

				auto default_data = info->FetchDefaultInfo();

				if (default_data && default_data->_type == DefaultInfo::Constant) {
					//Cant set the base of default data.
					value = _valueData[id].GetValueUnsafe(modifier);
					return { value, value };
				}
			}

			
			if (info && value < 0 && modifier == ExtraValueInput::Damage)
				UpdateDelay(owner, id, info); 
			
			
			return _valueData[id].ModValue(value, modifier); 

			
			float current = GetValue(owner, id, ExtraValueInput::All, info);
			float damage = modifier == ExtraValueInput::Damage ?  _valueData[id].GetValueUnsafe(ExtraValueInput::Damage) : 0;
			//float damage =  _valueData[id].GetValue(ExtraValueInput::Damage);
			
			_valueData[id].ModValue(value, modifier); 
			

			return {};
			if (current > 0 && current + value <= 0){
				SendEvent("OnActorValueDepleted", owner, info->GetFixedName(), aggressor);
			}
			else if (damage < 0 && damage + value >= 0) {
				SendEvent("OnActorValueRestored", owner, info->GetFixedName(), aggressor);
			}
		}


		void HandleRecoveryUpdate(RE::Actor* owner, std::pair<DataID, RegenData>& entry, ExtraValueInfo* info, float delta_time)
		{
			if (!info)
				return;
			
			RecoverInfo* recover_data = info->GetRecoverInfo();

			if (!recover_data || !recover_data->recoveryRate)
				return;




			float remainder = delta_time - entry.second._time;

			if (remainder <= 0) {
				//This WILL cause a crash. Make a function for this.
				entry.second._time -= delta_time;

				//if (a_this->IsPlayerRef())
				//	logger::info("B {}", remainder);

				return;
			}

			entry.second._time = 0;


			if (_valueData[entry.first].GetValueUnsafe(ExtraValueInput::Damage) >= 0) {
				return;
			}

			
			float mod_value = recover_data->recoveryRate(owner)->Call();
			
			//Would be good to isolate this some how.
			float current = GetValue(owner, entry.first, ExtraValueInput::Maximum, info);

			if (recover_data->IsFixed() == false){
				


				if (owner->IsPlayerRef() == true)
					logger::debug("!!rec {} -> {}/{}/{}", entry.first, mod_value, current, remainder);
				
				//Want to move this to a setting the mod creates and that people can make.
				constexpr float min_regen = 1.f;
				
				mod_value = mod_value / 100;
				mod_value *= fmax(current, min_regen);
			}
			
			mod_value *= remainder;




			if (owner->IsPlayerRef() == true)
				logger::debug("rec {} -> {}", entry.first, mod_value);

			float damage = _valueData[entry.first].GetValueUnsafe(ExtraValueInput::Damage);



			constexpr bool _temp_clamped = true;

			if (_temp_clamped && abs(damage) > current) {
				mod_value += abs(damage) - current;
			}





			auto out = _valueData[entry.first].ModValue(mod_value, ExtraValueInput::Damage);
			if (out.first != out.second) {
				info->SendOnActorValueChanged(owner, nullptr, RE::ActorValueModifier::kDamage, out.first, out.second);
				//SendEvent("OnActorValueRestored", owner, info->GetFixedName(), owner);
			}
			//This actually triggers
			//if (damage < 0 && damage + mod_value >= 0) {
			//	SendEvent("OnActorValueRestored", owner, info->GetFixedName(), owner);
			//}
		}

		void Update(RE::Actor* owner, float delta_time)
		{
			_tickValue += delta_time;

			if (_tickValue < tickTime)
				return;

			//This confines the updates to whatever the tick time should be, making it fire off not as fast.

			delta_time = _tickValue;

			_tickValue = 0;

			for (int i = 0; i < _recoveryData.size(); i++){
				std::pair<DataID, RegenData>& entry = _recoveryData[i];

				ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByData(entry.first);

				if (!info) {
					//Need a way to report this, ONCE.
					continue;
				}

				HandleRecoveryUpdate(owner, entry, info, delta_time);
			}
		}



	};



	class PlayerStorage : public ExtraValueStorage
	{
	private:
		static PlayerStorage& _singleton;//Note, defined elsewhere.

		//For it's singleton nature, I'm considering making this an atomic bool
		inline static bool _playable = false;
		//Would like this to be unordered some time.
	public:
		//Could order this to make searching easy
		std::vector<std::pair<DataID, SkillData>> _skillMap;

		SkillData* GetSkillData(DataID id)
		{
			//I'm just gonna run it.

			auto it = _skillMap.begin();
			auto end = _skillMap.end();

			it = std::find_if(it, end, [id](auto&& pair) {return id == pair.first; });

			
			return it != end ? &it->second : nullptr;
		}

	public:
		static PlayerStorage* GetSingleton()
		{
			return &_singleton;
		}

		//Returns the singleton, but reprocesses it should it not be playable. Also, playable is not serialized.
		static PlayerStorage* GetAsPlayable(bool excuse_unplayable)
		{
			//Lock this function.
			auto player_storage = GetSingleton();

			if (_playable)
				return player_storage;


			auto player = RE::PlayerCharacter::GetSingleton();

			//Need to access this while 3d loading isn't present. A new rule has to be established too. As long as a request for a value comes
			// from an actor, fulfill it.
			if constexpr(false)
			if (player->Is3DLoaded() == false) {
				if (!excuse_unplayable)
					logger::warn("PlayerStorage requested as playable is not 3DLoaded. Playability required, returning nullptr.");
				else
					logger::debug("PlayerStorage requested as playable is not 3DLoaded. Request cannot be met as required.");
				
				//*player_storage = PlayerStorage(true);
				player_storage->ResetStorage(false);

				return excuse_unplayable ? player_storage : nullptr;
			}

			//*player_storage = PlayerStorage(true);
			player_storage->ResetStorage(true);
			

			return player_storage;
		}


		static void HandleRevert(PlayerStorage& storage)
		{
			constexpr bool test = TOME::detail::static_revertable<PlayerStorage>;
			logger::debug("Reverting player storage");
			storage._playable = false;
		}


		

		
		void Serialize(TOME::SerialBuffer& buffer, bool& result) override
		{
			if (buffer.IsLoading() == true)
			{
				logger::debug("Making new player storage");
				//this should just be ResetStorageImpl honestly.
				ResetStorage(true);
			}
			
			constexpr bool serialize_skill = true;

			if constexpr (serialize_skill)
			{
				decltype(_skillMap) dataBuff;

				bool is_saving = buffer.IsSaving();

				buffer.Serialize(is_saving ? _skillMap : dataBuff, "2.0.0.7"_v.pack());

				if (!is_saving && dataBuff.size())
				{
					for (auto& [prev_id, data] : dataBuff)
					{
						ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByManifest(prev_id);

						if (!info) {
							logger::warn("ExtraValueInfo at DataID {} not found. Tossing data.", prev_id);
							continue;
						}

						DataID id = info->GetDataID();

						if (id == ExtraValueInfo::FunctionalID) {
							logger::warn("ExtraValueInfo {}({}) is now functional. Tossing data.", info->GetName(), id);
							continue;
						}

						SkillData* skill_data = GetSkillData(id);

						if (skill_data) {
							//This is all I care about in reality, the rest of the data may get saved on accident but I couldn't
							// give a shit about it to be real.
							skill_data->xp = data.xp;
							skill_data->xp = data.legendaryLevels;
						}
						else {
							logger::warn("Extra Value {}({}) no longer accepted as skill. Tossing Data.", info->GetName(), id);
						}


					}
				}
			}

			__super::Serialize(buffer, result);
			
			//Here is where I'll do the skill stuff.
		}


		PlayerStorage() = default;


		void ResetSkillData(RE::PlayerCharacter* player = nullptr)
		{
			//Chase the function this is based on, 1406E69A0. I believe that I may want to hook it so this can go off (and in turn, keeping actor value data around)

			//Move this
			static RE::Setting* fSkillUseCurve = RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUseCurve");

			constexpr float maximum = 100.f;


			if (!player)
				player = RE::PlayerCharacter::GetSingleton();

			for (auto& [data_id, skill_data] : _skillMap)
			{
				ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByData(data_id);


				auto skill_info = info->GetSkillInfo();

				if (!skill_info) {
					return;
				}

				auto skills = skill_info->GetSkill(player);


				
				float base = info->GetExtraValue(player, RE::ActorValueModifier::kTotal);
				
				skill_data.level = base;
				skill_data.levelThreshold = base >= maximum ? 0.f : (std::pow(base, fSkillUseCurve->GetFloat()) * skills.improveMult) + skills.improveOffset;
				skill_data.xp = 0;

			}
		}

	protected:
		friend class ExtraValueStorage;
		
		void ResetStorageImpl(bool init_default)
		{
			if (_valueData.size() == 0)
				return;

			auto* player = RE::PlayerCharacter::GetSingleton();

			//auto& skill_map = _skillMap;
			//skill_map = ExtraValueInfo::GetSkillfulValues(player);
			ExtraValueInfo::FillSkillfulValues(player, _skillMap);

			if (init_default)
				_playable = true;

			ResetSkillData(player);
		}

	public:
		void ResetStorage(RE::Actor* actor, bool init_default = false) override
		{
			if (!actor || actor->IsPlayerRef() == false) {
				logger::error("Actor in PlayerStorage::ResetStorage is not the player. ({:08X})", actor ? actor->formID : 0);
				return;
			}

			ResetStorageImpl(init_default);

			return __super::ResetStorage(actor, init_default);
		}

		void ResetStorage(bool init_default)
		{
			return ResetStorage(RE::PlayerCharacter::GetSingleton(), init_default);
		}


		PlayerStorage(bool use_default) : ExtraValueStorage(RE::PlayerCharacter::GetSingleton(), use_default)
		{
			
			//if deserializing, we won't really do anything, just create it.

			//auto size = _valueData.size();;
			
			

			ResetStorageImpl(use_default);
		}
	};


	using StorageView = ExtraValueStorage::StorageView;
}
