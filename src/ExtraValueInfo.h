#pragma once

#include "Types.h"
#include "Utility.h"
#include "Serialization/SerializableList.h"
#include "Serialization/SerializationTypePlayground.h"
namespace AVG
{

	class ExtraValueInfo;
	class AdaptiveValueInfo;
	class ExclusiveValueInfo;
	class FunctionalValueInfo;


	
#define NOT_ABSTRACT(mc_type) static_assert(!std::is_abstract_v<mc_type>, STRINGIZE(Type mc_type is abstract.));


	

	/*
	
	//This should allow me to make them like: "ExtraValueInfoFactory::Create("AdaptiveValueInfo")
	//using ExtraValueInfoItemFactory = ClassFactory<ExtraValueInfo, StringLiteral>;
	
	//template <class DerivedType, StringLiteral Type>
	//using ExtraValueInfoFactoryComponent = IndirectFactoryComponent<ExtraValueInfo, StringLiteral, DerivedType, Value>;

	static RoutineItem* Create(RecordIterator& data, uint16_t pos, RoutineItemType type)//Include record.
	{
		RoutineItem* item = RoutineItemFactory::Create(type);

		item->_position = pos;

		item->LoadFromViewImpl(data);

		return item;
	}

	template<std::derived_from<RoutineItem> DerivedItem>
	inline static DerivedItem* CreateAs(RecordIterator& data, uint16_t pos, RoutineItemType type)//Include record.
	{
		RoutineItem* item = Create(data, pos, type);

		DerivedItem* result = dynamic_cast<DerivedItem*>(item);

		//This needs to be handled better
		if (!result) {
			delete item;
		}

		return result;
	}
	//*/

    //All of these will likely be changed to info at some later point.

	//This is the info needed to treat this av like a skill, growing when given experience. Player only obviously.
	//Unimplmented obviously

	//While I would say no reason to have this on the player object, that's not really true, so lets get to fixing that. into something else.
	
	
	struct Skill
	{//This is a structure I'll be using to match RE::ActorValueInfo::Skill later, the structure aims to be the same (but the names proper)
		float useMult = 1.f;        // 00
		float useoffset = 0.f;     // 04
		float improveMult = 1.f;    // 08
		float improveOffset = 0.f;  // 0C
	};
	//TODO: static assert all skill entries please
	struct SkillInfo
	{
		union
		{
			struct
			{
				float useMult;
				float useOffset;
				float improveMult;
				float improveOffset;
			};
			Skill skill{};
		};
		
		float increment = 1;
		bool grantsXP = true;
		bool isAdvance = true;

		Skill GetSkill(RE::Actor*)
		{
			return skill;
		}

	};

	struct SkillData
	{
		float			level = 0;				//How many times it's leveled
		float			xp = 0;					//current experience
		float			levelThreshold = 1;		//level threshold to level up
		std::uint32_t	legendaryLevels = 0;
	};
	
	using ValueFormula = LEX::Formula<float(RE::Actor::*)()>;
	using SetFormula = LEX::Formula<void(RE::Actor::*)(RE::Actor*, float, float)>;


	//This is functionally default data. Update data isn't something I'll want to implement for a while, and since this doesn't serialize I'm comfortable
	// doing it later.
    struct DefaultInfo
    {
		enum Type
		{
			Implicit,	//The values will be set on start, and will not update
			Explicit,	//The values can only change from defined default when done so explicitly
			Constant,	//The values never change from the defined default
		};

		Type _type = Type::Implicit;

		//rate is required.
		//If value to update is zero, no timed update data will be used, and instead it will only update upon load.
		ValueFormula defaultFunction;
        
    };




    struct RecoverInfo
    {
		enum Flags : uint8_t
		{
			None = 0,
			Fixed = 1 << 0,		//When fixed is enabled, recovery is not percent based, but exactly what is returned.
			Negative = 1 << 1,	//This recovery will only acknowledge decay, and at 0 will no longer check for recovery
			Positive = 1 << 2,	//This recovery will only acknowledge regen, and will not increase if the damage value is 0.
			Polar = 0b110,		//Same as not having negative nor positive
		};

		float tmp_recDelay{};
		float tmp_recRate{};
		
		//While delay is not required, the rate is similarly required.
		ValueFormula recoveryDelay;  //Null will mean there is no delay, if recovery exists.
		ValueFormula recoveryRate;  //Null here means no recovery.
    
		Flags flags = None;

		constexpr bool IsFixed() { return !!(flags & Flags::Fixed); }

		//I thought about unique pointers, but realized, what's the point? (ba dum tssk)
	};

	struct RegenData
	{
		float _time = 0;
		float _pool{ NAN };
	};

	struct InputFlags
	{
		constexpr static InputFlags All() noexcept
		{
			return InputFlags{ ExtraValueInput::All, ExtraValueInput::All };
		}

		ExtraValueInput set{};
		ExtraValueInput get{};
	};



	enum class InfoFlags
	{
		None = 0,
		//Delete this.
		DerivedAffected = 1 << 0,	//Do plugins derived inherit the av aliases of this EVI?
		Negative = 0b010,
		Positive = 0b100,
		Direction = 0b110,	//This may not belong, but the idea is you use & on the flags with direction,
							// then if it's equal to negative the it clamps that way, positive clamps that way, neither it goes both.
	};

	//Along with this sort of information, I believe I will likely need a class called loading info.
	// This will store all the data that I don't want taking up permenant space, so I can just delete it later. Maybe, I can unionize it.
	// Anywho, this load data can store the pure string data that I want to carry onto, because I'm going to need something like that
	// for functions to be coroutines to be loaded first.


	

	struct ExtraValueListT
	{
		//The idea of this is that I serialize this, and it serves as the switch between serializing one thing or the other. Kinda dumb
		// I know but I made this a bit too object focused. UNLESS, I make a primary serializer that's basically just a call back function!
		// I'm so smart.
		template <class temp>
		void operator()(temp& entry, SerialArgument& serializer, bool& success)
		{
			
		}
	};



    class ExtraValueInfo// : public SerializationHandler//Not actually a serialization handler, the wrapper handles that (for now?)
    {
	protected:
		ExtraValueInfo() = default;

	public:
		struct SerializeEntry
		{
			void operator()(ExtraValueInfo*& entry, SerialArgument& serializer, bool& success)
			{
				constexpr std::string_view invalid = "INVALID";

				SerialString name{};

				bool is_saving = serializer.IsSerializing();

				if (is_saving)
				{
					if (!entry || entry->IsFunctional() == true)
						name = std::string{ invalid };
					else
						name = std::string{ entry->GetName() };
				}


				serializer.Serialize(name);
				
				logger::debug("{} '{}'", is_saving ? "saving" : "loading", name);

				if (!is_saving && name != invalid){
					entry = ExtraValueInfo::GetValueInfoByName(name);
				}
			}
		};

		static void SerializeList(SerialArgument& serializer, bool& success)
		{
			bool is_saving = serializer.IsSerializing();

			serializer.Serialize(is_saving ? _endTypeIndex[0] : _prevEndTypeIndex[0]);
			serializer.Serialize(is_saving ? _endTypeIndex[1] : _prevEndTypeIndex[1]);
			serializer.Serialize(is_saving ? _extraValueList : _previousExtraValueList);
		}

		static inline auto& list_serialize = SerializationHandler::CreatePrimarySerializer<SerialCallback>(PrimaryRecordType::ExtraValueInfo, SerializeList);


		enum ExtraValueType
		{
			Adaptive,
			Exclusive,	//Not created yet, exclusive is a value that is adaptive for the player, and functional for npcs. making something that's 
			// only takes up as much space as it needs to. It's also fine for both its get/set to not be defined
			Functional,
			Total
		};


		static void Create(std::string_view name, ExtraValueType type, const FileNode& node, bool legacy);

		static constexpr DataID FunctionalID = 0xFFFFFFFF;

	protected:
		

		static inline bool _finish = false;

		//called when included into the EVI list
		

    //static
    private:

		//Note that the value specified is the END of the values. So it means that if it's equal its invalid for the type you're
		// asking about.
		static inline DataID _endTypeIndex[ExtraValueType::Total];


		static inline DataID _nextDataID = 0;
		static inline DataID _nextDataOffset = 0;
        static inline ValueID _nextValueID = static_cast<ValueID>(RE::ActorValue::kTotal) + 1;

		//This only serializes 1 and 2 from the original
		static inline DataID _prevEndTypeIndex[ExtraValueType::Functional];
        //I would like to give this a rule in that it can only deserialize non-functional values. The same should go on other vector.
        static inline SerialVector<ExtraValueInfo*, SerializeEntry> _previousExtraValueList;

        // Should require validation. This is also the manifest now. When serialized, we just serialize the strings in
        // sequence.
        //static inline std::vector<AdaptiveValueInfo*> _adaptiveInfoList;


		

        //This may use a string hash sometime in the future.
        //static inline std::map<std::string, ExtraValueInfo*> _infoTable;


		//Could make a macro for the manipulation of this, with a built in null condition check.
		static inline std::array<std::vector<ExtraValueInfo*>, ExtraValueType::Total>* _infoTypeMap = new std::array<std::vector<ExtraValueInfo*>, ExtraValueType::Total>();

#define __infoTypeMap (*_infoTypeMap)

		//This serves to tell the cut off between player only recovery and adaptive.
		static inline uint32_t _exclusiveRecoverIndex = 0;

		//A list of indices that can recover. Their new vector is made on the spot, instead of manually.
		static inline std::vector<DataID> _recoverValueList{};
		static inline std::vector<DataID> _skillValueList{};

		//This is a way to interface with already existing actor values in a way that is compliant with the current system.
		// Such as giving recovery data to existing actor values. Though, it would seem this would be something these would 
		// severely fight over right? So I'm not keen to implement it yet. 
		//I think I would probably create this piece meal. Like you need to specify the priority of your datas changes, and only
		// one makes it through. So your recovery data implementation can only have 1 winner, your event's list can combine, 
		// etc, but for whatever cannot be merged, only one can win.
		//std::array <ExtraValueInfo*, (int)RE::ActorValue::kTotal> _overrideValues;


		static inline SerialVector<ExtraValueInfo*, SerializeEntry> _extraValueList;


    public:


		static ExtraValueInfo* GetValueInfoByName(std::string name)
		{
			auto result = std::find_if(_extraValueList->begin(), _extraValueList->end(), [=](auto it) { return Utility::StrCmpI(it->valueName, name); });

			if (_extraValueList->end() == result)
				return nullptr;

			return *result;
		}


		static ExtraValueInfo* GetValueInfoByValue(ValueID i)
		{
			constexpr uint32_t remove = static_cast<uint32_t>(RE::ActorValue::kTotal) + 1;

			if (i < remove || i == (uint32_t)RE::ActorValue::kNone)
				return nullptr;

			i -= remove;

			if (_endTypeIndex[ExtraValueType::Functional] <= i)
				return nullptr;

			return _extraValueList[i];
		}

		static ExtraValueInfo* GetValueInfoByAV(RE::ActorValue av)
		{
			return GetValueInfoByValue(static_cast<uint32_t>(av));
		}

		
		static ExtraValueInfo* GetValueInfoByData(DataID i)
		{
			//Give an aliased version that returns adaptive. Though, this is better, because EVIs aren't requested by name,
			// and this would end up happening for exclusive.
			if (_endTypeIndex[ExtraValueType::Exclusive] <= i)
				return nullptr;

			//Instead do a manual check of the type, for delegates.

			return _extraValueList[i];
		}


		static ExtraValueInfo* GetValueInfoByManifest(DataID i)
		{
			//searches for an value via it's manifest value.
			if (_prevEndTypeIndex[ExtraValueType::Exclusive] <= i)
				return nullptr;

			return _previousExtraValueList[i];
		}



		static std::vector<std::pair<DataID, RegenData>> GetRecoverableValues(RE::Actor* actor)
		{
			auto size = actor->IsPlayerRef() ? _recoverValueList.size() : _exclusiveRecoverIndex;

			std::vector<std::pair<DataID, RegenData>> result(size);

			auto it = _recoverValueList.begin();
			//Wondering if I could just use it.
			std::transform(result.begin(), result.end(), result.begin(), [&](auto pair) { auto i = it++; return std::make_pair(*i, RegenData()); });

			return result;
		}


		static std::vector<std::pair<DataID, SkillData>> GetSkillfulValues(RE::PlayerCharacter* player)
		{
			//This is HELLA temporary. But here's a thought, why not just create the vector I want right here,
			// then have it be copiable from ToggleCollection?

			//Instead of the skill value list, I believe I'd much rather prefer skills be grouped by having functional skills (delegates) be sooner, 
			// and skillful adaptive ones be later, allowing them to be bunched and iterated on.
			// To help with this, I might have a map of vectors that get iterated on and collapsed together.

			std::vector<std::pair<DataID, SkillData>> result(_skillValueList.size());

			auto it = _skillValueList.begin();

			std::transform(result.begin(), result.end(), result.begin(), [&](auto pair) { auto i = it++; return std::make_pair(*i, SkillData()); });

			return result;
		}



		static inline uint32_t GetCount(ExtraValueType type = ExtraValueType::Total)
		{
			switch (type)
			{
			case ExtraValueType::Total:
				return _endTypeIndex[ExtraValueType::Functional];


			case ExtraValueType::Adaptive:
				return _endTypeIndex[ExtraValueType::Adaptive];

			case ExtraValueType::Exclusive:
				return _endTypeIndex[ExtraValueType::Exclusive] - _endTypeIndex[ExtraValueType::Adaptive];

			case ExtraValueType::Functional:
				return _endTypeIndex[ExtraValueType::Functional] - _endTypeIndex[ExtraValueType::Exclusive];

			}

			//Log no valid type was given.

			return 0;
		}

		static inline uint32_t GetCountAV()
		{
			//Make this an API function
			return std::to_underlying(RE::ActorValue::kTotal) + GetCount(ExtraValueType::Total);
		}

		//Rename to get EndIndex
		static inline uint32_t GetCountUpto(ExtraValueType type = ExtraValueType::Functional)
		{
			return _endTypeIndex[type];
		}

		static inline DataID GetBeginIndex(ExtraValueType type)
		{
			switch (type)
			{
			//case ExtraValueType::Adaptive:
				//Adaptive is always the start.

			case ExtraValueType::Exclusive:
				return _endTypeIndex[ExtraValueType::Adaptive];

			case ExtraValueType::Functional:
				return _endTypeIndex[ExtraValueType::Exclusive];

			}
			return 0;
		}

		static inline uint32_t GetManifestCount(ExtraValueType type = ExtraValueType::Total)
		{
			switch (type)
			{
			case ExtraValueType::Total:
				return _prevEndTypeIndex[ExtraValueType::Exclusive];


			case ExtraValueType::Adaptive:
				return _prevEndTypeIndex[ExtraValueType::Adaptive];

			case ExtraValueType::Exclusive:
				return _prevEndTypeIndex[ExtraValueType::Exclusive] - _endTypeIndex[ExtraValueType::Adaptive];

			case ExtraValueType::Functional:
				return 0;

			}

			//Log no valid type was given.

			return 0;
		}


		static void FinishManifest()
		{
			//note for later, force finalization if the number of avs become too large.

			if (_finish) {
				logger::trace("Already finished manifest.");
				return;
			}
			//Used to compare to functional id. Not gonna do that no more.
			//_nextDataID = FunctionalID;

			_finish = true;

			{
				_recoverValueList.shrink_to_fit();
				
				std::vector<ExtraValueInfo*>& adapt_list = __infoTypeMap[0];
				std::vector<ExtraValueInfo*>& excl_list = __infoTypeMap[1];
				std::vector<ExtraValueInfo*>& func_list = __infoTypeMap[2];

				_endTypeIndex[0] = adapt_list.size();
				_endTypeIndex[1] = excl_list.size() + _endTypeIndex[0];
				_endTypeIndex[2] = func_list.size() + _endTypeIndex[1];
				
				_extraValueList->reserve(GetCount());

				_extraValueList->insert(_extraValueList->end(), adapt_list.begin(), adapt_list.end());
				_extraValueList->insert(_extraValueList->end(), excl_list.begin(), excl_list.end());
				_extraValueList->insert(_extraValueList->end(), func_list.begin(), func_list.end());
			}

			delete _infoTypeMap;

			logger::info("EVs finalized, {}/{}/{} ({})",
				GetCount(ExtraValueType::Adaptive),
				GetCount(ExtraValueType::Exclusive),
				GetCount(ExtraValueType::Functional),
				GetCount(ExtraValueType::Total));

			for (auto& entry : *_extraValueList)
			{
				logger::info("ExtraValue '{}', ID {}", entry->GetFixedName(), entry->GetValueID());
			}


			logger::info("Recovery Values {}", _recoverValueList.size());

			for (auto& entry : _recoverValueList)
			{
				logger::info("ID {}", entry);
			}
		}

		static bool Finished()
		{
			return _finish;
		}

    //Membered
	protected:
		//const char* valueName = nullptr;
		std::string valueName{};
		std::string displayName{};
		
		ValueID _valueIDOffset = static_cast<ValueID>(RE::ActorValue::kTotal);  //IE, invalid.

		InfoFlags _flags;



		//This does nothing if not an adaptive or exclusive
		virtual void SetDataID(DataID id) {}
		virtual void Included() {}
	public:
		//temporarily public
		RE::ActorValue _aliasID = RE::ActorValue::kTotal;

	public:
		virtual DataID		GetDataID() = 0;
		ValueID				GetValueID() { return int(RE::ActorValue::kTotal) + 1 + GetBeginIndex(GetType()) + _valueIDOffset; }
		ValueID				GetOffset() { return _valueIDOffset; }
		RE::ActorValue		GetAliasID() const { return _aliasID; }
		//I'd really like to convert these to clean old string_views that share
		std::string_view	GetName() { return valueName; }
		const char*			GetCName() { return valueName.c_str(); }
		std::string_view	GetDisplayName() { return displayName.empty() ? GetName() : displayName; }


		RE::BSFixedString	GetFixedName() { return RE::BSFixedString(valueName.c_str()); }
		RE::ActorValue		GetValueIDAsAV() { return static_cast<RE::ActorValue>(GetValueID()); }

		virtual void LoadFromFile(const FileNode& node, bool legacy);

		virtual InputFlags GetInputFlags() = 0;

		bool AllowsModifier() { return GetInputFlags().set & (ExtraValueInput::Temporary | ExtraValueInput::Permanent); }
		bool AllowsDamage() { return GetInputFlags().set & ExtraValueInput::Damage; }
		bool AllowsSkill() { return IsSkill(); }//Seperate from is skill, needs no skill data to be one.
		bool AllowsAdvance() { return IsSkill(); }//Is advance solely needs to check for skill info. Namely, it also needs to be able to grow.

		virtual bool IsSkill() { return !this ? false : GetSkillInfo(); }
		



		bool IsAdaptive() { return  GetType() == ExtraValueType::Adaptive; }
		bool IsFunctional() { return GetType() == ExtraValueType::Functional; }
		bool IsExclusive() { return GetType() == ExtraValueType::Exclusive; }


		//I would like this to actually be a const variable instead.
		virtual ExtraValueType GetType() const = 0;

		virtual SkillInfo* GetSkillInfo() { return nullptr; }
		virtual DefaultInfo* GetDefaultInfo() { return nullptr; }
		virtual RecoverInfo* GetRecoverInfo() { return nullptr; }

		SkillInfo* FetchSkillInfo()
		{
			if (!this)
				return nullptr;

			return GetSkillInfo();
		}
		DefaultInfo* FetchDefaultInfo()
		{
			if (!this)
				return nullptr;

			return GetDefaultInfo();
		}
		RecoverInfo* FetchRecoverInfo() { if (!this) return nullptr; return GetRecoverInfo(); }


    protected:
		
		static inline void AddExtraValueInfo(ExtraValueInfo* info, ExtraValueType type)
		{
			//Data ID and value id are the same, I just remembered stack overflow is a thing, no need to ignore -1
			bool recovery_value = info->GetRecoverInfo() != nullptr;
			auto skill_value = info->GetSkillInfo();
			

			DataID id;
			switch (type)
			{
			case ExtraValueType::Adaptive:
				//Think this might need it's own function?
				if (recovery_value) {
					_exclusiveRecoverIndex = _recoverValueList.size() + 1;
				}
				id = _nextDataID++;
				goto set_data;

			case ExtraValueType::Exclusive:
				id = _nextDataOffset++;
				set_data:

				if (recovery_value) {
					_recoverValueList.push_back(id);
				}


				if (skill_value && skill_value->isAdvance) {
					_skillValueList.push_back(id);
				}


				info->SetDataID(id);
				break;
			}

			//This whole process isn't needed atm.

			auto& category = __infoTypeMap[type];

			//info->_valueID = _nextValueID++;
			info->_valueIDOffset = category.size();//_nextValueID++;

			//
			//if (int(++_new_nextValueID) == FunctionalID) {
			//++_nextValueID;
			//}



			category.push_back(info);

			logger::info("Info \"{}\" created, data: {}, value offset: {}", info->valueName, info->GetDataID(), info->_valueIDOffset);
		}


    public:




		//Primarily used in adaptive values to determine the base value. With an update value it will return a none zero number.
		// Without being set, storage should be NaN. But I'll do that later.
		virtual float GetExtraValueDefault(RE::Actor* target) { return 0.f; }

        virtual float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) = 0;
		virtual bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) = 0;
		virtual bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) = 0;
		

		inline float GetExtraValue(RE::Actor* target, RE::ACTOR_VALUE_MODIFIER modifier)
		{
			ExtraValueInput input;

			switch (modifier) 
			{
			case RE::ACTOR_VALUE_MODIFIER::kTotal:
				input = ExtraValueInput::Base;
				break;

			case RE::ACTOR_VALUE_MODIFIER::kDamage:
				input = ExtraValueInput::Damage;
				break;

			case RE::ACTOR_VALUE_MODIFIER::kPermanent:
				input = ExtraValueInput::Permanent;
				break;

			case RE::ACTOR_VALUE_MODIFIER::kTemporary:
				input = ExtraValueInput::Temporary;
				break;

			}

			return GetExtraValue(target, input);
		}

        //Not implementing for now, it's effectively set.
		bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, RE::ACTOR_VALUE_MODIFIER modifier, float value) { return false; }
        
        
    };

	using ExtraValueType = ExtraValueInfo::ExtraValueType;


	struct SkillfulData
	{
		SkillInfo* GetSkillInfo() { return _skill; }


		SkillInfo* FetchSkillInfo()
		{
			if (!this)
				return nullptr;

			return GetSkillInfo();
		}

		SkillInfo& ObtainSkillInfo()
		{
			if (!_skill)
				_skill = new SkillInfo();

			return *_skill;
		}


		void LoadFromFile(const FileNode& node, bool is_legacy);



		SkillInfo* _skill{};//The skill data isn't stored here, just the information for it.


	};

	struct AdaptiveData : public SkillfulData
	{

		float GetExtraValue(ExtraValueInfo* info, RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) ;

		bool SetExtraValue(ExtraValueInfo* info, RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) ;

		bool ModExtraValue(ExtraValueInfo* info, RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) ;

		void LoadFromFile(const FileNode& node, bool is_legacy);



		float GetExtraValueDefault(RE::Actor* target) 
		{ 
			if (GetSkillInfo() != nullptr)
			{
				static RE::Setting* iAVDSkillStart = RE::GameSettingCollection::GetSingleton()->GetSetting("iAVDSkillStart");
				return iAVDSkillStart->GetSInt();
			}
			
			auto def = GetDefaultInfo(); 
			return !def ? 0 : def->defaultFunction(target); 
		}

		
		DefaultInfo* GetDefaultInfo() { return _default; }
		RecoverInfo* GetRecoverInfo() { return _recovery; }

		DefaultInfo* ObtainDefaultInfo()
		{
			if (!_default)
				_default = new DefaultInfo();

			return _default;
		}
		RecoverInfo* ObtainRecoverInfo()
		{
			if (!_recovery)
				_recovery = new RecoverInfo();

			return _recovery;
		}



		DefaultInfo* FetchDefaultInfo()
		{
			if (!this)
				return nullptr;

			return GetDefaultInfo();
		}
		RecoverInfo* FetchRecoverInfo() { if (!this) return nullptr; return GetRecoverInfo(); }




		inline AdaptiveData* adapt() noexcept
		{
			return this;
		}



		
		DefaultInfo* _default{};
		RecoverInfo* _recovery{};

		DataID _dataID = -1;






	};



    class AdaptiveValueInfo : public ExtraValueInfo, public AdaptiveData
    {
		void SetDataID(DataID id) override { _dataID = id; }
    public:
		DataID GetDataID() override { return  _dataID; }
		
		ExtraValueType GetType() const override { return ExtraValueType::Adaptive; }//these will be exclusive, I'm gonna use a flag for that.
		


		InputFlags GetInputFlags() override { return InputFlags::All(); }



		SkillInfo* GetSkillInfo() override { return adapt()->GetSkillInfo(); }
		DefaultInfo* GetDefaultInfo() override { return adapt()->GetDefaultInfo(); }
		RecoverInfo* GetRecoverInfo() override { return adapt()->GetRecoverInfo(); }

		float GetExtraValueDefault(RE::Actor* target) override { return adapt()->GetExtraValueDefault(target); }


		float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override
		{
			return adapt()->GetExtraValue(this, target, value_types);
		}

		bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			return adapt()->SetExtraValue(this, target, value, modifier);
		}

		bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			return adapt()->ModExtraValue(this, target, aggressor, value, modifier);
		}

        void Update()
        {
            //The function attempts to update
        }


		void LoadFromFile(const FileNode& node, bool legacy) override;

    };
	NOT_ABSTRACT(AdaptiveValueInfo);


	template <class Type>
	using ModifierArray = std::array<Type, ActorValueModifier::kTotal + 1>;


	struct FunctionalData
	{

		float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All)
		{
			//Needs to use _get, this also needs to wait for implementation

			if (value_types == ExtraValueInput::None)
				return 0;

			if (!(_getFlags & value_types))
				return 0;


			float bas = _get[ActorValueModifier::kTotal] && !!(value_types & ExtraValueInput::Base) ?
				_get[ActorValueModifier::kTotal](target) : 0;

			float prm = _get[ActorValueModifier::kPermanent] && !!(value_types & ExtraValueInput::Permanent) ?
				_get[ActorValueModifier::kPermanent](target) : 0;

			float tmp = _get[ActorValueModifier::kTemporary] && !!(value_types & ExtraValueInput::Temporary) ?
				_get[ActorValueModifier::kTemporary](target) : 0;

			float dmg = _get[ActorValueModifier::kDamage] && !!(value_types & ExtraValueInput::Damage) ?
				_get[ActorValueModifier::kDamage](target) : 0;

			dmg = fmin(dmg, 0);

			//logger::info("{}/{}/{}/{}", bas, prm, tmp, dmg);

			return bas + prm + tmp + dmg;
		}

		bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier)
		{
			if (!_set[modifier])
				return false;

			//float from = GetExtraValue(target, ModifierToValueInput(modifier));
			float from = modifier == RE::ActorValueModifier::kTotal ? NAN : 0;
			_set[modifier](target, nullptr, from, value);

			return true;
		}

		bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier)
		{

			if (!_set[modifier])
				return false;

			float current = GetExtraValue(target, ModifierToValueInput(modifier));

			_set[modifier](target, aggressor, current, value + current);

			return true;
		}



		ExtraValueInput GetFlags() const { return _getFlags; }
		ExtraValueInput SetFlags() const { return _setFlags; }


		//I want this to load specifically the below
		void LoadFromFile(const FileNode& node, bool legacy);




		ExtraValueInput _getFlags = ExtraValueInput::None;
		ExtraValueInput _setFlags = ExtraValueInput::None;

		ModifierArray<ValueFormula>	_get{};
		ModifierArray<SetFormula>	_set{};



		FunctionalData* function()
		{
			return this;
		}
	};

    class FunctionalValueInfo : public ExtraValueInfo, public FunctionalData
	{
        
		
        //Notice, use of functional value without a set function is prohibited.
        // The set function must have a few things.
        // The function must have a string so it knows what AV is being edited, allowing reuse of a single function.
        // the value it's to set if base, mod if not.
        // And finally the modifier being changed, or total if base.
        // In total, it looks like this
        //void(RE::Actor*, std::string, RE::ActorValueModifier, float);


		void HandleSetExport(RE::ACTOR_VALUE_MODIFIER modifier, RE::Actor* target, RE::Actor* cause, float from, float to)
		{//Rename this
			_set[modifier](target, cause, from, to);
		}


    public:
        DataID GetDataID() override { return FunctionalID; }


		ExtraValueType GetType() const override { return ExtraValueType::Functional; }

		//This isn't quite right though//Needs more than just damage.

		InputFlags GetInputFlags() override { return InputFlags{ GetFlags(), SetFlags() }; }


        float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override
		{
			return function()->GetExtraValue(target, value_types);
		}

		bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			return function()->SetExtraValue(target, value, modifier);
        }

		bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			return function()->ModExtraValue(target, aggressor, value, modifier);
		}


		bool AddSetFunction__(std::string func, ActorValueModifier mod, std::vector<std::string> context)
		{



			_set[mod] = SetFormula::Create("cause", "from", "to", func);

			if (!_set[mod])
				return false;

			_setFlags |= ModifierToValueInput(mod);

			return true;
		}


		void Included() override
		{
			//logger::info("Functional value {} created. Get: {:04B}, Set: {:04B}", GetName(), (int)_getFlags, (int)_setFlags);
		}

		void LoadFromFile(const FileNode& node, bool legacy) override;
	};
	NOT_ABSTRACT(FunctionalValueInfo);
	

	class ExclusiveValueInfo : public ExtraValueInfo, public FunctionalData, public AdaptiveData
	{
		void SetDataID(DataID id) override { _dataID = id; }

	public:
		DataID GetDataID() override { return GetCount(ExtraValueType::Adaptive) + _dataID; }

		ExtraValueType GetType() const override { return ExtraValueType::Exclusive; }

		//This isn't quite right though//Needs more than just damage.
		
		InputFlags GetInputFlags() override { return InputFlags{ GetFlags(), SetFlags() }; }



		float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override
		{
			if (target && target->IsPlayerRef() == true)
				return adapt()->GetExtraValue(this, target, value_types);
			else
				return function()->GetExtraValue(target, value_types);
		}

		bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			if (target && target->IsPlayerRef() == true)
				return adapt()->SetExtraValue(this, target, value, modifier);
			else
				return function()->SetExtraValue(target, value, modifier);
		}

		virtual bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			if (target && target->IsPlayerRef() == true)
				return adapt()->ModExtraValue(this, target, aggressor, value, modifier);
			else
				return function()->ModExtraValue(target, aggressor, value, modifier);
		}



		SkillInfo* GetSkillInfo() override { return adapt()->GetSkillInfo(); }
		DefaultInfo* GetDefaultInfo() override { return adapt()->GetDefaultInfo(); }
		RecoverInfo* GetRecoverInfo() override { return adapt()->GetRecoverInfo(); }

		float GetExtraValueDefault(RE::Actor* target) override { return target->IsPlayerRef() ? adapt()->GetExtraValueDefault(target) : 0.0f; }



		void Included() override
		{
			//logger::info("Functional value {} created. Get: {:04B}, Set: {:04B}", GetName(), (int)_getFlags, (int)_setFlags);
		}

		void FunctionalValueInfo_(std::string& name, ModifierArray<std::string> get_strings = {}, ModifierArray<std::string> set_strings = {})// : ExtraValueInfo{ name }
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.

			//Make these a function.

			for (ActorValueModifier mod = ActorValueModifier::kPermanent; mod <= ActorValueModifier::kTotal; mod++)
			{
				if (get_strings[mod] != "") {
					logger::debug("making {}", get_strings[mod]);
					_get[mod] = ValueFormula::Create(get_strings[mod]);
					_getFlags |= ModifierToValueInput(mod);
				}

				if (mod == ActorValueModifier::kTotal)
					break;
			}

			for (ActorValueModifier mod = ActorValueModifier::kPermanent; mod <= ActorValueModifier::kTotal; mod++)
			{
				//For there to be a set there, there must be a coresponding get function
				if (!(ModifierToValueInput(mod) & _getFlags)) {
					logger::info("corresponding get function must exist for set function.");
					continue;
				}

				//For there to be a set, there must be a get. Note that.
				if (set_strings[mod] == "")
					continue;

				_set[mod] = SetFormula::Create("cause", "from", "to", set_strings[mod]);

				_setFlags |= ModifierToValueInput(mod);


			}


			AddExtraValueInfo(this, ExtraValueType::Functional);


			//logger::info("Functional value {} created. Get: {:04B}, Set: {:04B}", name, (int)_getFlags, (int)_setFlags);

			//Should do nothing atm.
		}

		void LoadFromFile(const FileNode& node, bool legacy) override;
	};
	NOT_ABSTRACT(FunctionalValueInfo);

    //The Psuedo namespace is erased when the time has come.
	namespace Psuedo
	{
		inline float GetExtraValue(RE::Actor* target, std::string ev_name, ExtraValueInput value_types = ExtraValueInput::All)
		{
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (!info)
				logger::error("Cannot find info at {}", ev_name);
			return info ? info->GetExtraValue(target, value_types) : 0;
		}

		inline float GetExtraValue(RE::Actor* target, std::string ev_name, RE::ACTOR_VALUE_MODIFIER modifier)
		{
			ExtraValueInput ev_mod = ExtraValueInput::None;

			switch (modifier) {
			case RE::ACTOR_VALUE_MODIFIER::kTotal:
				ev_mod = ExtraValueInput::Base;
				break;

			case RE::ACTOR_VALUE_MODIFIER::kPermanent:
				ev_mod = ExtraValueInput::Permanent;
				break;

			case RE::ACTOR_VALUE_MODIFIER::kTemporary:
				ev_mod = ExtraValueInput::Temporary;
				break;

			case RE::ACTOR_VALUE_MODIFIER::kDamage:
				ev_mod = ExtraValueInput::Damage;
				break;

			default:
				return 0;
			}

			return GetExtraValue(target, ev_name, ev_mod);
		}

		

		inline bool SetExtraValue(RE::Actor* target, std::string ev_name, float value, RE::ACTOR_VALUE_MODIFIER modifier = RE::ACTOR_VALUE_MODIFIER::kTotal)
		{
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (info)
				info->SetExtraValue(target, value, modifier);
			
			return info;
		}

		inline bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, std::string ev_name, float value, RE::ACTOR_VALUE_MODIFIER modifier = RE::ACTOR_VALUE_MODIFIER::kTotal)
		{
			if (!value)
				logger::warn("Modification is empty");
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (info)
				info->ModExtraValue(target, aggressor, value, modifier);

			return info;
		}



		inline float GetEitherValue(RE::Actor* target, std::string ev_name, ExtraValueInput value_types = ExtraValueInput::All)
		{
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (info)
				return info->GetExtraValue(target, value_types);
			
			RE::ActorValue av = RE::ActorValueList::GetSingleton()->LookupActorValueByName(ev_name);

			if (av == RE::ActorValue::kNone)
				return NAN;

			float base = !!(value_types & ExtraValueInput::Base) ? target->AsActorValueOwner()->GetBaseActorValue(av) : 0;
			float perm = !!(value_types & ExtraValueInput::Permanent) ? target->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kPermanent, av) : 0;
			float temp = !!(value_types & ExtraValueInput::Temporary) ? target->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av) : 0;
			float dmg = !!(value_types & ExtraValueInput::Damage) ? target->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kDamage, av) : 0;
			
			return base + perm + temp + dmg;
		}

		/*
		


		inline bool SetEitherValue(RE::Actor* target, std::string ev_name, float value, RE::ACTOR_VALUE_MODIFIER modifier = RE::ACTOR_VALUE_MODIFIER::kTotal)
		{
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (info)
				info->SetExtraValue(target, value, modifier);

			return info;
		}

		inline bool ModEitherValue(RE::Actor* target, RE::ActorValue av, float value, RE::ACTOR_VALUE_MODIFIER modifier = RE::ACTOR_VALUE_MODIFIER::kTotal)
		{
			if (!value)
				logger::warn("Modification is empty");
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (info)
				info->ModExtraValue(target, value, modifier);

			return info;
		}
		//*/
	}
}


