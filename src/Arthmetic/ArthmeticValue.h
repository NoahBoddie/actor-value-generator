#pragma once

#include "Arthmetic/RoutineItem.h"
#include "Arthmetic/IReadyArthmetic.h"

namespace Arthmetic
{
    class RoutineArgument;

    //A const value for a Routine. I would like to rename this
    struct ArthmeticValue : public RoutineItem, public IReadyArthmetic, public RoutineItemFactoryComponent<ArthmeticValue, RoutineItemType::Value>
    {
        float _value{};


        float Run(const Target targets) override { return _value; }
        void Run(RoutineArgument* argument, float& return_value, size_t& index) override { return_value = _value; }


        void LoadFromView(RecordIterator& data) override
        {
            std::string code{ data->view };
            
            float value = std::stof(code);

            _value = value;
        }


        RoutineItemType GetItemType() override { return RoutineItemType::Value; }

        //RoutineItemType GetItemType() override { return RoutineItemType::Value; }
        
        //This is allowed, but it should be setting pos to an invalid
        ArthmeticValue() = default;
        ArthmeticValue(float val) : _value(val) { AttemptFullValidation(); }
    };


}