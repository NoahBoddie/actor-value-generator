#pragma once

#include "Arthmetic/TargetType.h"
#include "Arthmetic/RoutineArgument.h"
#include "Arthmetic/IDelegate.h"
#include "Arthmetic/DelegateArgument.h"

namespace Arthmetic
{
    //Phase this out of existence, for now it gets to stay.


    //Make delivery a class.
    //A new array is created for this, const to prevent misedits.
    //At a later time, it will need to be a custom type to carry around ArthArgs to possibly make new ones.
    using ArthmeticFunctionCallback = float(RE::Actor*, const std::vector<DelegateArgument>&);


    inline std::map<std::string, ArthmeticFunctionCallback*> functionMap;//At a later point, this will need a class associated with it.

	struct FunctionDelegate : public IDelegate
    {

        
        //This should be a pair at a later point, with defaults remember those, don't need now,
        // but would prefer for GetActorValue.
	    std::vector<DelegateArgument> params;
    
        ArthmeticFunctionCallback* _callback = nullptr;/*[](RE::Actor* target, const std::vector<ArthmeticFunctionParameter>& parameter) -> float
        {
            float min = parameter[0].GetNumber();
            float max = parameter[1].GetNumber();
        
            //Flawed implementation, do not care.
            int range = max - min + 1;
            int num = rand() % range + min;
        
            cout << "Special Function: min = " << min << ", max = " <<  max << ", result is num " << num << ";";
        
            return num;
        };
        //*/

        //It would seem that this doesn't actually override anything?
		//float Run(const ArgTargetParams& tar_params) override { return _callback(tar_params[_target], params); }

        float Run(const ArgTargetParams& tar_params) override { return _callback(tar_params[_target], params); }

        void Run(RoutineArgument* argument, float& return_value, size_t& index)
        {
            //In super class please
            ArgTargetParams tar_params = argument->GetTargets();
            return_value = Run(tar_params);
        }

        //FunctionDelegate(uint16_t pos) : params{}, IDelegate(pos) {}
    };
}