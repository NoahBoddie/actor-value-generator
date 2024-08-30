#pragma once

#include "Arthmetic/IDelegate.h"
#include "Arthmetic/Coroutine.h"
#include "Arthmetic/Subroutine.h"
#include "Arthmetic/TargetType.h"

#include "Arthmetic/IReadyArthmetic.h"
#include "Arthmetic/ParameterSetting.h"
namespace Arthmetic
{
    
    //temp location.
    //inline std::map<std::string, Subroutine*> routineMap;





	class RoutineArgument;
	

	struct Property : public ArthmeticObject, public DelegateArgument
	{

		void OnLinkage(LinkerFlags link_type, bool& success) override
		{
			constexpr int space_taker = 0;

			if (IsObject() == true)
				SetOwner(nullptr, nullptr, nullptr);
		}


		LinkerFlags GetLinkerFlags() const override 
		{ 
			auto type = _type & ~ArgumentType::HarmlessFlags;

			return type == ArgumentType::UnlinkedObject ? LinkerFlags::Object : LinkerFlags::None;
		}


		Property(std::string& data) : DelegateArgument(CreateDelegateArgument(data, 1))
		{
			if (IsNumber())
				logger::debug("PROP TEST {}", GetNumberParam());
		}
	};


	inline std::map<std::string, Property*> propertyMap;



	//RoutineDelegate is deprecated.
    struct RoutineDelegate : public IDelegate
    {
        //So this will be outdated at some point, outside of it's environment this will be known as the property delegate,
        // which interfaces with parameters, and global properties.

        //Of course, they only return numbers, and this is largely because other delegates solve the problem of non number 
        // properties being used in it's regions.

        //The general gist will be that it will either have a number for a parameter slot, or it will have a pointer to an
        // IReadyArthmetic to use


        IReadyArthmetic* _readyItem = nullptr;

        float Run(Target target) override { return _readyItem->Run(target); }

        void Run(RoutineArgument* argument, float& return_value, size_t& index) override
        { 
            //I don't want to do it right now, but basically make a new ArthArg and run it, set the value to return value.
            // main reason is because this is derived.
            //ArgTargetParams tar_params = argument->GetTargets();
            return_value = Run(argument->GetTarget());
        }


        void LoadFromView(RecordIterator& it) override
        {
            std::string code{ it->view };

            //if (routineMap.contains(code) == true) {
            if (propertyMap.contains(code) == true) {
                
                //_routine = routineMap[code];
                //_readyItem = propertyMap[code];

                logger::info("Qc");
            }
            else {
                //cout << "EEE " << code;
                logger::info("property name not found: {}", code);
                throw nullptr;
            }
        }

        RoutineItemType GetItemType() override { return RoutineItemType::RoutineValue; }

        RoutineDelegate()
        {
            logger::info("RoutineDelegate created");
        }
    };





	enum class PropertyState
	{
		Undefined,
		Parameter,
		Property
	};





	struct PropertyValue : public RoutineItem, public RoutineItemFactoryComponent<PropertyValue, RoutineItemType::RoutineValue>
	{
		//This cannot always be a ready athmetic, so I won't make it one. For now.
		PropertyState GetPropertyState()
		{
			if (IsPartiallyValid() == false)
				return PropertyState::Undefined;
			else if (_raw & address_bytes)
				return PropertyState::Property;
			else
				return PropertyState::Parameter;
		}

		TargetSelector* _selector = nullptr;

		union
		{
			mutable uint64_t _raw = 0;

			mutable char* _propertyName;
			int32_t _propertyIndex;
			Property* _propertyObject;
		};

		static constexpr uint64_t address_bytes = 0xFFFFFFFF00000000;

		int32_t GetPropertyIndex()
		{
			if (GetPropertyState() != PropertyState::Parameter)
				return -1;

			return _propertyIndex;
		}


		Property* GetPropertyObject()
		{
			if (GetPropertyState() != PropertyState::Property)
				return nullptr;

			return _propertyObject;
		}

		//It says, RoutineArgument, but what it actually means is target selector that you can sift through.
		float GetPropertyValue(RoutineArgument* argument)
		{
			//argument = argument->GetOriginalArgument();

			switch (GetPropertyState())
			{
			case PropertyState::Parameter:
				logger::info("ingore:parameter case watch");
				return !argument || !argument->_args ? 0 : (*argument->_args)[_propertyIndex].GetNumberParam(argument);

			case PropertyState::Property:
				return _propertyObject->GetNumberParam(argument);

			case PropertyState::Undefined:
				logger::error("undefined property state");
				break;

			default:
				logger::error("Unknown property state");
				break;
			}

			return 0;

		}

		void Run(RoutineArgument* argument, float& return_value, size_t& index) override
		{
			return_value = GetPropertyValue(argument);
		}


		void LoadFromView(RecordIterator& data) override
		{
			std::string code{ data->view };

			SetData(code);
		}

		LinkerFlags GetLinkerFlags() const override { return IsPartiallyValid() ? LinkerFlags::None : LinkerFlags::External; }




		RoutineItemType GetItemType() override { return RoutineItemType::Value; }


		void OnDeclareOwner(std::deque<ArthmeticObject*>& owner_stack) override
		{
			if (IsPartiallyValid() == true)
				return;


			ArthmeticObject* owner = owner_stack.front();

			IFormula* formula = dynamic_cast<IFormula*>(owner);

			if (!formula)
			{
				//How critical is this really?
				do_return(ARTHMETIC_LOGGER(warn, "No formula found."));
				
			}

			ParameterSetting* result = formula->FindParamSettingByName(_propertyName);

			if (!result)
				return;

			logger::info("Parameter '{}' found and linked.", _propertyName);


			FreeData();

			_propertyIndex = result->index;
			
			AttemptValidation();
		}


		void OnLinkage(LinkerFlags link_type, bool& success) override 
		{
			if (IsPartiallyValid() == true)
				return;

			std::string code{ _propertyName };

			FreeData();
			//Make sure it's the right type please.
			if (propertyMap.contains(code) == true) {
				_propertyObject = propertyMap[code];
				logger::info("Property '{}' found and linked.", code);
			}
			else {
				logger::error("property '{}' not found", code);
				success = false;
			}
		}

		void SetData(std::string& str)
		{
			if (IsPartiallyValid() == true)
				return;
			
			FreeData();

			_propertyName = strdup(str.c_str());
		}

		void FreeData() const
		{
			if (IsPartiallyValid() == true || !_propertyName)
				return;

			logger::debug("freeing property '{}'", _propertyName);

			free(_propertyName);

			_raw = 0;
		}


		~PropertyValue() { FreeData(); }
	};
}