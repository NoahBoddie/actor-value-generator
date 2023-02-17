#pragma once

namespace Arthmetic
{
	/*
	template<class T>requires(T::Self == 0)
	constexpr bool target_has_self() { return true; }
	template<class T>
	constexpr bool target_has_self() { return false; }

	//Static doesn't have to exist. if not present, it will skip over it.
	template<class T>requires(T::Static != 0)
	constexpr bool target_has_static() { return true; }
	template<class T>
	constexpr bool target_has_static() { return false; }


	template<class T>requires(T::Total != 0)
	constexpr bool target_has_total() { return true; }
	template<class T>
	constexpr bool target_has_total() { return false; }


	//This is externally defined now, by whatever is using it. The imperitive is, when you define it you must define:
	//Self, Total, And StaticTargets. These 3 are the corner stones of this code
	//This should be determined by a macro name called TARGET_ENUM
	enum TargetType_Deprecated : uint8_t
	{
		Self,
		//DamageCause,//This is basically the person who did the thing damage if there is one.
		//Owner,
		//Player,
		//CombatTarget,
		//DialogueTarget,
		//ScriptForm1,
		//ScriptForm2,
		Static,
		Total = Static
	};

	//static_assert(target_has_self<TargetType>(), "TargetType::Self does not exist.");
	//static_assert(target_has_static<TargetType>(), "TargetType::Static does not exist.");
	//static_assert(target_has_total<TargetType>(), "TargetType::Total does not exist.");

#ifdef ARTH_TARGET_TYPE
	using ArgTargetParams  = std::array<ARTH_TARGET_TYPE*, TargetType::Total>;
#else 
	using ArgTargetParams  = std::array<void*, TargetType::Total>;
#endif


#ifdef ARTH_TARGET_TYPE
	using StaticTargetArgs = std::array<ARTH_TARGET_TYPE*, TargetType::Total>;
#else 
	using StaticTargetArgs = std::array<void*, TargetType::Total>;
#endif

	//Just gonna force this shit for now.
	//class ObjectType;

	using ObjectType_Temp = RE::TESForm;



#ifdef Custom_List_Class

	struct TargetList
	{

		//This is what's used externally, because if I introduce new types I think the different versions would break shit.
		//I think these will consist of 2 different types.

		//The first would be the dynamic targets, then the static targets. Would make it easier to handle I think. and would prevent an index
		// error. Will work on that when targeting becomes more important.

		//Additionally, make a wrapper object for accessing the target and object types. This way, I can send my personal exception and bail the function

		std::unique_ptr<ARTH_TARGET_TYPE*[]> targets = nullptr;
		size_t length;

		TargetList(ArgTargetParams& target_params)
		{
			//targets = decltype(targets)(new ArgTargetParams(target_params));
			length = target_params.size();
		}


		TargetList(const ArgTargetParams& target_params)
		{
			//targets = decltype(targets)(new ArgTargetParams(target_params));
			length = target_params.size();
		}
	};

#else
	using TargetList = std::vector<ARTH_TARGET_TYPE*>;

#endif
	//The actual target type that's sent should be different. Basically, it should be a pointer
	// to the array with a constant integer that says how many there are. This way, 
	// updates that introduce new targets don't break existing plugin targeting set ups.
	// I believe the types because of the specification of type.

	//Old, not sure why it's not working. std::derived_from<ARTH_TARGET_TYPE>... DerivedTarget
	template<class... DerivedTarget>//requires(sizeof...(DerivedTarget) < TargetType::Total)
	ArgTargetParams MakeTargetParamList(DerivedTarget*... target_list)//This shouldn't exceed target type total
	{
		return ArgTargetParams{ target_list... };
	}
	

	TargetList MakeTargetListFromTarget(ARTH_TARGET_TYPE* run_target)
	{
		//This too will have to be defined elsewhere. All 
		ArgTargetParams start_list{ run_target };

		//Stuff is to be done here.

		return TargetList(start_list.begin(), start_list.end());
	}

	//*/


	//using Target = void;			//Would be form
	//using TargetType = TestEnum;	//Would be form type. Used to have something require a specific form type to proceed. (Can even make a function for?
	//using TargetContext = void;		//Would be extra data
/*

#ifdef ARTH_OBJECT_TYPE
	using BaseObject = ARTH_OBJECT_TYPE;
#else 
	using BaseObject = void;
#endif


#ifdef ARTH_CONTEXT_TYPE
	using TargetContext = ARTH_CONTEXT_TYPE;
#else 
	using TargetContext = void;
#endif


#ifdef ARTH_ENUM_TYPE
	using TargetType = ARTH_ENUM_TYPE;
#else 
	using TargetType = std::uint32_t;
#endif

//*/

	class RoutineArgument;

	struct Target
	{
		static Target LookUpTarget(std::string context);

		//Use this instead of target, fuctional selectors could be used to get specific targets, like worn objects or most significant
		// healthed extradata or whatever. IE, treat this like a pointer.
		BaseObject* focus = nullptr;
		TargetContext* context = nullptr;//Might give this a unique class specifically too to hold onto more extra data.


		//Should have some way to declare this undeclared.
		//Basically, this is supposed to be determined via declaration like this rather than an override.
		TargetType GetType();


		Target() = default;
		constexpr Target(BaseObject* _f) : focus(_f) {}
		constexpr Target(BaseObject* _f, TargetContext* _c) : focus(_f) {}
		Target& operator=(BaseObject* base) { focus = base; context = nullptr; return *this; }

		//Should effectively be an alias for Target*
		operator BaseObject* () const { return focus; }

		BaseObject* operator->() const
		{
			return focus;
		}
	};

	struct TargetSelector
	{
		//Note, by design, this isn't always supposed to be filled. What this effectively means is when called from something like
		// a delegate owns one of these as a pointer, and when the target is going to be gotten, the routineArgument goes in, safeGet is called
		// and since it has no selector, it uses the default from RoutineArgument.


		virtual Target GetTarget(TargetSelector*) = 0;


		Target GetTargetSafe(TargetSelector* selector)
		{
			//This version of get target, while a member function doesn't actually require the member to exist. And if the member does not
			// it will just return the target from the target used.

			if (!this) {
				return selector ? selector->GetTarget(nullptr) : nullptr;
			}

			return GetTarget(selector);
		}


		inline Target GetTarget(Target target);


		inline Target GetTargetSafe(Target target);

		//Not always will there be context, and if there's some way to get context that could be good.
		// Main point however is that items can be sent with extra information.
		virtual TargetContext* GetContext() { return nullptr; }



		//THIS, replaces a the object type in Object

		virtual RoutineArgument* AsRoutineArg() { return nullptr; }
	};
	
	

	//Move me!
	TargetType Target::GetType() { return focus ? focus->GetFormType() : RE::FormType::None; }
	//Move me!

	class BasicSelector : public TargetSelector
	{
	private:
		//This is the basic version of the selector, most things can use this, but not all things would have to.
		// RoutineArgument is primarily what is going to be handling this.
		Target _target;
	
	public:
		Target GetTarget(TargetSelector* selector = nullptr) override { return _target; }

		BasicSelector() = default;
		BasicSelector(Target tar) : _target(tar) {}
	};

	//Under something like the above, youd have something like KeywordSelectors, that would just be manually created and managed 
	// constant targets.

	Target TargetSelector::GetTarget(Target target)
	{
		BasicSelector basic(target);
		return GetTarget(&basic);
	}

	Target TargetSelector::GetTargetSafe(Target target)
	{
		BasicSelector basic(target);
		return GetTargetSafe(&basic);
	}
	
	
	//Unsure what should be used yet.
	using SelectFunction = Target(*)(BaseObject*);

	class FunctionalSelector : public TargetSelector
	{
		//This isn't going to be used, but if I were to do something like
		// target.combatTarget, combat target would be a functional selector

		TargetSelector* _select;
		SelectFunction _func;

		Target GetTarget(TargetSelector* selector) override
		{
			//How this works is it resolves the thing above it, and then performs a function on that.
			// new functions should be able to be added via mod api.

			Target result = _select->GetTargetSafe(selector);

			return _func(result);
		}
	};




	class ParameterSelector : public TargetSelector
	{
		//While not all selectors would need to, routine args would undoubtably need to have something loaded into their selector in order to function.
		// Perhaps all selectors take another selector, making proper space for routine argument without needing one.
		//Alright, so here is how this is gonna work. It sends a regular old TargetSelector with each one. It can't be IN it all the time so you know.
		// So what gets sent will still be the routine argument, which is what is currently the target. BUT, for a parameter selector it will
		// transfer it into being a routineArgument. This means I don't need to use it always, but it can be used for specific types.

		std::int32_t _index = -1;

		//Target* GetTarget() override { return selector->GetTarget(); }


		Target GetTarget(TargetSelector* selector) override
		{
			RoutineArgument* argument = selector->AsRoutineArg();
			return nullptr;
			//Will do this when I move it. For now, no targeting works aside from routine.
			//return !argument || !argument->_args ? nullptr : (*argument->_args)[_index].GetObjectParam(nullptr);
		}
	};


}