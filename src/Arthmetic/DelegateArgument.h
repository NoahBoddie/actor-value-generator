#pragma once

#include "Arthmetic/Record.h"
#include "Arthmetic/ArgumentType.h"
#include "Arthmetic/IReadyArthmetic.h"


namespace Arthmetic
{
    class RoutineArgument;
    class IDirective;
    class Property;

    //This is to be renamed delegate parameter, as both delegate types would use them.
    //delegate parametes should take routine arguments, so I can have delegate parameters that refer to parameters being used
    // Then for each of these, should I be in the use X parameter

    //I would like to rename this to variable, more accurate.
	struct DelegateArgument
    {
        //std::string name{};
    
        mutable ArgumentType _type{};
    
        ArgumentType GetType() const { return _type & ~ArgumentType::AllFlags; }
        ArgumentType GetCoreType() const { return _type & ~ArgumentType::RawFlags; }
        ArgumentType GetFlags() const { return _type & ArgumentType::RawFlags; }



        inline bool IsResolved() const { return !!(_type & ArgumentType::Resolved); }
        inline bool IsManaging() const { return IsArthmetic() && !!(_type & ArgumentType::Incomplete); }
        inline bool IsParameter() const { return !!(_type & ArgumentType::Parameter); }
        inline bool IsProperty() const { return !!(_type & ArgumentType::Property); }
        inline bool AnyAllowed() const { return !!(_type & ArgumentType::Any); }

        inline bool IsType(ArgumentType query_type) const 
        { 
            ArgumentType this_type = _type & ~ArgumentType::RawFlags;
            query_type &= ~ArgumentType::RawFlags;
            return query_type == this_type; 
        }

        //These don't care about any, it is to say that it is currently loaded with the value of this.
        inline bool IsObject() const { return IsType(ArgumentType::Object); }
        inline bool IsUnlinkedObject() const { return !!(_type & ArgumentType::UnlinkedObject); }

        inline bool IsNumber() const { return IsType(ArgumentType::Number) || IsArthmetic(); }//Arthmetic should count in this.
        inline bool IsString() const { return IsType(ArgumentType::String); }
        inline bool IsArthmetic() const { return IsType(ArgumentType::Arthmetic); }


        inline bool CanBeType(ArgumentType query_type) const
        {
            if (AnyAllowed() == true)
                return true;

            return IsType(query_type);
        }
         
        inline bool CanBeObject() const { return CanBeType(ArgumentType::Object); }
        inline bool CanBeNumber() const { return CanBeType(ArgumentType::Number); }
        inline bool CanBeString() const { return CanBeType(ArgumentType::String); }







        DelegateArgument ResolveArgument(TargetSelector* selector)
        {
            //THis will need a routine argument to resolve itself when it uses parameters. So I'll take care of that later.
            
            
            //THIS is a temporary fix, some how things end up being considered resolved when they shouldn't be, like parameter using arguments.
            if (IsResolved() == true)
                return *this;

            DelegateArgument resolve_param{};

            resolve_param._type = ArgumentType::Resolved;

            //Right here, it should have a say if any is used.

            get_switch (GetCoreType())
            {
            case ArgumentType::Arthmetic:
            case ArgumentType::Number:
            {
                float value = GetNumberParam(selector);
                if (switch_value == ArgumentType::Arthmetic)
                   // logger::info("CALLER, the arthmetic is up, is managing? {} {}", IsManaging(), value);


                resolve_param = value;
                break;
            }
            case ArgumentType::String:
            {
                auto value = GetStringParam(selector);


                resolve_param = value;

                //logger::info("CALLER, the Strings is up, is resolved? {} {} {}, I am {}", resolve_param.IsResolved(), (int)resolve_param._type, value, _type);

                //I'm not supposed to have to do this, but I'm thinking I may have to.
                resolve_param._type |= ArgumentType::Resolved;

                break;
            }
            case ArgumentType::Object:
            {
                auto value = GetObjectParam(selector);
                //logger::info("CALLER, the arthmetic is up, is managing? {} {}", IsManaging(), value);

                resolve_param = value;
                break;
            }

            case ArgumentType::Parameter://Find parameter, and fill.
                break;
            default://Anything else is allowed to just pour in it's value.
                resolve_param = *this;
                break;
            }

            return resolve_param;
        }

        void ForceResolved() { _type |= ArgumentType::Resolved; }



        union
        {
            
            mutable uint64_t _value{0};
            mutable char* _string;//When outputted, put into an std::string
            mutable float _number;
            mutable uint32_t _index;
            //mutable const char* _hardString;//Only to be filled when the type isn't something to be managed.
            mutable IReadyArthmetic* _arthmetic;
            mutable Property* _property;
            mutable Target _object;

            
        };
    
        //Not sure if I want something like this loose, we'll see.
        DelegateArgument() = default;
        

        DelegateArgument(const DelegateArgument& a_rhs) { TransferData(a_rhs); }
    

        DelegateArgument(const DelegateArgument&& a_rhs) { TransferData(a_rhs); }
        

        bool ConstructAs(DataType type, std::string& str)
        {
            FreeData(true);


            switch (type)
            {
                //I'm only doing this for now the rest can bugger off.
                // Make these inline functions as well so I can reuse them.
            case DataType::ParameterCall:
            {
                //May be able to use incomplete to differentiate, but incomplete already implies a lack of linking.
                // It'd be fucking weird to tack that on top.
                _type = ArgumentType::Parameter;
                logger::debug("Setting param name {} W/ Other", str);
                _string = strdup(str.c_str());
                return true;
            }
            case DataType::Object:
            {
                _type = ArgumentType::Object | ArgumentType::Incomplete;
                logger::debug("Setting object name {} W/ Other", str);
                _string = strdup(str.c_str());
                return true;
            }
            
            default:
                return false;
            }
        }

        //*/
        DelegateArgument(const RecordIterator& a_rhs)
        {
            switch (a_rhs->GetType())
            {
                //I'm only doing this for now the rest can bugger off.
                // Make these inline functions as well so I can reuse them.
            case DataType::ParameterCall:
            {
                //May be able to use incomplete to differentiate, but incomplete already implies a lack of linking.
                // It'd be fucking weird to tack that on top.
                _type = ArgumentType::Parameter;
                std::string null_term_data(a_rhs->view.begin(), a_rhs->view.end());//a_rhs->view.data();
                logger::debug("Setting param name {}", null_term_data);
                _string = strdup(null_term_data.c_str());
                return;
            }
            case DataType::Object:
            {
                _type = ArgumentType::Object | ArgumentType::Incomplete;
                std::string null_term_data(a_rhs->view.begin(), a_rhs->view.end());//a_rhs->view.data();
                logger::debug("Setting object name {}", null_term_data);
                _string = strdup(null_term_data.c_str());
                return;
            }
            default:
                break;
            }

        }


        DelegateArgument& operator=(DelegateArgument a_rhs)
        {
            FreeData(true);

            TransferData(a_rhs);

            return *this;
        }


        DelegateArgument& operator=(float a_rhs)
        {
            FreeData(true);

            _type &= ArgumentType::Resolved;
            _type |= ArgumentType::Number;

            _number = a_rhs;

            return *this;
        }

        DelegateArgument& operator=(std::string& a_rhs)
        {
            FreeData(true);

            //This shouldn't keep resolved if it's going to give it a new one.
            _type &= ArgumentType::Resolved;
            _type |= ArgumentType::String;

            _string = strdup(a_rhs.c_str());

            return *this;
        }

        DelegateArgument& operator=(Target a_rhs)
        {
            FreeData(true);

            _type &= ArgumentType::Resolved;
            _type |= ArgumentType::Object;

            _object = a_rhs;

            return *this;
        }

        DelegateArgument& operator=(BaseObject* a_rhs)
        {
            FreeData(true);

            _type &= ArgumentType::Resolved;
            _type |= ArgumentType::Object;

            _object = a_rhs;

            return *this;
        }



        DelegateArgument(std::string& str)
        {
            //When created, but only when created, this is a manager, so even if marked resolved, it will handle it's data.
            _type = ArgumentType::String;
            //_string =  new char[str.size()];
            //strcpy(_string, str.c_str());
            _string = strdup(str.c_str());
        }
    
    
        DelegateArgument(float num)
        {
            _type = ArgumentType::Number;
            _number = num;
        }
           
        DelegateArgument(IReadyArthmetic* func)
        {
            _type = ArgumentType::Arthmetic;
            _arthmetic = func;
        }
    
        DelegateArgument(Target tar)
        {
            _type = ArgumentType::Object;
            _object = tar;
        }

        //
        DelegateArgument(BaseObject* obj)
        {
            _type = ArgumentType::Object;
            _object = obj;
        }
    

        DelegateArgument(ArgumentType type)
        {
            //When created like this, no flags are allowed. Accept any which is a type.
            _type = type & ~ArgumentType::AllFlags;
        }


        //The routine object sent here will need to be some sort of simple helper class
        //, something like a union that can tell me if it's a routine argument 
        // or a target. Because these can be used without a routine argument,
        // but will still come with a target

        const char* GetCStringParam(TargetSelector* selector = nullptr) const;
        std::string GetStringParam(TargetSelector* selector = nullptr) const;
        float GetNumberParam(TargetSelector* selector = nullptr) const;
        Target GetObjectParam(TargetSelector* selector = nullptr) const;
        

        bool SetOwner(IDirective* directive, int index);


        void TransferData(const DelegateArgument& a_rhs) const
        {
            if (IsManaging() == true){
                _type |= ArgumentType::Incomplete;
            }

            //bool resolved = IsResolved();// && !IsManaging();
            bool is_string = (a_rhs._type & ~(ArgumentType::Any | ArgumentType::Resolved)) == ArgumentType::String;

            //Something for these resolved types, shouldn't allow them to be set with things like arthmetic
            
            //if (a_rhs._type == ArgumentType::String && a_rhs._string)
            if (is_string && a_rhs._string)
            {
                _type &= ArgumentType::Resolved;
                _type |= a_rhs._type;

                
                _string = strdup(a_rhs._string);
               
                //logger::info("TRANS S '{}'", (int)_type);
                
                /*
                if (!resolved) {
                    _string = strdup(a_rhs._string);
                }
                else {
                    logger::info("harmless copy");
                    _string = a_rhs._string;
                }
                //*/
            }
            else if (a_rhs._type == ArgumentType::Parameter && a_rhs._string)
            {
                _type = ArgumentType::Parameter;

                _string = strdup(a_rhs._string);
            }
            else
            {
                //logger::info("TEST '{}'", _value);

                _type &= ArgumentType::Resolved;
                _type |= a_rhs._type;
                _value = a_rhs._value;
            }


            //Strings don't have managers because they don't have shared data, they copy. Subroutines are less easy
            // to do that, so if this is a string, whoever is "managing" is kept, because it actually means
            // they need to delete regardless of resolution. Confusing I know
            if (a_rhs.IsManaging() == true)
            {
                logger::critical("DFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

                a_rhs._type |= ArgumentType::Incomplete;
                _type &= ~ArgumentType::Incomplete;
            }

        }

        void FreeData(bool set_null, bool ignore_resolve = false) const
        {
            //Free data has an issue I need to fucking resolve.
            //return;
            //if (IsResolved() == true)// && IsManaging() == false)//!ignore_resolve && 
            //    return;

            ArgumentType flags = GetFlags();

            get_switch (GetType())
            {
            case ArgumentType::Arthmetic:
                logger::debug("query deleting arthmetic, {}", !(flags & ArgumentType::Incomplete), _arthmetic != nullptr);
            if (!(flags & ArgumentType::Incomplete) && _arthmetic != nullptr)
            {
                logger::info("Deleting arthmetic");
                delete _arthmetic;
            }
            break;

            default:
                //IE, if it only has this and nothing else
                if (_type != ArgumentType::Parameter || (_type & ~ArgumentType::HarmlessFlags) != ArgumentType::UnlinkedObject)
                    return;

                goto skip_param_check;
                //return;
            case ArgumentType::String:
            case ArgumentType::AnyString:
            if (IsParameter() == true)
                return;
                
            skip_param_check:
            if (_string)
            {
                //logger::debug("freeing '{}' or {}, is str {}, is para {}", _string, _index, switch_value == ArgumentType::String, _type == ArgumentType::Parameter);
                free(_string);

            }
            break;
            }


            if (set_null)
                _value = 0;
        }
    
        ~DelegateArgument()
        {
            FreeData(true);
        }
    };
    //using ArthFuncParam = DelegateArgument;



    //using ParameterList = std::vector<ARTH_TARGET_TYPE*>;
    struct ParameterList : public std::vector<DelegateArgument>
    {
        using std::vector<DelegateArgument>::vector;

        DelegateArgument defaultValue = 0.f;

        //I won't be making constructors yet, giving it some time.
        //ParameterList()

        //This to make new constructors.
        // So one constructor I'd maybe like is giving delegate parameters that are basically paired with indexes to put them in.
        // Oh, yeah, also default value.
    };


}