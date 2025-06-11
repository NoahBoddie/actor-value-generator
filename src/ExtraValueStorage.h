#pragma once

#include "Types.h"

#include "ExtraValueInfo.h"//Put in cpp please pepeW
#include "Serialization/SerialConstructor.h"
#include "Serialization/SerializationTypePlayground.h"
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
		void SetValue(float value, ExtraValueInput modifier = ExtraValueInput::Base)
		{
			std::lock_guard<std::mutex> behaviour_guard(av_lock);

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
			std::lock_guard<std::mutex> behaviour_guard(av_lock);

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
	
	struct PlayerStorage;

	class ExtraValueStorage : public SerializationHandler
	{
	public:
		
		struct SerializeClass
		{
			void operator()(std::pair<const SerialFormID, ExtraValueStorage*>& entry, SerialArgument& serializer, bool& success)
			{
				success = serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID


				bool is_deserializing = serializer.IsDeserializing();

				if (success && is_deserializing == true) {
					RE::Actor* actor = nullptr;
					if (success) {
						actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

						if (!actor) {
							logger::error("Actor FormID {:08X} invalid, dumping.", static_cast<RE::FormID>(entry.first));
							success = false;
						}
						else {
							logger::info("Actor {}(FormID:{:08X}) successful, creating and deserializing.", actor->GetName(), static_cast<RE::FormID>(entry.first));
							entry.second = new ExtraValueStorage(actor, true);
						}
					}
					else
					{
						logger::error("Failure deserializing form ID.");
					}
				}

				//If the pointer is null or the success is false, it will dump the data, and return unsuccessful.
				success = serializer.DumpIfFailure(entry.second, success);

				if (success)
					logger::info("serialized: {:08X} at {:08X}", static_cast<RE::FormID>(entry.first), (uint64_t)entry.second);
				else
					logger::error("failed to de/serialize");
			}
			
			void operator()(std::map<SerialFormID, ExtraValueStorage*>&)
			{
				logger::debug("Reverting extra value storage");
				RemoveAllStorages();
			}
		};

		using EVStorageMap = SerializableMap<SerialFormID, ExtraValueStorage*, SerializeClass, SerializeClass>;


	//static
	private:
		
		//May make these unique, and only give out regular pointers.
		//inline static EVStorageMap& _valueTable = Initializer<EVStorageMap, PrimaryRecordType>(HandlePrimarySerializer, PrimaryRecordType::ExtraValueStorage);
		inline static EVStorageMap& _valueTable = SerializationHandler::CreatePrimarySerializer<EVStorageMap>(PrimaryRecordType::ExtraValueStorage);
		//inline static std::map<RE::FormID, ExtraValueStorage*>* _valueTable = nullptr;
	
	protected:
		ExtraValueStorage() = default;


		void ResetStorageImpl(RE::Actor* actor, bool init_default);

		


	public:
		void ResetStorage(RE::Actor* owner, bool init_default = false);


		static ExtraValueStorage* GetStorage(RE::Actor* actor);

		
		static ExtraValueStorage& ObtainStorage(RE::Actor* actor);


		static bool RemoveStorage(RE::FormID _id);

		static void RemoveAllStorages()
		{
			for (auto& store_pair : *_valueTable)
			{
				delete store_pair.second;
			}

			_valueTable->clear();
		}

	//membered
		//This should delete it's copy assignment and intializers.

	protected:

	public:
		SerialVector<ExtraValueData> _valueData;

		//This serves as both a map and manifest. And while, yes, this is a vector, ideally the data is concurent, never to be deleted
		// until it no longer exists.
		//I think I MAY, want to introduce pooling into this.

		
		SerialVector<std::pair<DataID, RegenData>> _recoveryData;
		
		float _tickValue = 0;
		
		float lastGameTime = 0;

		ExtraValueStorage(RE::Actor* actor, bool uses_default);
		
	public:
		float* GetExtraValueDelay(DataID id)
		{//switch to binary search if it's sorted
			auto result = std::find_if(_recoveryData->begin(), _recoveryData->end(), [=](auto it) { return it.first == id; });

			if (_recoveryData->end() != result) {
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
			//Here, if info is loaded an update will be performed if it's on constant update.
			duo<float> result = _valueData[id].GetValue(modifiers);
			
			if (isnan(result.first) == true) {
				info = info ? info : ExtraValueInfo::GetValueInfoByData(id);
				result.first = info->GetExtraValueDefault(owner);
			}

			return result.first + result.second;
		}

		void SetValue(RE::Actor* owner, DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr)
		{ 
			//If flag is constant, then it will need to check if its actually allowed to set that.

			if (modifier == ExtraValueInput::Base) {
				info = info ? info : ExtraValueInfo::GetValueInfoByData(id);

				auto default_data = info->FetchDefaultInfo();

				if (default_data && default_data->_type == DefaultInfo::Constant) {
					//Cant set the base of default data.
					logger::warn("Cannot set the base value of constant Extra Value '{}'.", info->GetName());
					return;
				}
			}


			_valueData[id].SetValue(value, modifier); 
		}

		//Add in the accuser, no idea if that might come in handly
		void ModValue(RE::Actor* owner, RE::Actor* aggressor, DataID id, float value, ExtraValueInput modifier = ExtraValueInput::Base, ExtraValueInfo* info = nullptr)
		{
			//If used with info, it will attempt to update the recovery data.
			//No info should actually mean something, meaning don't run needed stuff.

			if (modifier == ExtraValueInput::Base) {
				info = info ? info : ExtraValueInfo::GetValueInfoByData(id);

				auto default_data = info->FetchDefaultInfo();

				if (default_data && default_data->_type == DefaultInfo::Constant) {
					//Cant set the base of default data.
					return;
				}
			}

			
			if (info && value < 0 && modifier == ExtraValueInput::Damage)
				UpdateDelay(owner, id, info); 
			
			
			_valueData[id].ModValue(value, modifier); 

			return;
			float current = GetValue(owner, id, ExtraValueInput::All, info);
			float damage = modifier == ExtraValueInput::Damage ?  _valueData[id].GetValueUnsafe(ExtraValueInput::Damage) : 0;
			//float damage =  _valueData[id].GetValue(ExtraValueInput::Damage);
			
			_valueData[id].ModValue(value, modifier); 
			

			return;
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





			_valueData[entry.first].ModValue(mod_value, ExtraValueInput::Damage);
			
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

			for (int i = 0; i < _recoveryData.GetSize(); i++){
				std::pair<DataID, RegenData>& entry = _recoveryData[i];

				ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByData(entry.first);

				if (!info) {
					//Need a way to report this, ONCE.
					continue;
				}

				HandleRecoveryUpdate(owner, entry, info, delta_time);
			}
		}


		void HandleSerialize(SerialArgument& serializer, bool& success) override 
		{ 
			serializer.Serialize(_tickValue, 1 << 8 * 3);

			if (serializer.IsSerializing() == true) {
				int vect_size = _valueData->size();
				int mani_size = ExtraValueInfo::GetCountUpto(ExtraValueType::Exclusive);

				logger::debug("SAVING: {} vs {}", vect_size, mani_size);
				
				serializer.Serialize(_valueData);
				serializer.Serialize(_recoveryData);
			}
			else 
			{
				SerialVector<ExtraValueData> value_dump;
				SerialVector<std::pair<DataID, RegenData>> recover_dump;


				
				serializer.Serialize(value_dump);
				serializer.Serialize(recover_dump);

				//Should I algorithm for this?

				
				int vect_size = value_dump->size();
				int mani_size = ExtraValueInfo::GetManifestCount();

				logger::debug("LOADING: {} vs {}", vect_size, mani_size);

				//Removing a check for manifest size because then it will won't init
				// new actor values when loading an actor in.
				for (uint32_t i = 0; i < vect_size && i < mani_size; i++)
				{
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
				

				for (uint32_t i = 0; i < recover_dump->size(); i++)
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
					auto result = std::find_if(_recoveryData->begin(), _recoveryData->end(), predicate);
			;
					if (_recoveryData->end() != result) {
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
		SerialVector<std::pair<DataID, SkillData>> _skillMap;

		SkillData& GetSkillData(DataID id)
		{
			//I'm just gonna run it.
			auto result = std::find_if(_skillMap->begin(), _skillMap->end(), [id](auto&& pair) {return id == pair.first; });

			return result->second;
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

		void Revert() override 
		{
			logger::debug("Reverting player storage");
			_playable = false;
		}
		
		void HandleSerialize(SerialArgument& serializer, bool& success) override
		{
			if (serializer.IsDeserializing() == true)
			{
				logger::debug("Making new player storage");
				//this should just be ResetStorageImpl honestly.
				*this = PlayerStorage(true);
			}
			
			__super::HandleSerialize(serializer, success);
			
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

			for (auto& [data_id, skill_data] : *_skillMap)
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
		
		void ResetStorageImpl(RE::PlayerCharacter* owner, bool init_default)
		{
			__super::ResetStorageImpl(owner, init_default);

			if (_valueData->size() == 0)
				return;

			auto* player = RE::PlayerCharacter::GetSingleton();

			auto& skill_map = _skillMap.get();


			skill_map = ExtraValueInfo::GetSkillfulValues(owner);

			if (init_default)
				_playable = true;

			ResetSkillData(player);
		}

	public:
		void ResetStorage(bool init_default)
		{
			ResetStorageImpl(RE::PlayerCharacter::GetSingleton(), init_default);
		}


		PlayerStorage(bool use_default) : ExtraValueStorage(RE::PlayerCharacter::GetSingleton(), use_default)
		{
			
			//if deserializing, we won't really do anything, just create it.

			//auto size = _valueData->size();;
			
			

			ResetStorageImpl(RE::PlayerCharacter::GetSingleton(), use_default);
		}
	};
}
