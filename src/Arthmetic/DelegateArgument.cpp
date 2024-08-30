#pragma once

#include "Arthmetic/DelegateArgument.h"

#include "Arthmetic/IDirective.h"
#include "Arthmetic/ParameterSetting.h"
#include "Arthmetic/RoutineArgument.h"

#include "Arthmetic/TargetType.h"

namespace Arthmetic
{

    const char* DelegateArgument::GetCStringParam(TargetSelector* selector) const
    {
        RoutineArgument* argument = selector ? selector->AsRoutineArg()->GetOriginalArgument() : nullptr;

        //It's this function that's in aeffective. No idea why, but even fresh it doesn't function.
        // Note, it's not subroutine, it's this.

        ArgumentType type = GetType();

        switch (type)
        {
        case ArgumentType::String:
        case ArgumentType::AnyString:

            if (IsParameter() == true)
            {
                logger::info("stringed parameter? {} / {}", _index, argument && argument->_args);

                return !argument || !argument->_args ? "" : (*argument->_args)[_index].GetCStringParam(argument);

            }
            else if (IsProperty() == true) {
                return _property->GetCStringParam(selector);
            }
            else
                return _string;

        default:
            return "";//This is to throw when the time comes.
        }

        //Later this throws an exception to short the function immediately, however at a later point, if any is used
        // NO exception should be thrown, just some way to convey an error value.

        //I think I'll seperate THIS version and a version that would be for API purposes.
        if (IsString() == false)
            return "";

        if (IsParameter() == true)
        {
            logger::info("stringed parameter? {} / {}", _index, argument && argument->_args);

            return !argument || !argument->_args ? "" : (*argument->_args)[_index].GetCStringParam(argument);
        }
        
        return _string;

        //case ArgumentType::Property:
        return _property->GetCStringParam(selector);
    }

    std::string DelegateArgument::GetStringParam(TargetSelector* selector) const
    {
        return GetCStringParam(selector);
    }


    float DelegateArgument::GetNumberParam(TargetSelector* selector) const
    {
        RoutineArgument* argument = selector ? selector->AsRoutineArg()->GetOriginalArgument() : nullptr;

        //It's this function that's in aeffective. No idea why, but even fresh it doesn't function.
        // Note, it's not subroutine, it's this.

        ArgumentType type = GetType();

        switch (type)
        {
        case ArgumentType::Number:
        case ArgumentType::AnyNumber:
            
            if (IsParameter() == true)
            {
                logger::debug("numbered parameter? {} / {}", _index, argument && argument->_args);

                return !argument || !argument->_args ? 0 : (*argument->_args)[_index].GetNumberParam(argument);

            }
            else if (IsProperty() == true){
                
                return _property->GetNumberParam(selector);
            }
            else {
                logger::debug("returning number {}", _number);
                return _number;
            }
        case ArgumentType::Arthmetic:
        {
            logger::debug("arth hit {}", _arthmetic != nullptr);
            return _arthmetic->RunImpl(argument);
        }

        default:
            logger::error("Improper type {}", (int)_type);
            return 0;//This is to throw when the time comes.
        }
    }



    Target DelegateArgument::GetObjectParam(TargetSelector* selector) const
    {
        RoutineArgument* argument = selector ? selector->AsRoutineArg()->GetOriginalArgument() : nullptr;

        //It's this function that's in aeffective. No idea why, but even fresh it doesn't function.
        // Note, it's not subroutine, it's this.

        ArgumentType type = GetType();

        switch (type)
        {
        case ArgumentType::Object:
        case ArgumentType::AnyObject:

            if (IsParameter() == true)
            {
                logger::info("objected parameter? {} / {}", _index, argument && argument->_args);

                return !argument || !argument->_args ? nullptr : (*argument->_args)[_index].GetObjectParam(argument);

            }
            else if (IsProperty() == true) {
                return _property->GetObjectParam(selector);
            }
            else
                return _object;

        default:
            return 0;//This is to throw when the time comes.
        }

        /*
        RoutineArgument* argument = selector ? selector->AsRoutineArg()->GetOriginalArgument() : nullptr;

        if (IsObject() == false)
            return nullptr;

        return _object;


        //case ArgumentType::Property:
            return _property->GetObjectParam(selector);

        //*/
    }


    //I really would like to make this something fucking else at some point. That would make me delighted.
    bool DelegateArgument::SetOwner(IDirective* directive, std::deque<ArthmeticObject*>* owner_stack, ArthmeticObject* owner)//, int index)
    {
        //Index isn't used right now, but it's used to test the would be parameters for validation.

        //This doesn't give a shit about objects right now, that's a pain in the ass.

        //If it's a parameter that's a null arg, then we have a name ask for.
        
        auto cleaned_type = _type & ~ArgumentType::HarmlessFlags;

        logger::debug("owner set, arg type {} {}", (int)cleaned_type, (int)_type);
        
        //if (cleaned_type != ArgumentType::Parameter && cleaned_type != ArgumentType::UnlinkedObject)
        //    return true;//Not a null arg, bailing

        switch (cleaned_type)
        {
        case ArgumentType::Parameter:
            if (directive)
            {
                ParameterSetting* result = directive->FindParamSettingByName(_string);
                
                if (!result)
                    return false;
                
                logger::info("Parameter '{}' found and linked.", _string);

                FreeData(true);

                _index = result->index;
                //We make sure to remove the resolved flag, as it is anything but
                _type |= result->defaultValue._type & ~ArgumentType::Resolved;        

                break;
            }
            else
            {
                std::string code{ _string };

                FreeData(true);

                if (propertyMap.contains(code) == true) {
                    _property = propertyMap[code];
                    _type = _property->GetType() | ArgumentType::Property;//Can't remain property AND have this now can it?
                    logger::info("Property '{}' found and linked.", code);
                }
                else {
                    do_return_X(logger::error("property '{}' not found", code), false);
                }
                break;
            }
            break;
        case ArgumentType::UnlinkedObject:
            if (!directive)
            {
                std::string code{ _string };

                FreeData(true);

                //Do an actual fucking look up.
                Target link = Target::LookUpTarget(code);

                if (link.focus){
                    logger::info("Object '{}' found and linked.", code);
                    _type &= ~ArgumentType::Incomplete;
                    _object = link;
                }
                else {
                    do_return_X(logger::error("Object '{}' not found", code), false);
                }
            }
            break;
        case ArgumentType::Arthmetic:
            if (auto formula = dynamic_cast<IFormula*>(_arthmetic); owner_stack && formula) {
                logger::info("Linked subarthmetic");
                formula->DeclareOwner(*owner_stack, owner);
            }
            else
            {
                logger::error("No sub arthmetic or not setting owner.");
                return false;
            }
            break;
        }

        return true;
    }
}