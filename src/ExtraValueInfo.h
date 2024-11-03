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
	
	
	struct SkillInfo
	{
		float useMult;        // 00
		float offsetMult;     // 04
		float improveMult;    // 08
		float improveOffset;  // 0C
	};

	struct SkillData
	{
		float			level;
		float			xp;
		float			levelThreshold;
		std::uint32_t	legendaryLevels;
	};
	
	struct SkillData_original
	{
		//So this would be the storage stuff,
		struct PlayerSkills
		{
		public:
			struct Data
			{
			public:
				struct Skills
				{
					enum Skill : std::uint32_t
					{
						kOneHanded = 0,
						kTwoHanded = 1,
						kArchery = 2,
						kBlock = 3,
						kSmithing = 4,
						kHeavyArmor = 5,
						kLightArmor = 6,
						kPickpocket = 7,
						kLockpicking = 8,
						kSneak = 9,
						kAlchemy = 10,
						kSpeech = 11,
						kAlteration = 12,
						kConjuration = 13,
						kDestruction = 14,
						kIllusion = 15,
						kRestoration = 16,
						kEnchanting = 17,
						kTotal
					};
				};
				using Skill = Skills::Skill;

				struct SkillData
				{
				public:
					// members
					float level;           // 0
					float xp;              // 4
					float levelThreshold;  // 8
				};
				static_assert(sizeof(SkillData) == 0xC);

				// members
				float         xp;                              // 000
				float         levelThreshold;                  // 004
				SkillData     skills[Skill::kTotal];           // 008
				std::uint32_t legendaryLevels[Skill::kTotal];  // 0E0
			};
			static_assert(sizeof(Data) == 0x128);

			void AdvanceLevel(bool a_addThreshold);

			// members
			Data* data;  // 0
		};
		//and this would be the EVI stuff
		struct Base  // AVSK
		{
			
		};

		//float experience;
        //float levelUpAt;
	};

	using ValueFormula = LEX::Formula<float(RE::Actor::*)()>;
	using SetFormula = LEX::Formula<void(RE::Actor::*)(RE::Actor*, float, float)>;


	//This is functionally default data. Update data isn't something I'll want to implement for a while, and since this doesn't serialize I'm comfortable
	// doing it later.
    struct DefaultData
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




    struct RecoveryData
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


    //RELOCATE
	constexpr uint32_t sign_bit = 0x80000000;
    
    /*
    //These aren't gonna be used, I forgot to account for functional so rip.
    DataID ValueToData(ValueID value)
	{
        //always reinterpret cast this in here, if the thing is unsigned I need to make something that will convert it
        //sign_bit is invalid, none is used for actor value so you know.
		if (value <= static_cast<DataID>(RE::ActorValue::kTotal) || value == sign_bit)
			return ExtraValueInfo::FunctionalID;

        int extra = value > sign_bit ? 2 : 1;

        return value - (static_cast<DataID>(RE::ActorValue::kTotal) + extra);
	}

    ValueID DataToValue(DataID data)
	{
		ValueID raw_value = data + static_cast<ValueID>(RE::ActorValue::kTotal);

        int extra = raw_value == sign_bit ? 2 : 1;

        return raw_value + extra;
	}
    //*/


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


		static void Create(std::string_view name, ExtraValueType type, const FileNode& node);

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

			if (i < remove)
				return nullptr;

			i -= remove;

			if (_endTypeIndex[ExtraValueType::Functional] <= i)
				return nullptr;

			return _extraValueList[i];
		}

		
		static ExtraValueInfo* GetValueInfoByData(DataID i)
		{
			//Give an aliased version that returns adaptive. Though, this is better, because EVIs aren't requested by name,
			// and this would end up happening for exclusive.
			if (_endTypeIndex[ExtraValueType::Exclusive] <= i)
				return nullptr;

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


		static std::map<DataID, SkillData> GetSkillfulValues(RE::PlayerCharacter* player)
		{
			//This is HELLA temporary. But here's a thought, why not just create the vector I want right here,
			// then have it be copiable from ToggleCollection?
			return {};
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

			if (_nextDataID == FunctionalID) {
				logger::trace("Already finished manifest.");
				return;
			}
			
			_nextDataID = FunctionalID;

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


    //Membered
	protected:
		//const char* valueName = nullptr;
		std::string valueName{};
		
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
		//I'd really like to convert these to clean old string_views that share
		std::string_view	GetName() { return valueName; }
		const char*			GetCName() { return valueName.c_str(); }
		RE::BSFixedString	GetFixedName() { return RE::BSFixedString(valueName.c_str()); }
		RE::ActorValue		GetValueIDAsAV() { return static_cast<RE::ActorValue>(GetValueID()); }

		virtual void LoadFromFile(const FileNode& node) = 0;

		virtual bool AllowsSetting() { return false; }

		virtual bool IsAdaptive() { return false; }
		virtual bool IsFunctional() { return false; }
		virtual bool IsExclusive() { return false; }

		//I would like this to actually be a const variable instead.
		virtual ExtraValueType GetType() { return ExtraValueType::Total; }

		virtual SkillData* GetSkillData() { return nullptr; }
		virtual DefaultData* GetDefaultData() { return nullptr; }
		virtual RecoveryData* GetRecoveryData() { return nullptr; }

		virtual SkillData* MakeSkillData() { return nullptr; }
		virtual DefaultData* MakeDefaultData() { return nullptr; }
		virtual RecoveryData* MakeRecoveryData() { return nullptr; }


		SkillData* GetSkillDataSafe()
		{
			if (!this)
				return nullptr;

			return GetSkillData();
		}
		DefaultData* GetDefaultDataSafe()
		{
			if (!this)
				return nullptr;

			return GetDefaultData();
		}
		RecoveryData* GetRecoveryDataSafe() { if (!this) return nullptr; return GetRecoveryData(); }

		SkillData* MakeSkillDataSafe() 
		{ 
			if (!this)
				return nullptr;

			return MakeSkillData(); 
		}
		DefaultData* MakeDefaultDataSafe()
		{
			if (!this)
				return nullptr;

			return MakeDefaultData();
		}
		RecoveryData* MakeRecoveryDataSafe()
		{
			if (!this)
				return nullptr;

			return MakeRecoveryData();
		}



    protected:
		
		static inline void AddExtraValueInfo(ExtraValueInfo* info, ExtraValueType type)
		{
			//Data ID and value id are the same, I just remembered stack overflow is a thing, no need to ignore -1
			bool recovery_value = info->GetRecoveryData() != nullptr;
			switch (type)
			{
			case ExtraValueType::Adaptive:
				//Think this might need it's own function?
				if (recovery_value) {
					_exclusiveRecoverIndex = _recoverValueList.size() + 1;
				}
			case ExtraValueType::Exclusive:
				if (recovery_value) {
					_recoverValueList.push_back(_nextDataID);
				}

				info->SetDataID(_nextDataID++);
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





    class AdaptiveValueInfo : public ExtraValueInfo
    {
		SkillData* _skill{};//The skill data isn't stored here, just the information for it.
		DefaultData* _default{};
		RecoveryData* _recovery{};

        DataID _dataID{};

		void SetDataID(DataID id) override { _dataID = id; }
    public:
		DataID GetDataID() override { return _dataID; }
		bool IsAdaptive() override { return true; }
		ExtraValueType GetType() override { return ExtraValueType::Adaptive; }//these will be exclusive, I'm gonna use a flag for that.
		virtual bool AllowsSetting() { return true; }


		SkillData* GetSkillData() override { return _skill; }
		DefaultData* GetDefaultData() override { return _default; }
		RecoveryData* GetRecoveryData() override { return _recovery; }

		float GetExtraValueDefault(RE::Actor* target) override { auto def = GetDefaultData(); return !def ? 0 : def->defaultFunction(target); }

		SkillData* MakeSkillData() override
		{
			if (!_skill)
				_skill = new SkillData();

			return _skill;
		}

		DefaultData* MakeDefaultData() override
		{
			if (!_default)
				_default = new DefaultData();

			return _default;
		}
		RecoveryData* MakeRecoveryData() override
		{
			if (!_recovery)
				_recovery = new RecoveryData();

			return _recovery;
		}


        float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override;

        bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override;

		bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) override;

        void Update()
        {
            //The function attempts to update
        }

        void AdaptiveValueInfo_(std::string& name)// : ExtraValueInfo{ name }
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.
            AddExtraValueInfo(this, ExtraValueType::Adaptive);
			//logger::info("{}", _dataID);
		}
		
		//Note, this is a temp solution.
        void AdaptiveValueInfo_(std::string& name, std::string& delay, std::string& rate)// : ExtraValueInfo{ name }
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.

			_recovery = new RecoveryData();
			//_recovery->tmp_recDelay = delay;
			//_recovery->tmp_recRate = rate;
			_recovery->recoveryDelay = ValueFormula::Create(delay);//, "ActorValueGenerator::Commons");
			_recovery->recoveryRate = ValueFormula::Create(delay);

			//This should be done EXTERNALLY, that way it's easier to make exclusive.
			AddExtraValueInfo(this, ExtraValueType::Adaptive);

			//logger::info("{}", _dataID);
		}

		void LoadFromFile(const FileNode& node) override;

    };

	template <class Type>
	using ModifierArray = std::array<Type, ActorValueModifier::kTotal + 1>;

	using ExportSetData = ActorValueGeneratorAPI::ExportSetData;
	using ExportFunction = ActorValueGeneratorAPI::ExportFunction;


	inline void DamageHealth(RE::Actor* target, RE::Actor* aggressor, std::string original_av, std::vector<std::string>, RE::ACTOR_VALUE_MODIFIER modifier, float value)
	{
		if (target)
		{
			target->AsActorValueOwner()->RestoreActorValue(modifier, RE::ActorValue::kHealth, value);
		}
	}


	//New one of these, maybe, send papyrus event?
	inline void AffectActorValue(const ExportSetData& data)
	{
		size_t ex_size = data.export_context.size();

		logger::debug("??? {} {} {}", !data.target, ex_size , isnan(data.from));

		if (!ex_size || isnan(data.from) || !data.target)
			return;
		logger::debug("CONTEXT {}", data.export_context[0]);


		ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(data.export_context[0]);

		RE::ActorValue av = info ? info->GetValueIDAsAV() : Utility::StringToActorValue(data.export_context[0]);;

		if (av == RE::ActorValue::kTotal)
			return;

		logger::debug("AV {}", (int)av);


		float mod_value = data.to - data.from;

		float mult_value = 1.f;

		if (ex_size > 1) {
			try{
				mult_value = std::stof(data.export_context[1]);
			}
			catch (const std::exception&){}
		}



		if (data.av_modifier == ActorValueModifier::kTotal){
			float base = data.target->AsActorValueOwner()->GetBaseActorValue(av);
			data.target->AsActorValueOwner()->SetBaseActorValue(av, (mod_value * mult_value) + base);
		}
		else{
			data.target->AsActorValueOwner()->RestoreActorValue(data.av_modifier, av, mult_value * mod_value);
		}

	}


    class FunctionalValueInfo : public ExtraValueInfo
	{
        
		
        //Notice, use of functional value without a set function is prohibited.
        // The set function must have a few things.
        // The function must have a string so it knows what AV is being edited, allowing reuse of a single function.
        // the value it's to set if base, mod if not.
        // And finally the modifier being changed, or total if base.
        // In total, it looks like this
        //void(RE::Actor*, std::string, RE::ActorValueModifier, float);

		//Easy way to tell which ones this actually has.
		ExtraValueInput _getFlags = ExtraValueInput::None;
		ExtraValueInput _setFlags = ExtraValueInput::None;

		ModifierArray<ValueFormula>	_get{};
		ModifierArray<SetFormula>	_set{};

		void HandleSetExport(RE::ACTOR_VALUE_MODIFIER modifier, RE::Actor* target, RE::Actor* cause, float from, float to)
		{//Rename this
			_set[modifier](target, cause, from, to);
		}


    public:
        DataID GetDataID() override { return FunctionalID; }
		bool IsFunctional() { return true; }
		ExtraValueType GetType() override { return ExtraValueType::Functional; }


		ExtraValueInput GetFlags() const { return _getFlags; }
		ExtraValueInput SetFlags() const { return _setFlags; }

		//This isn't quite right though//Needs more than just damage.
		virtual bool AllowsSetting() override { return SetFlags(); }

        float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override
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

		bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			if (!_set[modifier])
				return false;

			float from = GetExtraValue(target, ModifierToValueInput(modifier));

			_set[modifier](target, nullptr, from, value);

            return true;
        }

		bool AddSetFunction(std::string func, ActorValueModifier mod, std::vector<std::string> context)
		{
			


			_set[mod] = SetFormula::Create("cause", "from", "to", func);

			if (!_set[mod])
				return false;

			_setFlags |= ModifierToValueInput(mod);

			return true;
		}

		virtual bool ModExtraValue(RE::Actor* target, RE::Actor* aggressor, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{

			if (!_set[modifier])
				return false;

			float from = GetExtraValue(target, ModifierToValueInput(modifier));

			_set[modifier](target, aggressor, from, value);

            return true;
		}

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

		void LoadFromFile(const FileNode& node) override;
	};

	

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


