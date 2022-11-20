#pragma once

#include "Types.h"
#include "Utility.h"

namespace AVG
{
    class IReadyArthmetic;

       
	class AdaptiveValueInfo;
	class FunctionalValueInfo;

    //All of these will likely be changed to info at some later point.

	//This is the info needed to treat this av like a skill, growing when given experience. Player only obviously.
	//Unimplmented obviously
	struct SkillData
	{
		//float experience;
        //float levelUpAt;
	};

    struct UpdateData
    {
		//rate is required.
		//If value to update is zero, no timed update data will be used, and instead it will only update upon load.
		IReadyArthmetic* updateRate = nullptr; 
        float valueToUpdate = 0.f;
    };


    struct RecoveryData
    {
		float tmp_recDelay{};
		float tmp_recRate{};
		
		//While delay is not required, the rate is similarly required.
		IReadyArthmetic* recoveryDelay = nullptr;  //Null will mean there is no delay, if recovery exists.
		IReadyArthmetic* recoveryRate = nullptr;  //Null here means no recovery.
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
		DerivedAffected = 1 << 0,	//Do plugins derived inherit the av aliases of this EVI?

	};

	//Along with this sort of information, I believe I will likely need a class called loading info.
	// This will store all the data that I don't want taking up permenant space, so I can just delete it later. Maybe, I can unionize it.
	// Anywho, this load data can store the pure string data that I want to carry onto, because I'm going to need something like that
	// for functions to be coroutines to be loaded first.

    class ExtraValueInfo 
    {
    //static
    private:
        static inline DataID _nextDataID = 0;
        static inline ValueID _nextValueID = static_cast<ValueID>(RE::ActorValue::kTotal) + 1;

        static inline DataID _adaptiveCount = 0;

        //static inline std::vector<string> _currSerialManifest;
        static inline std::vector<std::string> _prevSerialManifest;

        // Should require validation. This is also the manifest now. When serialized, we just serialize the strings in
        // sequence.
        static inline std::vector<AdaptiveValueInfo*> _adaptiveInfoList;


		

        //This may use a string hash sometime in the future.
        static inline std::map<std::string, ExtraValueInfo*> _infoTable;

        //This will need to be able to find based on their value.

        //I'll need to find a way to get this to find on 3 fronts, ValueID, DataID, and string.

	protected:
		//This method will be kinda temporary.
		static inline std::vector<std::pair<DataID, float>> _recoveryInfoList{};

		//This method is just to make sure, I don't even think it's needed.
		static inline uint32_t _numRecover = 0;
    public:
        
        static constexpr DataID FunctionalID = 0xFFFFFFFF;

        static ExtraValueInfo* GetValueInfoByName(std::string name) 
        { 
			//note, I'm aware of how shit this is. But I'm in the middle of a transition.

			auto result = std::find_if(_infoTable.begin(), _infoTable.end(), [=](auto it) { return Utility::StrCmpI(it.first, name); });

			if (_infoTable.end() == result)
				return nullptr;

			return result->second;

			

			// Use std find in order to name match.
            auto result_o = _infoTable.find(name); 
            return result_o == _infoTable.end() ? nullptr : result_o->second; 
        }

		//These 2 need checks to just straight up ignore invalid values.
        static AdaptiveValueInfo* GetValueInfoByValue(size_t i)
		{
			constexpr uint32_t add = static_cast<uint32_t>(RE::ActorValue::kTotal) + 1;

			if (i < add)
				return nullptr;

            //Currently, doesn't do as it should, this is supposed to be the list associated with the actor value enum
			return _adaptiveCount + add <= i ? nullptr : _adaptiveInfoList[i - add];
		}

        static AdaptiveValueInfo* GetValueInfoByData(size_t i) 
        { 
            return _adaptiveCount <= i ? nullptr : _adaptiveInfoList[i]; 
        }

		
        static std::vector<std::pair<DataID, float>> GetRecoverableValues()
		{
			//This is HELLA temporary. But here's a thought, why not just create the vector I want right here,
			// then have it be copiable from ToggleCollection?
			return _recoveryInfoList;
		}


        static uint32_t GetAdaptiveCount()
		{
			//if this returns functional, its value is not yet valid.

			if (_nextDataID != FunctionalID)
				return FunctionalID;

			return _adaptiveCount;
		}

		//Sloppy, but I need it now.
		static uint32_t GetEVCount() { return _infoTable.size(); }

        //Is public for now, not sure what it'll look like later.
		static void FinishManifest()
		{
			//note for later, force finalization if the number of avs become too large.

			if (_nextDataID == FunctionalID) {
				logger::trace("Already finished manifest");
				return;
			}
			_adaptiveCount = _nextDataID;

			_nextDataID = FunctionalID;

			auto const_size = _adaptiveInfoList.size();

			//_currSerialManifest.resize(const_size);

			_adaptiveInfoList.resize(const_size);

			_recoveryInfoList.resize(_numRecover);

			//make the creation here.

			logger::info("finalized, {} vs {}", const_size, _adaptiveCount);
		}


    //Membered
	protected:
		//const char* valueName = nullptr;
		std::string valueName{};
		
		//Might remake into a list of TESFile's to keep them on short hand.
		std::vector<std::string> masterFiles;
		

		const char* lynchpin = nullptr;

		ValueID _valueID = static_cast<ValueID>(RE::ActorValue::kTotal);  //IE, invalid.

		InfoFlags _flags;

	public:
		virtual DataID  GetDataID() = 0;
		ValueID         GetValueID() { return _valueID; }

		virtual bool IsAdaptive() { return false; }
		virtual bool IsFunctional() { return false; }

        virtual SkillData* GetSkillData() { return nullptr; }
        virtual UpdateData* GetUpdateData() { return nullptr; }
		virtual RecoveryData* GetRecoveryData() { return nullptr; }

    protected:
		DataID PushAdaptiveInfo(AdaptiveValueInfo* a_this)
		{
			//Ideally, if its right next to functional, it is not allowed to add anything to that slot (because it wouldn't)
			// even be accessible
			if (_nextDataID >= FunctionalID - 1)
				throw nullptr;

			//_currSerialManifest.push_back(name);
			_adaptiveInfoList.push_back(a_this);

			DataID dataID = _nextDataID;
			_nextDataID++;

            //PushInfo();

            return dataID;
		}

        void PushInfo()
		{
			std::string name = valueName;
			
			
			
			_infoTable[name] = this;

			_valueID = _nextValueID;
			_nextValueID++;

			logger::info("Info \"{}\" created, data: {}, value: {}", name, "???", GetValueID());
		}

    public:




        virtual float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) = 0;
		virtual bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) = 0;
		virtual bool ModExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) = 0;
		

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
		bool ModExtraValue(RE::Actor* target, RE::ACTOR_VALUE_MODIFIER modifier, float value) { return false; }
        
        
        ExtraValueInfo(std::string& name)
        {
            //I would like this to have the job if appointing this to a list. It also copies the string given.
			//For now, I think I'd like to make it a string, just for ease of use.
			valueName = name;
			PushInfo();
        }
    };

    class AdaptiveValueInfo : public ExtraValueInfo
    {
		SkillData* _skill{};//The skill data isn't stored here, just the information for it.
		UpdateData* _update{};
		RecoveryData* _recovery{};

        DataID _dataID{};

    public:
		DataID GetDataID() override { return _dataID; }
		bool IsAdaptive() override { return true; }

        SkillData* GetSkillData() override { return _skill; }
		UpdateData* GetUpdateData() override { return _update; }
		RecoveryData* GetRecoveryData() override { return _recovery; }

        float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override;

        bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override;

		bool ModExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override;

        void Update()
        {
            //The function attempts to update
        }

        AdaptiveValueInfo(std::string& name) : ExtraValueInfo{ name }
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.

            _dataID = PushAdaptiveInfo(this);
			logger::info("{}", _dataID);
		}
		
		//Note, this is a temp solution.
        AdaptiveValueInfo(std::string& name, float delay, float rate) : ExtraValueInfo{ name }
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.

            _dataID = PushAdaptiveInfo(this);
			_recovery = new RecoveryData();
			_recovery->tmp_recDelay = delay;
			_recovery->tmp_recRate = rate;

			_recoveryInfoList.push_back(std::make_pair(_dataID, 0.f));  //Kinda sloppy, but less sloppy than the previous so you know.
			_numRecover++;
			logger::info("{}", _dataID);
		}


    };

    
    class FunctionalValueInfo : public ExtraValueInfo
	{
        using ExportValue = void (RE::Actor*, std::string, RE::ACTOR_VALUE_MODIFIER, float);

        //Notice, use of functioanl value without a set function is prohibited.
        // The set function must have a few things.
        // The function must have a string so it knows what AV is being edited, allowing reuse of a single function.
        // the value it's to set if base, mod if not.
        // And finally the modifier being changed, or total if base.
        // In total, it looks like this
        //void(RE::Actor*, std::string, RE::ActorValueModifier, float);


        IReadyArthmetic*    _get{};
		ExportValue*        _set{};

    public:
        DataID GetDataID() override { return FunctionalID; }
		bool IsFunctional() { return true; }

        float GetExtraValue(RE::Actor* target, ExtraValueInput value_types = ExtraValueInput::All) override
		{
			//Needs to use _get, this also needs to wait for implementation.
			return 0;
		}

		bool SetExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier) override
		{
			if (!_set)
				return false;
		     

            return true;
        }

		virtual bool ModExtraValue(RE::Actor* target, float value, RE::ACTOR_VALUE_MODIFIER modifier)
		{
			if (!_set || !_get)
				return false;

			return true;
		}

        FunctionalValueInfo(std::string& name) : ExtraValueInfo{ name }
		{
			//I would like this to have the job if appointing this to a list. It also copies the string given.

			//Should do nothing atm.
		}
	};

	
	static float temp_player_delay = 0;
		

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

		inline bool ModExtraValue(RE::Actor* target, std::string ev_name, float value, RE::ACTOR_VALUE_MODIFIER modifier = RE::ACTOR_VALUE_MODIFIER::kTotal)
		{
			if (!value)
				logger::warn("Modification is empty");
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(ev_name);

			if (info)
				info->ModExtraValue(target, value, modifier);

			return info;
		}
	}
}


