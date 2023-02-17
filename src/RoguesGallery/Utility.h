#pragma once


#include <cmath>
#include <stdarg.h>
# define pi				3.14159265358979323846  /* pi */
//This doesn't need to be define, just make this a constexpr
constexpr uint32_t sign_bit = 0x80000000;

//#define rename_and_move_dev_symbol

using string_hash = uint32_t;




namespace RGL
{

	//*
	//I haven't taken to it yet here, but in the places I use "using" I would like to put uName.
	// That way I know where to go for the definition.

	using HitEvent = RE::TESHitEvent;
	using CombatEvent = RE::TESCombatEvent;
	using REFRLoadEvent = RE::TESObjectLoadedEvent;
	using EffectEvent = RE::TESActiveEffectApplyRemoveEvent;
	using AnimEvent = RE::BSAnimationGraphEvent;
	using EquipEvent = RE::TESEquipEvent;

	using AnimSource = RE::BSTEventSource<AnimEvent>;
	using HitSource = RE::BSTEventSource<RE::TESHitEvent>;
	using CombatSource = RE::BSTEventSource<RE::TESCombatEvent>;
	using REFRLoadSource = RE::BSTEventSource<RE::TESObjectLoadedEvent>;
	using EffectSource = RE::BSTEventSource<RE::TESActiveEffectApplyRemoveEvent>;
	using EquipSource = RE::BSTEventSource<RE::TESEquipEvent>;

	using SkyrimVM = RE::BSScript::IVirtualMachine;



	using uATTACK_ENUM = RE::ATTACK_STATE_ENUM;
	using BipedObjectSlot = RE::BIPED_MODEL::BipedObjectSlot;
	using ArmorType = RE::BIPED_MODEL::ArmorType;
	using EventResult = RE::BSEventNotifyControl;
	using AVModifier = RE::ACTOR_VALUE_MODIFIER;
	using AttackDataFlags = RE::AttackData::AttackFlag;
	using HitFlag = RE::HitData::Flag;
	using HitEventFlag = RE::TESHitEvent::Flag;
	using uCastingSource = RE::MagicSystem::CastingSource;
	using uMagicCastState = RE::MagicCaster::State;
	using uWardState = RE::MagicSystem::WardState;
	//enum class GraphValueType { kBool, kFloat, kInt };
	
	using uEntryPoint = RE::BGSEntryPoint::ENTRY_POINT;

	using ProjectileType = RE::BGSProjectileData::Type;


	enum class EffectValue {Magnitude, Area, Duration };





	constexpr BipedObjectSlot operator &(BipedObjectSlot lhs, BipedObjectSlot rhs) noexcept
	{
		return static_cast<BipedObjectSlot> (
			static_cast<std::underlying_type<BipedObjectSlot>::type>(lhs) &
			static_cast<std::underlying_type<BipedObjectSlot>::type>(rhs)
			);
	}

	constexpr HitFlag operator &(HitFlag lhs, HitFlag rhs) noexcept
	{
		return static_cast<HitFlag> (
			static_cast<std::underlying_type<HitFlag>::type>(lhs) &
			static_cast<std::underlying_type<HitFlag>::type>(rhs)
			);
	}



	template<class T> inline constexpr T operator ~ (T a) { return (T)~(int)a; }
	template<class T> inline constexpr T operator | (T a, T b) { return (T)((int)a | (int)b); }
	template<class T> inline constexpr T operator & (T a, T b) { return (T)((int)a & (int)b); }
	template<class T> inline constexpr T operator ^ (T a, T b) { return (T)((int)a ^ (int)b); }
	template<class T> inline constexpr T& operator |= (T& a, T b) { return (T&)((int&)a |= (int)b); }
	template<class T> inline constexpr T& operator  &= (T& a, T b) { return (T&)((int&)a &= (int)b); }
	template<class T> inline constexpr T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }
	//*/
	//template<class T> inline constexpr bool Any(T a, T b) { return (a | b) == a; }
	template<class T> inline constexpr bool Any(T a, T b) { return int(a & b) != 0; }

	template <class Enum, Enum A, Enum B> inline constexpr bool enum_contains = (A & B) == B;


	template <class Type, Type val, Type set> struct set_type { static constexpr Type value = set; };
	template <class Type, Type val, Type set>  inline constexpr Type set_type_v = set_type<Type, val, set>::value;

	template<bool...> struct bool_pack;
	template<bool... bs>
	using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;


	template<bool... bs>
	using any_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, set_type_v<bool, bs, false>...>>;


	//creates the default value of literally anything if applicable
	

	//namespace DefaultType
	struct DefaultType
	{
		struct Default
		{
			template <class T> operator T() const { static T value{}; return value; }
		} static inline value;
	};

#define __MACRO__NULL_COND_CHOICE(PREFIX,_0,_1,_2,_3,_4,_5,SUFFIX,...) PREFIX##SUFFIX 

#define __MACRO__CHOICE__5(PREFIX,_0,_1,_2,_3,_4,_5,SUFFIX,...) PREFIX##SUFFIX 


//This macro only really works on stuff that doesn't want to return stuff
#define NULL_CONDITION(mc_variable) if (mc_variable) mc_variable
//Make a method for a second parameter to be a default value

//#define NULL_CONDITION_RET(mc_variable) mc_variable ? DefaultType::value : mc_variable
#define RET_1(mc_variable) RET_2(mc_variable, DefaultType::value)
#define RET_2(mc_variable, mc_default_value) !mc_variable ? mc_default_value : mc_variable

#define RET_0() static_assert(false, "Return Null Conditional requires at least 1 parameter.");
#define RET_3() static_assert(false, "Return Null Conditional does not take 3 parameters.");
#define RET_4() static_assert(false, "Return Null Conditional does not take 4 parameters.");
#define RET_5() static_assert(false, "Return Null Conditional does not take 5 parameters.");


#define NULL_CONDITION_RET(...) __MACRO__NULL_COND_CHOICE(RET_, _0,__VA_ARGS__,5,4,3,2,1,0)(__VA_ARGS__)



#define TMP_5() static_assert(false, "Temporary Null Conditional does not take 5 parameters.");
#define TMP_4() static_assert(false, "Temporary Null Conditional does not take 4 parameters.");
#define TMP_0() static_assert(false, "Temporary Null Conditional requires at least 1 parameter.");

#define TMP_1(mc_conditional) if (auto __macro__cond_value = mc_conditional) if (__macro__cond_value) __macro__cond_value
#define TMP_2(mc_test, mc_condition) TMP_1(NULL_CONDITION_RET(mc_test)##->##mc_condition)
#define TMP_3(mc_test, mc_condition, mc_default_value) TMP_1(NULL_CONDITION_RET(mc_test, mc_default_value)##->##mc_condition)



#define NULL_CONDITION_TMP(...) __MACRO__NULL_COND_CHOICE(TMP_, _0,__VA_ARGS__,5,4,3,2,1,0)(__VA_ARGS__)





#define ENUM_2(mc_enum_name, mc_integer_name) \
namespace detail { struct MacroStruct__##mc_enum_name { enum Type : mc_integer_name; }; }\
using mc_enum_name = detail::MacroStruct__##mc_enum_name::Type; \
enum detail::MacroStruct__##mc_enum_name::Type : mc_integer_name 

#define ENUM_1(mc_enum_name) ENUM_2(mc_enum_name, int)

#define ENUM(...) __MACRO__CHOICE__5(ENUM_, _0,__VA_ARGS__,5,4,3,2,1,0)(__VA_ARGS__)

//2 versions of the above seek to occur, the first will present the object in full, the other version will seek to 
//test the first argument, then use the second on it. IE, it will use NULL_CONDITION_RET in order to safe test the value.
//Any entry past will create a static assert saying "X" Evaluation must only have 2 parameters

	struct foo_cond_null
	{
		void test_1()
		{}
		foo_cond_null* test_2()
		{
			return test_val;
		}

		foo_cond_null* test_val = this;
	};

	inline void Test(foo_cond_null* cond_test)
	{
		NULL_CONDITION(cond_test)->test_1();
		auto get_tester = NULL_CONDITION_RET(cond_test)->test_2();
		if (auto test_foo = NULL_CONDITION_RET(cond_test)->test_2())
			if (test_foo)
				return;

		NULL_CONDITION_TMP(cond_test, test_2())->test_1();
	}


	class UtilityRG
	{
	private:
		

	public:



		static float Clamp(float value, float a, float b, bool r = false)
		{//Reverse clamp is functioning quite wrong, fix later.
			//if r is true, only the formers are considered, if false only the latters are.
			float min = a > b ? (r ? a : b) : (r ? b : a);
			float max = min == a ? b : a;

			if (value < min)
				return min;
			else if (value > max)
				return max;
			else
				return value;
		}

		static float ReverseClamp(float value, float a, float b)
		{
			return Clamp(value, a, b, true);
		}


		static int iClamp(float value, float a, float b, bool r = false)
		{
			return (int)Clamp(value, a, b, r);
		}

		static int iReverseClamp(float value, float a, float b)
		{
			return iClamp(value, a, b, true);
		}


		static float Lerp(float a, float b, float percent) 
		{
			percent = Clamp(percent, 0, 1);

			return LerpUnclamped(a, b, percent);
		}

		static float LerpUnclamped(float a, float b, float percent)
		{
			return (b - a) * percent + a;
		}

		static float GetDistance(RE::TESObjectREFR* a_ref, RE::TESObjectREFR* b_ref)
		{
			if (!a_ref || !b_ref)
				return -1.f;

			RE::NiPoint3 a_pos = a_ref->GetPosition();
			RE::NiPoint3 b_pos = b_ref->GetPosition();

			return a_pos.GetDistance(b_pos);

		}

		static RE::ActiveEffect* GetEffectFromID(RE::MagicTarget* magicTarget, std::uint16_t uniqueID)
		{
			if (!magicTarget)
				return nullptr;

			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return nullptr;

			for (auto effect : *activeEffects)
			{
				if (effect->usUniqueID == uniqueID)
					return effect;
			}

			return nullptr;
		}

		static RE::ActiveEffect* GetEffectFromSetting(RE::MagicTarget* magicTarget, RE::EffectSetting* setting)
		{
			if (!magicTarget)
				return nullptr;


			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return nullptr;

			for (auto effect : *activeEffects)
			{
				if (effect->GetBaseObject() == setting)
					return effect;
			}

			return nullptr;
		}

		//Use a vector
		static std::list<RE::ActiveEffect*> GetAllEffectsFromSetting(RE::MagicTarget* magicTarget, RE::EffectSetting* setting)
		{

			std::list<RE::ActiveEffect*> returnList;

			if (!magicTarget)
				return returnList;

			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return returnList;

			for (auto effect : *activeEffects)
			{
				if (effect->GetBaseObject() == setting)
					returnList.push_back(effect);
			}

			return returnList;
		}

		static RE::TESObjectWEAP* GetAttackingWeapon(RE::Actor* actor)
		{
			if (!actor)
				return nullptr;

			auto atkInvData = actor->GetAttackingWeapon();

			if (!atkInvData)
				return nullptr;
#pragma push_macro("GetObject")
#undef GetObject
			auto result = atkInvData->GetObject();
#pragma pop_macro("GetObject")
			if (!result)
				return nullptr;

			return result->As<RE::TESObjectWEAP>();
		}


		/// <summary>
		/// Use BGSAttackDataMap::GetAttackData_1403E88F0 instead
		/// </summary>
		/// <param name="actor"></param>
		/// <param name="value"></param>
		/// <returns></returns>
		static RE::BGSAttackData* GetAttackData(RE::Actor* actor, const RE::BSFixedString& value)
		{
			if (!actor)
				return nullptr;


			auto race = actor->GetRace();

			if (!race)
				return nullptr;

			auto& atkMap = race->attackDataMap->attackDataMap;

			auto iter = atkMap.find(value);
			if (iter != atkMap.end()) {
				return iter->second.get();
			}

			return nullptr;
		}

		static RE::BGSAttackData* GetCurrentAttackData(RE::Actor* actor)
		{
			//use Get currentData here
			if (!actor)
				return nullptr;

			auto& actor_runtime = actor->GetActorRuntimeData();

			if (!actor_runtime.currentProcess ||
				!actor_runtime.currentProcess->high ||
				!actor_runtime.currentProcess->high->attackData)
				return nullptr;

			return actor_runtime.currentProcess->high->attackData.get();
		}

		static bool IsCasting(RE::Actor* actor)
		{
			if (!actor)
				return false;

			for (int i = 0; i < 4; i++)
			{
				auto cast_source = static_cast<RE::MagicSystem::CastingSource>(i);
				auto magic_caster = actor->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand);

				if (magic_caster && magic_caster->state)
					return true;
			}
		}
	};
}