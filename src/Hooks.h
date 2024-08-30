#pragma once

#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"
#include "FormExtraValueHandler.h"
#include "Utility.h"
#include "Addresses.h"

#include "xbyak/xbyak.h"

namespace AVG
{
	
	//General note, you don't need to be putting the xbyak code on the trampoline, it's effectively exclusively for personal use,
	// and we're jumping from it locally.



	//I'D LIKE TO BE MOVED PLEASE

	inline bool IsAE()// = REL::Module::GetRuntime() == REL::Module::Runtime::AE;
	{
		return REL::Module::GetRuntime() == REL::Module::Runtime::AE;
		//static bool is_ae = REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_5_97) == std::strong_ordering::greater;
		//return is_ae;
	}

	/*
	template<class T>
	bool ArrayCheck(T& value)
	{
		return std::is_array<T>::value;
	}

	template <class T>
	void write_branch(std::uintptr_t a_src, std::uintptr_t ret_add = 0, int index = 0)
	{
		//should come with a size too. As well as a return address, which specifies what to set the func as.
		// this is optional for write branch only.
		auto& trampoline = SKSE::GetTrampoline();

		auto query = trampoline.write_branch<5>(a_src, (uintptr_t)T::thunk);

		auto& go_to = 0 <= index && ArrayCheck(T::func) ? T::func[index] : T::func;

		if (query == a_src)
			go_to = ret_add;
		else
			go_to = query;
	}

	template <class T>
	void write_vfunc(std::uintptr_t a_src, std::uintptr_t ret_add = 0, int index = 0)
	{
		//should come with a size too. As well as a return address, which specifies what to set the func as.
		// this is optional for write branch only.
		auto& trampoline = SKSE::GetTrampoline();

		auto query = trampoline.write_branch<5>(a_src, (uintptr_t)T::thunk);

		auto& go_to = 0 <= index && ArrayCheck(T::func) ? T::func[index] : T::func;

		if (query == a_src)
			go_to = ret_add;
		else
			go_to = query;
	}
	//*/

	int IsCallOrJump(uintptr_t addr)
	{
		//0x15 0xE8//These are calls, represented by negative numbers
		//0x25 0xE9//These are jumps, represented by positive numbers.
		//And zero represent it being neither.

		if (addr)
		{
			auto first_byte = reinterpret_cast<uint8_t*>(addr);

			switch (*first_byte)
			{
			case 0x15:
			case 0xE8:
				return -1;

			case 0x25:
			case 0xE9:
				return 1;

			}
		}

		return 0;
	}

	template <int I>
	using ActorType = std::conditional_t<I == 0, RE::Character*, RE::PlayerCharacter*>;
	
	template <int I>
	struct PlayerOrActor
	{
		using ActorType = std::conditional_t<I == 0, RE::Character*, RE::PlayerCharacter*>;

		void* _ptr;

		operator ActorType* () const noexcept { return reinterpret_cast<ActorType*>(_ptr); }
		ActorType* operator-> () const noexcept { return reinterpret_cast<ActorType*>(_ptr); }
	};


	//VTABLE
	struct GetActorValueHook
	{
		static void Patch()
		{
			//*
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[5] };
			REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[5] };

			func[0] = PlayerCharacter__Actor_VTable.write_vfunc(0x01, thunk<0>);
			func[1] = Character__Actor_VTable.write_vfunc(0x01, thunk<1>);

			logger::info("GetActorValueHook complete...");

			/*/
			
			auto hook_addr = REL::RelocationID(37517, 38462).address();//SE: 0x620D60, AE: 0x658440, VR: ???
			auto return_addr = hook_addr + RELOCATION_OFFSET(0x6, 0x5);
			
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//Preserves these instructions, then jumps to the last functional instruction.
					
					if (IsAE())
					{
						mov(r11, rsp);
						push(rbp);
						push(rsi);
					}
					else
					{
						push(rbp);
						push(rsi);
						push(rdi);
						push(r14);
					}

					

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func[0] = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk<0>);

			logger::info("GetActorValueHook complete...");
			//*/
		}
		template <int I>
		static float thunk(RE::ActorValueOwner* a_this, RE::ActorValue av)
		{
			//logger::info("AVGA hook");

			RE::Character* target = skyrim_cast<RE::Character*>(a_this);

			uint32_t raw_value = std::bit_cast<uint32_t>(av);


			if (a_this->GetIsPlayerOwner() == true && std::bit_cast<uint32_t>(RE::ActorValue::kVariable01) == raw_value) {
				int catcher = 4;
				logger::debug("catcher {} and av {}", catcher, raw_value);
			}

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);


			if (info) //raw_value == 256
			{
				auto value = info->GetExtraValue(target, ExtraValueInput::All);
				logger::debug("hit get {}, VALUE: {}", raw_value, value);
				return value;

			}
			else
			{
				//logger::debug("pass get {}", raw_value);
				return func[I](a_this, av);
			}
		}



		static inline REL::Relocation<decltype(thunk<0>)> func[2];
		//static inline REL::Relocation<decltype(thunk)> func_;
	};

	//VTABLE
	struct SetActorValueHook
	{
		//Note, hook is actually for SetBaseActorValue. There's a sub under this that effectively is only used by this.
		// If you anticipate problems hook that instead, if not keep hooking this.

		static void Patch()
		{
			//*
			
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[5] };
			REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[5] };

			func[0] = PlayerCharacter__Actor_VTable.write_vfunc(0x04, thunk<0>);
			func[1] = Character__Actor_VTable.write_vfunc(0x04, thunk<1>);

			logger::info("SetBaseActorValueHook complete...");
			

			/*/
			
			auto hook_addr = REL::RelocationID(37520, 38465).address();//SE: 0x621070, AE: 0x6587C0, VR: ???
			auto return_addr = hook_addr + RELOCATION_OFFSET(0x7, 0x9);
			
			
			struct Code : Xbyak::CodeGenerator
			{
				//template <REL::Module::Runtime Runtime>
				Code(uintptr_t ret_addr)
				{
					//Preserves these instructions, then jumps to the last functional instruction.
					if (IsAE())
					{
						cmp(edx, 0x0FFFFFFFF);
						jz("end");
					}
					else
					{
						sub(rsp, 0x38);
						cmp(edx, 0x0FFFFFFFF);
					}
					
					mov(rax, ret_addr);
					jmp(rax);

					L("end");
					ret();

				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func[0] = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk<0>);

			logger::info("SetBaseActorValueHook complete...");
			//*/
		}

		template<int I>
		static void thunk(RE::ActorValueOwner* a_this, RE::ActorValue av, float value)
		{
			//logger::info("AVBS hook");

			RE::Character* target = skyrim_cast<RE::Character*>(a_this);

			uint32_t raw_value = std::bit_cast<uint32_t>(av);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info){//raw_value == 256) {
				logger::debug("hit set {}", raw_value);
				info->SetExtraValue(target, value, RE::ACTOR_VALUE_MODIFIER::kTotal);
				//Psuedo::SetExtraValue(target, "HitsTaken", value);
			} else {
				//logger::debug("pass set {}", raw_value);
				//if (av != RE::ActorValue::kNone)
					return func[I](a_this, av, value);
			}
		}

		static inline REL::Relocation<decltype(thunk<0>)> func[2];
	};

	//NOT a vtable call.
	struct ModActorValueHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			//Use variantID at some point pls.
			//auto hook_addr = REL::ID(37523 /*0x621120*/).address();
			auto hook_addr = REL::RelocationID(37523, 38468).address();//SE: 0x621120, AE: 0x6589B0, VR: ???
			auto return_addr = hook_addr + 0x5;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//uintptr_t arg_8 = qword ptr 10h
					mov(ptr[rsp + 0x10], rbx);
					
					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			//func = (uintptr_t)code.getCode();

			//trampoline.write_branch<5>(hook_addr, thunk);
			
			//return;

			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;


			logger::info("ModActorValueHook complete...");
			//*/
		}

		static void thunk(RE::Character* a_this, RE::ACTOR_VALUE_MODIFIER a2, RE::ActorValue a3, float a4, RE::Character* actor)
		{
			//logger::debug("AVSM hook");

			uint32_t raw_value = std::bit_cast<uint32_t>(a3);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);


			if (a4 > 0 && a2 == ActorValueModifier::kDamage && a3 == RE::ActorValue::kHealth)
			{
				//logger::info("Health boost!");
				//throw nullptr;
			}

			if (info) {  //raw_value == 256) {
				logger::debug("hit mod {}, {}, val {}, who {}", raw_value, (int32_t)a2, a4, !actor ? "none" : actor->GetName());
				info->ModExtraValue(a_this, actor, a4, a2);
				//Psuedo::ModExtraValue(a_this, "HitsTaken", a4, a2);
			} else {
				//logger::debug("pass mod {}, {}, val {}, who {}", raw_value, (int32_t)a2, a4, !actor ? "none" : actor->GetName());
				return func(a_this, a2, a3, a4, actor);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	//Can become a vtable call instead.
	struct GetBaseActorValueHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			//*
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[5] };
			REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[5] };

			func[0] = PlayerCharacter__Actor_VTable.write_vfunc(0x03, thunk<0>);
			func[1] = Character__Actor_VTable.write_vfunc(0x03, thunk<1>);

			logger::info("GetBaseActorValueHook Complete...");
		
			return;
			
			/*/

			auto hook_addr = REL::RelocationID(37519, 38464).address();//SE: 0x620F30, AE: 0x658680, VR: ???
			auto return_addr = hook_addr + 0x5;
			
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//uintptr_t arg_10 = qword ptr 18h;
					mov(ptr[rsp + 0x18], rbx);
					
					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			func[0] = (uintptr_t)code.getCode();

			trampoline.write_branch<5>(hook_addr, thunk<0>);
			
			logger::info("GetBaseActorValueHook Complete...");
			//*/
		}

		template <int I>
		static float thunk(RE::ActorValueOwner* a_this, RE::ActorValue a2)
		//static float thunk(void* a_this, RE::ActorValue a2)
		{
			//logger::debug("AVBG hook");

			//using CharacterType = std::conditional_t<I == 0, RE::Character, RE::PlayerCharacter>;
			using CharacterType = RE::Character;

			CharacterType* target = skyrim_cast<CharacterType*>(a_this);
			//CharacterType* target = skyrim_cast<CharacterType*>(a_this);

			uint32_t raw_value = std::bit_cast<uint32_t>(a2);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info) {  //raw_value == 256) {
				auto value = info->GetExtraValue(target, ExtraValueInput::Base);
				//auto value = Psuedo::GetExtraValue(target, "HitsTaken", ExtraValueInput::Base);
				logger::debug("hit base {}, val {}", raw_value, value);

				return value;
			} 
			else 
			{
				auto value = func[I](a_this, a2);
				//logger::debug("pass base {}, val {}", raw_value, value);
				
				return value;
			}
		}

		static inline REL::Relocation<decltype(thunk<0>)> func[2];
		//static inline REL::Relocation<decltype(thunk<0>)> func[2];
	};

	
	struct GetActorValueModifierHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			//Use variantID at some point pls.
			//auto hook_addr = REL::ID(37524 /*0x621350*/).address();
			auto hook_addr = REL::RelocationID(37524, 38469).address();//SE: 0x621350, AE: 0x658BD0, VR: ???
			auto return_addr = hook_addr + 0x6;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//AE/SE versions remain the same
					push(rbx);
					sub(rsp, 0x20);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
			
			auto& trampoline = SKSE::GetTrampoline();

			//func = (uintptr_t)code.getCode();

			//trampoline.write_branch<5>(hook_addr, thunk);
			
			//return;
			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;

			logger::info("GetActorValueModifier Hook complete...");
			//*/
		}

		static float thunk(RE::Character* a_this, RE::ACTOR_VALUE_MODIFIER a2, RE::ActorValue a3)
		{
			//logger::debug("AVGM hook");
			uint32_t raw_value = std::bit_cast<uint32_t>(a3);

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByValue(raw_value);

			if (info) {  //raw_value == 256) {
				//auto value = Psuedo::GetExtraValue(a_this, "HitsTaken", a2);
				auto value = info->GetExtraValue(a_this, a2);
				logger::debug("hit mdfr {}, modifier {}, val {}", raw_value, a2, value);

				return value;
			} 
			else 
			{
				//return func(a_this, a2, a3);
				auto value = func(a_this, a2, a3);
				//logger::debug("pass mdfr {}, modifier {}, val {}", raw_value, a2, value);

				return value;
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	//Use manual rewrite.
	struct GetActorValueIDFromNameHook
	{
		static void Patch()
		{
			//Use variantID at some point pls.

			
			auto hook_addr = REL::RelocationID(26570, 27203).address();//SE: 0x3E1450, AE: 0x3FC5A0, VR: ???
			auto return_addr = hook_addr + 0x5;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//SE/AE preservation not required
					//uintptr_t arg_0 = qword ptr 8;
					mov(ptr[rsp + 0x8], rbx);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };
		
			auto& trampoline = SKSE::GetTrampoline();

			//func = (uintptr_t)code.getCode();

			//trampoline.write_branch<5>(hook_addr, thunk);
			
			//return; 

			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;


			logger::info("GetActorValueIDFromNameHook complete...");
			//*/
		}

		static RE::ActorValue thunk(char* av_name)
		{
			//logger::debug("AVID hook");
			//Would like strcmp with case insensitivity
			/*
			int i = 0;

			while ()
			{
				char& letter = av_name
			}
			//*/

			//I actually think the other should go first. Would prevent overriding too.
			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByName(av_name);

			if (info) {//Utility::StrCmpI(av_name, "HitsTaken") == true) {
				ValueID id = info->GetValueID();
				logger::debug("EV Queried at  {}", id);
				return static_cast<RE::ActorValue>(id);
			}
			else {
				return func(av_name);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	//Deprecated
	struct ActorUpdateHook
	{
		//I hate this method of putting stuff on the main thread like this
		//If I can, I'd rather hook the update function for avs, or whatever calls that.
		static void Patch()
		{
			REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[0] };
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[0] };

			func[0] = Character__Actor_VTable.write_vfunc(0xAD, thunk<0>);
			func[1] = PlayerCharacter__Actor_VTable.write_vfunc(0xAD, thunk<1>);

			logger::info("ActorUpdate Hook complete...");
		}
		//The main purpose of this function shouldn't be to to update directly, I was thinking it should be to update states primarily?
		template <unsigned int I = 0>
		//static void thunk(RE::Character* a_this, float a2)
		static void thunk(std::conditional_t<I == 0, RE::Character*, RE::PlayerCharacter*> a_this, float a2)
		{
			func[I](a_this, a2);
			

			if (a2 == 0)
				a2 = Utility::GetDeltaTime();

			//A question remains if I would like to handle this by recording the last time it was updated, instead of just respecting
			// delta times alone. I'll decide later.

			//Note, this is not how this shit would be going down.
			
			//This would never create, if it doesn't exist, no damage can exist.
			ExtraValueStorage* value_storage = ExtraValueStorage::GetStorage(a_this);

			if (!value_storage) {
				//if (a_this->IsPlayerRef())
				//	logger::debug("A {}", value_storage != nullptr);
				
				return;
			}
		
			value_storage->Update(a_this, a2);
		}

		static inline REL::Relocation<decltype(thunk<0>)> func[2];
	};


	struct RecalculateLeveledActorHook
	{
		static void Patch()
		{
			//return;
			// 
			//SE: 0x5D57B0, AE: 0x60C1D0, VR:???
			REL::Relocation<uintptr_t> RecalculateLeveledActor{ REL::RelocationID { 36333, 37323 }, 0x51 };
			
			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(RecalculateLeveledActor.address(), thunk);
			
			logger::info("RecalculateLeveledActor Hook complete...");
		}
		
		
		static DWORD thunk(RE::TESObjectREFR* a_this, void* a2, void* a3)
		{
			//I think I'll want to do this if it's a dynamic form. Because that means it's relinquishing it's current form.

			//BUUUUUUUUUT that's what it does when it runs for the first time. So I'm just going to have to see.

			//This seems to only really happen when leveled actors come into existence, or when they're reset (going to test that bit later).
			// I could check if I actually need to remove if the base object is a leveled one (IE, not inited yet). Least I think how it works.
			//logger::debug("REFR hook");
			
			if (ExtraValueStorage::RemoveStorage(a_this->formID) == true)
				logger::debug("PLACEHOLDER: Removing {}, FormID {:08X}", a_this->GetName(), a_this->formID);
			
			return func(a_this, a2, a3);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};


	struct ActorValueUpdateHook
	{
		static void Patch()
		{
			auto& trampoline = SKSE::GetTrampoline();
 
			//SE: 0x620610, AE: 0x657D40, VR:??????
			REL::Relocation<uintptr_t> ActorValueUpdate{ REL::RelocationID { 37509, 38451 }, 0x19 };

			//func = trampoline.write_call<5>(ResetReference.address(), (uintptr_t)code.getCode());
			func = trampoline.write_call<5>(ActorValueUpdate.address(), thunk);

			logger::info("ActorValueUpdate Hook complete...");
		}


		static int32_t thunk( RE::Character* a_this, RE::ActorValue a2, float a3)
		{
			//I think I'll want to do this if it's a dynamic form. Because that means it's relinquishing it's current form.

			//BUUUUUUUUUT that's what it does when it runs for the first time. So I'm just going to have to see.

			//This seems to only really happen when leveled actors come into existence, or when they're reset (going to test that bit later).
			// I could check if I actually need to remove if the base object is a leveled one (IE, not inited yet). Least I think how it works.
			//logger::debug("REFR hook");
			
			//Currently, this doesn't seek to forcibly create a new storage unless interacted with, I'm thinking that there can be a setting that controls
			// this, that one can include in their settings. That way, even if one mod REALLY needs them to be loaded before any kind of interaction, 
			// I can get it. 
			//But for now, this.
			ExtraValueStorage* value_storage = ExtraValueStorage::GetStorage(a_this);

			if (!value_storage) {
				goto retn;
			}

			value_storage->Update(a_this, a3);
			
		retn:
			return func(a_this, a2, a3);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};



	struct MagicItemCtorHook
	{
		static void Patch()
		{
			REL::Relocation<uintptr_t> ctor_hook{ REL::RelocationID { 11171, 11278 }, 0x20 };
			auto hook_addr = REL::RelocationID(33817, 34609).address();//SE: 0x1004C0 , AE: 0x10C140, VR: ???

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(ctor_hook.address(), thunk);

			logger::info("MagicItemCtor Hook complete...");
			//*/
		}

		static RE::TESBoundObject* thunk(RE::TESBoundObject* a_this)
		{
			//I believe this is the hook, but the parameters might not be right.
			auto* magic_item = static_cast<RE::MagicItem*>(a_this);

			if (!magic_item)
				goto end;

			//logger::debug("As bound {:X}, as magic {:X}", (uintptr_t)a_this, (uintptr_t)magic_item);

			magic_item->pad74 = 0;
			magic_item->pad84 = 0;

			end:
			return func(a_this);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	//Override call. There are few and far between calls in here, so I think I'll likely just want to remake this.
	struct GetActorValueForCostHook
	{
		//
		//Note, hook is actually for SetBaseActorValue

		static void Patch()
		{
			auto hook_addr = REL::RelocationID(33817, 34609).address();//SE: 0x556780, AE: 0x5792A0, VR: ???
			auto return_addr = hook_addr + 0x5;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					//uintptr_t arg_0 = qword ptr 08h
					mov(ptr[rsp + 0x08], rbx);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;


			logger::info("GetActorValueForCostHook complete...");
			//*/
		}

		static RE::ActorValue thunk(RE::MagicItem* a_this, int32_t right_hand)
		{
			//GetCostCannotCastReason 140556810. Determines if it's being out of magicka

			auto result = func(a_this, right_hand);
			
			if (!a_this)
				return result;


			uint32_t value = right_hand ? a_this->pad74 : a_this->pad84;

			uint32_t pad1 = a_this->pad74;
			uint32_t pad2 = a_this->pad84;

			result = !value ? result : static_cast<RE::ActorValue>(value);

			logger::debug("TEST PAD OF {} {} and {}, result {}", a_this->formID, pad1, pad2, (int)result);

			return result;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};



	//Deprecated
	template <class... PatchTypes> requires(sizeof...( PatchTypes) >  0)
	struct ReadFromFileStreamHook
	{
		using Args = std::tuple<PatchTypes...>;
		
		static constexpr size_t PatchSize = sizeof...(PatchTypes);

		template<size_t I>
		using FormType = std::tuple_element<I, Args>::type;


		template <size_t I>
		static void NthPatch()
		{

			using FormClass = FormType<I>;
			
			REL::Relocation<uintptr_t> FormType__TESForm_VTable = REL::Relocation<uintptr_t>{ FormClass::VTABLE[0] };

			//func[I] = FormType__TESForm_VTable.write_vfunc(0x06, thunk<I>);
			std::get<I>(func) = FormType__TESForm_VTable.write_vfunc(0x06, thunk<FormClass, I>);

			logger::info("Created Nth patch for {} at {}", typeid(FormClass).name(), I);

			if constexpr (I > 0)
				NthPatch<I - 1>();
		}


		//I hate this method of putting stuff on the main thread like this
		static void Patch()
		{
			//What I can do is, this thing makes an array of relocations with me making a parameter pack of types, then once this parameter
			// back of types is established I can basically just do Type::VTABLE[0]..., then iterate through all of them. Actually, no reason
			// I can do that like that here right?
			//I'm still gonna hold off because pulling out the old compile time looper would be a pain.

			NthPatch<PatchSize - 1>();

			//REL::Relocation<uintptr_t> EffectSetting__TESForm_VTable{ RE::VTABLE_EffectSetting[0] };

			//func = EffectSetting__TESForm_VTable.write_vfunc(0x06, thunk);
		}
		

		template<class ClassType, size_t I>
		static bool thunk(void* a1, RE::TESFile* a2)
		{
			using FormClass = FormType<I>;

			if constexpr (std::is_same_v<FormClass, void>)
				return false;
			else 
			{
				FormClass* a_this = reinterpret_cast<FormClass*>(a1);

				auto result = std::get<I>(func)(a_this, a2);

				
				if (!result || !a2) {
					return result;
				}

				FormExtraValueHandler::AddUnrepresentedForm(a_this);

				return result;
			}
			
		}
		
		//If I can, find some way to simpli
		static inline std::tuple<REL::Relocation<decltype(thunk<PatchTypes, 0>)>...> func;
	};

	template <class... PatchTypes> requires(sizeof...(PatchTypes) > 0)
	struct InitializeAfterAllFormsReadHook
	{
		using Args = std::tuple<PatchTypes...>;

		static constexpr size_t PatchSize = sizeof...(PatchTypes);

		template<size_t I>
		using FormType = std::tuple_element<I, Args>::type;


		template <size_t I>
		static void NthPatch()
		{

			using FormClass = FormType<I>;

			REL::Relocation<uintptr_t> FormType__TESForm_VTable = REL::Relocation<uintptr_t>{ FormClass::VTABLE[0] };

			//func[I] = FormType__TESForm_VTable.write_vfunc(0x06, thunk<I>);
			std::get<I>(func) = FormType__TESForm_VTable.write_vfunc(0x13, thunk<FormClass, I>);

			logger::info("Created Nth patch for {} at {}", typeid(FormClass).name(), I);

			if constexpr (I > 0)
				NthPatch<I - 1>();
		}


		//I hate this method of putting stuff on the main thread like this
		static void Patch()
		{
			//What I can do is, this thing makes an array of relocations with me making a parameter pack of types, then once this parameter
			// back of types is established I can basically just do Type::VTABLE[0]..., then iterate through all of them. Actually, no reason
			// I can do that like that here right?
			//I'm still gonna hold off because pulling out the old compile time looper would be a pain.

			NthPatch<PatchSize - 1>();

			//REL::Relocation<uintptr_t> EffectSetting__TESForm_VTable{ RE::VTABLE_EffectSetting[0] };

			//func = EffectSetting__TESForm_VTable.write_vfunc(0x06, thunk);
		}


		template<class ClassType, size_t I>
		static bool thunk(void* a1)
		{
			using FormClass = FormType<I>;

			if constexpr (std::is_same_v<FormClass, void>)
				return false;
			else
			{
				FormClass* a_this = reinterpret_cast<FormClass*>(a1);

				auto result = std::get<I>(func)(a_this);


				if (!result) {
					return result;
				}

				FormExtraValueHandler::AddUnrepresentedForm(a_this);

				return result;
			}

		}

		//If I can, find some way to simpli
		static inline std::tuple<REL::Relocation<decltype(thunk<PatchTypes, 0>)>...> func;
	};


	//Deprecated
	//template <class... PatchType>
	struct EffectSetting_InitializeAfterAllFormsReadHook
	{
		//I hate this method of putting stuff on the main thread like this
		static void Patch()
		{
			//What I can do is, this thing makes an array of relocations with me making a parameter pack of types, then once this parameter
			// back of types is established I can basically just do Type::VTABLE[0]..., then iterate through all of them. Actually, no reason
			// I can do that like that here right?
			//I'm still gonna hold off because pulling out the old compile time looper would be a pain.

			REL::Relocation<uintptr_t> EffectSetting__TESForm_VTable{ RE::VTABLE_EffectSetting[0] };

			//It's possible the vtable has changed between versions. For 
			func = EffectSetting__TESForm_VTable.write_vfunc(0x13, thunk);
		}

		static void thunk(RE::EffectSetting* a_this)
		{
			//A big note is this will have to have another function handle processing, I want to template a lot of this design
			// and that won't lend well to specification.
			
				RE::ActorValue primary = a_this->data.primaryAV;
			
				RE::ActorValue secondary = a_this->data.secondaryAV;

				RE::ActorValue resist = a_this->data.resistVariable;

				func(a_this);

				//need an "if owned value set, if not don't" function, but I'll do that later.

				a_this->data.primaryAV = primary;
				a_this->data.secondaryAV = secondary;
				a_this->data.resistVariable = resist;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	struct UnarmedWeaponSpeedHook
	{
		static void Patch()
		{
			REL::Relocation<uintptr_t> WeaponSpeedHook{ REL::RelocationID { 41694, 42779 }, 0x29 };//0x71B670, 0x758510

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(WeaponSpeedHook.address(), thunk);

			logger::info("WeaponSpeedUpdate Hook complete...");
		}

		//This hook is so ununique btw, that I think I can just write branch this shit. Straight up.
		static std::int64_t thunk(RE::ActorValueOwner* av_owner, RE::TESObjectWEAP* weap, std::int64_t unk3)
		{
			//SE: 0x2EFF868, AE: 0x2F99450
			//static uintptr_t fists = REL::RelocationID(514923, 401061).address();

			static RE::TESObjectWEAP* fists = RE::TESForm::LookupByID<RE::TESObjectWEAP>(0x1F4);

			if (!weap)
				weap = fists;//reinterpret_cast<RE::TESObjectWEAP*>(fists);

			return func(av_owner, weap, unk3);
		}

		static inline REL::Relocation<decltype(thunk)> func;
		};


	struct FirstTestEquipHook
	{

		static void Patch()
		{


			auto hook_addr = REL::ID(37938).address();//0x637a80

			//before anything else, I'll be turning the first operation, rather all the first operations into no ops to see if it sticks

			//I shouldn't need to pattern match for this to function. Hopefully.
			REL::safe_write(hook_addr, &Utility::NoOperation9[0], 0x9);

			auto return_addr = hook_addr + 0x9;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					test(rdx, rdx);
					jz("end");



					mov(rax, ret_addr);
					jmp(rax);

					L("end");
					ret();
					//*/
				}
			} static code{ return_addr };
			//auto size = code.getSize();
			auto& trampoline = SKSE::GetTrampoline();
			//auto result = trampoline.allocate(size);
			//std::memcpy(result, code.getCode(), size);
			//trampoline.write_branch<5>(hook_addr, (std::uintptr_t)result);
			//func = return_addr;

			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			//if (place_query == hook_addr)
			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;

			logger::info("Place {} vs func {} vs Hook {} vs Ret {}, call placed is {}", (uintptr_t)place_query, func.address(), hook_addr, return_addr, placed_call);
		}


		//This hook is so ununique btw, that I think I can just write branch this shit. Straight up.
		static void thunk(RE::ActorEquipManager* a_this, RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraData, std::uint32_t a_count, const RE::BGSEquipSlot* a_slot, bool a_queueEquip, bool a_forceEquip, bool a_playSounds, bool a_applyNow)
		{
			logger::info("FIRST GO");
			//return true;
			//if (a_actor)//This is done to preserve the functionality to get to that point.
				return func(a_this, a_actor, a_object, a_extraData, a_count, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow);
		}
		

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct SecondTestEquipHook
	{

		static void Patch()
		{


			auto hook_addr = REL::ID(37938).address();//0x637A80
			auto return_addr = hook_addr + 0x9;
			//*
			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					test(rdx, rdx);
					jz("end");



					mov(rax, ret_addr);
					jmp(rax);

					L("end");
					ret();
					//*/
				}
			} static code{ return_addr };
			//auto size = code.getSize();
			auto& trampoline = SKSE::GetTrampoline();
			//auto result = trampoline.allocate(size);
			//std::memcpy(result, code.getCode(), size);
			//trampoline.write_branch<5>(hook_addr, (std::uintptr_t)result);
			//func = return_addr;

			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			//if (place_query == hook_addr)
			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;

			logger::info("Place {} vs func {} vs Hook {} vs Ret {}, call placed is {}", (uintptr_t)place_query, func.address(), hook_addr, return_addr, placed_call);
		}


		//This hook is so ununique btw, that I think I can just write branch this shit. Straight up.
		static void thunk(RE::ActorEquipManager* a_this, RE::Actor* a_actor, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraData, std::uint32_t a_count, const RE::BGSEquipSlot* a_slot, bool a_queueEquip, bool a_forceEquip, bool a_playSounds, bool a_applyNow)
		{
			logger::info("SECOND GO");
			//return true;
			if (a_actor)//This is done to preserve the functionality to get to that point.
				return func(a_this, a_actor, a_object, a_extraData, a_count, a_slot, a_queueEquip, a_forceEquip, a_playSounds, a_applyNow);
		}


		static inline REL::Relocation<decltype(thunk)> func;
	};


	struct Hooks
	{
		static void Install()
		{
			//return;
			
#ifdef _DEBUG
			constexpr size_t reserve = 256;
#else
			constexpr size_t reserve = 70;//98 again.
#endif

			//Simple safe measure, I don't need the space atm though.
			
			
			
			SKSE::AllocTrampoline(reserve);
			//Total hook count: 98 bytes

			//Remember to allocate the trampoline smile.

			//Here's a thought, instead of patching this shit manually, what would happen if I say, just pluck the function from it's
			// vtable to get where I'm supposed to pull from? Quite devilish, and no fox hunts. Though I would have to test for 
			// proper preservation.
			// 
			//REL::Relocation<uintptr_t> Character__ActorValueOwner_VTable{ RE::VTABLE_Character[6] };

			//Temp section
			//UnarmedWeaponSpeedHook::Patch();

			//FirstTestEquipHook::Patch();
			//SecondTestEquipHook::Patch();

			//Gets killed in second one, figure it out.
			//-end

			//ActorUpdateHook::Patch();
			ActorValueUpdateHook::Patch();
			RecalculateLeveledActorHook::Patch();

			GetActorValueHook::Patch();
			SetActorValueHook::Patch();
			ModActorValueHook::Patch();
			GetBaseActorValueHook::Patch();
			GetActorValueModifierHook::Patch();
			GetActorValueIDFromNameHook::Patch();
			
			//MagicItemCtorHook::Patch();
			//GetActorValueForCostHook::Patch();
			
			
			
			ReadFromFileStreamHook<RE::TESIdleForm, RE::BGSCameraPath, RE::TESTopicInfo>::Patch();
			//This should have less repetition and no need for a set
			// didn't work.
			//InitializeAfterAllFormsReadHook<RE::TESIdleForm, RE::BGSCameraPath, RE::TESTopicInfo>::Patch();
			
			//These are no longer included.
			//EffectSetting_InitializeAfterAllFormsReadHook::Patch();

			

			//constexpr std::uint8_t NoOperation3[0x3]{ 0x0F, 0x1F, 0x00 };
			//static_assert(sizeof(NoOperation3) == 0x3);
			
			REL::RelocationID comp_id1{ 34269, 35069 };//SE: 0x5671F0, AE: 0x58AE30, VR:???
			REL::RelocationID comp_id2{ 34271, 35071 };//SE: 0x5672C0, AE: 0x58AF00, VR:???

			
			//bool is_ae = REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_5_97) == std::strong_ordering::greater;

			uintptr_t comp_off1 = RELOCATION_OFFSET(0x3E, 0x45);
			uintptr_t comp_off2 = RELOCATION_OFFSET(0xCD, 0xD0);

			const uint8_t* op_addr = !IsAE() ? &Utility::NoOperation3[0] : &Utility::NoOperationA[0];
			size_t op_size = !IsAE() ? 0x3 : 0xA;

			//REL::Module::get().version().compare(v)
			REL::safe_write(comp_id1.address() + comp_off1, op_addr, op_size);
			REL::safe_write(comp_id2.address() + comp_off2, op_addr, op_size);
			//REL::safe_write(comp_id2.address() + 0xCD, &Utility::NoOperationA, 0xA);

			//GetActorValueHook::func = Character__ActorValueOwner_VTable.write_vfunc(0x01, GetActorValueHook::thunk);
			//SetActorValueHook::func = Character__ActorValueOwner_VTable.write_vfunc(0x07, SetActorValueHook::thunk);

			logger::info("Hooks installed.");
		}
	};
}

//GetActorValue, handled. A collective function.
//GetPermanentActorValue(GetMaximumActorValue), not yet handled. Is actually get maximum value. Name is erroneous.
//GetBaseActorValue, non-handled. Straight forward to do so.
//SetBaseActorValue, handled.
//ModActorValue(ModifyActorValue), non-handled, confused to what it actually does, name may be misleading.
//RestoreActorValue(RestoreActorValue), hook complete. Handled via 140621120. There is a non-vtable version of this. Seems to iterface well enough.
//SetActorValue, basically is SetBaseActorValue

//CheckClampDamageModifier, needs spot treating, but otherwise you know. Won't encounter for the most part because of effects. But yeah.
//GetActorValueModifier(140621350) Needs a direct hook