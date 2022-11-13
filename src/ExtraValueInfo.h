#pragma once

#include "Types.h"

namespace AVG
{
    class IReadyArthmetic;

       
	class AdaptiveValueInfo;
	class FunctionalValueInfo;

    //All of these will likely be changed to info at some later point.

	//This is the info needed to treat this av like a skill, growing when given experience. Player only obviously.
	//Unimplmented obviously
	class SkillData
	{
		//float experience;
        //float levelUpAt;
	};

    class UpdateData
    {
		IReadyArthmetic* updateCalc = nullptr; 
        EVTick tickToUpdate = 0;
    };


    class RecoveryData
    {
		IReadyArthmetic* recoveryDelay = nullptr;  //Null will mean there is no delay, if recovery exists.
		IReadyArthmetic* recoveryValue = nullptr;  //Null here means no recovery.
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
    public:
        
        static constexpr DataID FunctionalID = 0xFFFFFFFF;

        static ExtraValueInfo* GetValueInfoByName(std::string name) 
        { 
            auto result = _infoTable.find(name); 
            return result == _infoTable.end() ? nullptr : result->second; 
        }

        static AdaptiveValueInfo* GetValueInfoByValue(size_t i)
		{
            //Currently, doesn't do as it should, this is supposed to be the list associated with the actor value enum
			return _adaptiveCount <= i ? nullptr : _adaptiveInfoList[i];
		}

        static AdaptiveValueInfo* GetValueInfoByData(size_t i) 
        { 
            return _adaptiveCount <= i ? nullptr : _adaptiveInfoList[i]; 
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

			logger::info("finalized, {} vs {}", const_size, _adaptiveCount);
		}


    //Membered
	protected:
		//const char* valueName = nullptr;
		std::string valueName{};
		const char* lynchpin = nullptr;

		ValueID _valueID = static_cast<ValueID>(RE::ActorValue::kTotal);  //IE, invalid.

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


