#pragma once

#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"
#include "FormExtraValueHandler.h"
#include "Utility.h"
#include "Addresses.h"

#include "xbyak/xbyak.h"

#include "ActorValueExtendedList.h"




namespace RE
{
	enum struct AdvanceAction
	{
		NormalUsage = 0,
		PowerAttack = 1,
		Bash = 2,
		LockpickSuccess = 3,
		LockpickBroken = 4,
	};

	//True location is StatsMenu__TreeData

	using TreeStates = StatsMenu::UnkData::State;


	

	struct TreeData
	{
	public:
		

		// members
		BSEffectShaderProperty* shader = nullptr;							// 00
		stl::enumeration<TreeStates, std::uint32_t> state = TreeStates::kResting;		// 08
		std::uint32_t                          timestampMS = 0;		// 0C

		//StatsMenu::UnkData
		TreeData() = default;
		TreeData(const StatsMenu::UnkData& other)
		{
			shader = other.unk00;
			state = other.state;
			timestampMS = other.unk0C;
		}
	};
	static_assert(sizeof(TreeData) == 0x10);
}




namespace AVG
{
	std::vector<RE::TreeData> fakeTreeList{ 19 };


	
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
			//REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[0] };
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[0] };

			//func[0] = Character__Actor_VTable.write_vfunc(0xAD, thunk<0>);
			func[1] = PlayerCharacter__Actor_VTable.write_vfunc(0xAD, thunk<1>);

			logger::info("ActorUpdate Hook complete...");
		}
		static void Do()
		{
			//BSKeyboardDevice::Keys::kE
			auto device = RE::BSInputDeviceManager::GetSingleton()->devices[RE::INPUT_DEVICES::kKeyboard];

			if (device)
			{
				auto keyboard = skyrim_cast<RE::BSWin32KeyboardDevice*>(device);

				if (keyboard)
				{
					auto it = keyboard->deviceButtons.find(RE::BSKeyboardDevice::Keys::kE);

					if (it != keyboard->deviceButtons.end())
					{
						it->second->heldDownSecs = 0;
					}
				}
			}
		}
		
		//The main purpose of this function shouldn't be to to update directly, I was thinking it should be to update states primarily?
		template <unsigned int I = 0>
		//static void thunk(RE::Character* a_this, float a2)
		static void thunk(std::conditional_t<I == 0, RE::Character*, RE::PlayerCharacter*> a_this, float a2)
		{

			Do();

			return func[I](a_this, a2);
			

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

			if (value_storage) {
				value_storage->Update(a_this, a3);
			}

			return func(a_this, a2, a3);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};


	void GetFullname(RE::BSFixedString& a1, RE::TESFile* a2)
	{
		//1BB110
		using func_t = decltype(&GetFullname);
		REL::Relocation<func_t> func{ RELOCATION_ID(15296, 0) };
		return func(a1, a2);
	}


	int64_t ItsRewindTime(RE::TESFile* a1)
	{
		//17D0A0
		using func_t = decltype(&ItsRewindTime);
		REL::Relocation<func_t> func{ RELOCATION_ID(13893, 0) };
		return func(a1);
	}

	//VTABLE
	struct SpellItem_GetSkillUsageDataHook
	{
		//Note, hook is actually for SetBaseActorValue. There's a sub under this that effectively is only used by this.
		// If you anticipate problems hook that instead, if not keep hooking this.

		static void Patch()
		{
			//*

			REL::Relocation<uintptr_t> SpellItem_VTable{ RE::SpellItem::VTABLE[0]};
		

			func = SpellItem_VTable.write_vfunc(0x60, thunk);

			logger::info("SpellItem_GetSkillUsageData hook complete...");
		}

		
		static bool thunk(RE::SpellItem* a_this, RE::MagicItem::SkillUsageData& a2)
		{
			bool result = func(a_this, a2);

			if (!result && a_this->GetSpellType() != RE::MagicSystem::SpellType::kVoicePower) {
				//Double check the skill use data for it being skillful
				if (a2.skill > RE::ActorValue::kTotal) {
					//Need to check the value for being a skill.
					result = true;
				}
			}

			return result;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	using PlayerSkills = RE::PlayerCharacter::PlayerSkills;


	inline void AdvanceSkill_Impl(PlayerSkills* a_this, RE::ActorValue a2, float a3, RE::TESBoundObject* a4,
		RE::AdvanceAction a5, bool a6, bool a7)
	{
		//SE: 0x6E61D0, AE: 0x720F40, VR: ???
		using func_t = decltype(&AdvanceSkill_Impl);
		REL::Relocation<func_t> func{ RELOCATION_ID(40554, 41561) };
		return func(a_this, a2, a3, a4, a5, a6, a7);
	}

	float GetXPFromSkillRank(float skill)
	{
		static RE::Setting* fXPPerSkillRank = RE::GameSettingCollection::GetSingleton()->GetSetting("fXPPerSkillRank");

		//I'mma be real, I'm not trying to sort this shit out one bit.

		float v1;
		float v3;
		float v4;
		float v5;
		float v6;

		v1 = skill + 1.0;
		v3 = (((skill + 1.0) + 1.0) * (skill + 1.0)) * 0.5;
		v4 = v3;

		if ((v3 - v4) < 0.0)
		{
			v4 = v4 - 1.0;
		}
		v5 = (v1 * skill) * 0.5;
		v6 = v5;
		if ((v5 - v6) < 0.0)
		{
			v6 = v6 - 1.0;
		}
		return (v4 - v6) * fXPPerSkillRank->GetFloat();
	}


	//Prologue
	struct AdvanceSkillHook
	{
		static void Patch()
		{
			auto hook_addr = REL::RelocationID(40554, 41561).address();//SE: 0x6E61D0, AE: 0x720F40, VR: ???

			auto return_addr = hook_addr + (!IsAE() ? 0x5 : 0x6);

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					bool ae = IsAE();

					mov(ae ? rax : r11, rsp);
					push(ae ? rdi : rbp);
					push(ae ? r12 : rsi);
					mov(ae ? r11 : rax, ret_addr);
					jmp(ae ? r11 : rax);
				}
			} static code{ return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			auto placed_call = IsCallOrJump(hook_addr) > 0;

			auto place_query = trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);

			if (!placed_call)
				func = (uintptr_t)code.getCode();
			else
				func = place_query;

			logger::info("AdvanceSkill Hook complete...");
			//*/
		}


		//rcx : PlayerSkills
		//edx :actorvalue
		//xmm2 :value
		//r9 : TESBoundObject*
		//-> : AdvanceAction
		//-> : UseSkillMult
		//-> : printMessage (Something message related
		//char, char char


		using PlayerSkills = RE::PlayerCharacter::PlayerSkills;

		static void thunk(PlayerSkills* a_this, RE::ActorValue skill, float value, RE::TESBoundObject* form, 
			RE::AdvanceAction action, bool use_skill_mult, bool hide_message)
		{
			//TODO: Make this a function, move these settings

			static RE::Setting* sSkillIncreased = RE::GameSettingCollection::GetSingleton()->GetSetting("sSkillIncreased");
			static RE::Setting* fSkillUseCurve = RE::GameSettingCollection::GetSingleton()->GetSetting("fSkillUseCurve");
			

			if (skill > RE::ActorValue::kTotal) {
				logger::info("Catching skill input, {} {} {} {} {} {}", (int)skill, value, form ? form->GetName() : "<null>",
					magic_enum::enum_name(action), use_skill_mult, hide_message);



				auto player = RE::PlayerCharacter::GetSingleton();

				float base = player->AsActorValueOwner()->GetBaseActorValue(skill);

				constexpr float maximum = 100.f;

				if (base >= maximum) {
					return;
				}
				


				ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByAV(skill);

				auto skill_info = info->GetSkillInfo();

				if (!skill_info || !skill_info->isAdvance){
					return;
				}

				PlayerStorage* storage = PlayerStorage::GetAsPlayable(false);

				

				auto& skill_data = storage->GetSkillData(info->GetDataID());

				float increase;
				if (use_skill_mult) {
					increase = (skill_info->useMult * value) + skill_info->useOffset;
				}
				else
				{
					increase = value;
				}



				auto& actionObject = REL::RelocateMember<RE::TESBoundObject*>(player, 0x9F0, 0x10F0);
				auto& actionAction = REL::RelocateMember<RE::AdvanceAction>(player, 0xAEC, 0x11EC);
				auto& advanceSkill = REL::RelocateMember<RE::ActorValue>(player, 0xAE8, 0x11E8);


				actionObject = form;
				actionAction = action;
				advanceSkill = skill;

				RE::BGSEntryPoint::HandleEntryPoint(RE::BGSEntryPoint::ENTRY_POINT::kModSkillUse, player, &increase);


				actionObject = nullptr;
				actionAction = RE::AdvanceAction::NormalUsage;
				advanceSkill = RE::ActorValue::kNone;

				skill_data.xp += increase;

				bool leveled = false;

				//This may not be the best way to do things, but for now.
				while (skill_data.levelThreshold && skill_data.xp >= skill_data.levelThreshold)
				{
					leveled = true;
					
					auto prev = base;

					player->AsActorValueOwner()->SetActorValue(skill, base += skill_info->increment);
					//player->AsActorValueOwner()->ModActorValue(skill, 1.0f);
				
					skill_data.levelThreshold = base < maximum ? (std::pow(base, fSkillUseCurve->GetFloat()) * skill_info->improveMult) + skill_info->improveOffset : 0.f;

					if (skill_info->grantsXP)
					{
						float exp = GetXPFromSkillRank(prev);

						a_this->data->xp += exp;
					}

					//As you can see the level is completely unhandled right now. Because I genuinely, can't be arsed.

					/*
					struct 

					int32_t args[4];

					args[0] = (int32_t)skill;
					
					
					REL::Relocation<uint32_t> index{ REL::RelocationID(508420, 000).address() };

					RE::BGSStoryEventManager::GetSingleton()->AddEvent(index.get(), args);
					//*/
					
					//694650
					//REL::Relocation<void(RE::PlayerCharacter*, RE::ActorValue)> Levelup{ REL::RelocationID(39227, 000).address() };
					//Levelup(player, skill);
				}


				if (leveled && !hide_message)
				{
					//v23 = off_141DE6778;
					//player->vftable_ActorValueOwner_B0->GetBaseActorValue_18(&player->vftable_ActorValueOwner_B0, a1);
					//LODWORD(v24) = sub_1403E1130(a1);
					//f_snprintf_1401423D0(Buffer, 200i64, v23, v24);
					
					char buffer[200]{};
					
					
					auto name = info->GetDisplayName();
					
					//This needs to use print properly, mainly sprintf. Also, it prints too much. Rather, it prints all the time. Only do when it levels.
					



					std::sprintf(buffer, sSkillIncreased->GetString(), name.data(), (int)base);
					//auto print = std::vformat(sSkillIncreased->GetString(), std::make_format_args(name, base));
					
					//SE: 880160, AE: 8C25B0
					REL::Relocation<void(int32_t, const char*, RE::TESQuest*, uint64_t)> SendEventIGuess{ REL::RelocationID(50751, 51646).address() };
					
					//SendEventIGuess(20, print.c_str(), nullptr, 0);
					SendEventIGuess(20, buffer, nullptr, 0);
				}



				//REL::RelocateMember<RE::TESBoundObject*>(player, 0x9F0, 0x10F0) = load ? EPModSkillUsage_AdvanceObjectHasKeyword : nullptr;
				//player->GetPlayerRuntimeData().

				//skill_info->

				//skill_data.
				//player_skill->

				return;
			}
				
			func(a_this, skill, value, form, action, use_skill_mult, hide_message);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	//Prologue
	struct IncrementSkillHook
	{
		static void Patch()
		{
			auto hook_addr = REL::RelocationID(40555, 41562).address();//SE: 0x6E64D0, AE: 0x721330, VR: ???

			auto return_addr = hook_addr + 0x6;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t ret_addr)
				{
					mov(r11, rsp);
					push(rbp);
					push(r14);

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

			logger::info("AdvanceSkill Hook complete...");
			//*/
		}




		static void thunk(PlayerSkills* a_this, RE::ActorValue skill, uint32_t inc)
		{
			if (skill <= RE::ActorValue::kTotal) {
				return func(a_this, skill, inc);
			}

			logger::info("Capturing skill increment, {} {}", (int)skill, inc);


			auto player = RE::PlayerCharacter::GetSingleton();

			ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByAV(skill);

			auto skill_info = info->GetSkillInfo();

			if (!skill_info || !skill_info->isAdvance) {
				return;
			}

			PlayerStorage* storage = PlayerStorage::GetAsPlayable(false);



			auto& skill_data = storage->GetSkillData(info->GetDataID());

			float mult = 0.0f;
					
			if (auto& threshold = skill_data.levelThreshold; threshold != 0.0f)
			{
				mult = skill_data.xp / threshold;
			}
				
			if (inc)
			{
				while (inc--){
					AdvanceSkill_Impl(a_this, skill, skill_data.levelThreshold - skill_data.xp, nullptr, RE::AdvanceAction::NormalUsage, false, inc);
				} 
			}
			skill_data.xp += mult * skill_data.levelThreshold;
			
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};





	struct GetActorValueInfoHook
	{
		static void Patch()
		{
			auto hook_addr = REL::RelocationID(26569, 27202).address();//SE: 0x3E1420, AE: 0x3FC570, VR: ???
		
			auto& trampoline = SKSE::GetTrampoline();
		
			trampoline.write_branch<5>(hook_addr, (uintptr_t)thunk);


			logger::info("GetActorValueInfoHook complete...");
		}

		static RE::ActorValueInfo* thunk(RE::ActorValue actor_value)
		{
			if (ExtraValueInfo::Finished() == false){
				if (RE::ActorValue::kTotal <= actor_value)
					return nullptr;
			}
			else if (ExtraValueInfo::GetCountAV() <= (uint32_t)actor_value)
				return nullptr;
			
			RE::ActorValueInfo** actorValues = RE::ActorValueList::GetSingleton()->actorValues;

			return actorValues[std::to_underlying(actor_value)];
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};





	//These sorely need some common ground to leech on. Like the tag

	struct MagicItemCtorHook
	{
		static void Patch()
		{
			//SE: 0x1004C0 , AE: 0x10C140, VR: ???
			REL::Relocation<uintptr_t> ctor_hook{ REL::RelocationID { 11171, 11278 }, 0x20 };

			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(ctor_hook.address(), thunk);

			logger::info("MagicItemCtor Hook complete...");
			//*/
		}

		static RE::TESBoundObject* thunk(RE::TESBoundObject* a_this)
		{
			//I believe this is the hook, but the parameters might not be right.
			auto* magic_item = static_cast<RE::MagicItem*>(a_this);//??

			if (magic_item)
			{
				Utility::GetCostSetting(magic_item, kRightHand) = RE::ActorValue::kNone;
				Utility::GetCostSetting(magic_item, kLeftHand) = RE::ActorValue::kNone;

			}

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
			//SE: 0x556780, AE: 0x5792A0, VR: ???
			auto hook_addr = REL::RelocationID(33817, 34609).address();
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

			if (auto test = Utility::GetCostSetting(a_this, right_hand); test != RE::ActorValue::kNone)
				result = test;


			return result;

			uint32_t value = right_hand ? a_this->pad74 : a_this->pad84;

			uint32_t pad1 = a_this->pad74;
			uint32_t pad2 = a_this->pad84;

			result = value != (uint32_t)RE::ActorValue::kNone ? result : static_cast<RE::ActorValue>(value);


			return result;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};



	//I think initialize form is probably better to use.
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
	

	struct Changable_InitAfterAllFileHook
	{
		static void Install()
		{

			REL::Relocation<uintptr_t> TESObjectBOOK__TESForm_VTable = REL::Relocation<uintptr_t>{ RE::TESObjectBOOK::VTABLE[0] };

			func = TESObjectBOOK__TESForm_VTable.write_vfunc(0x13, thunk);

		}


		
		static void thunk(RE::TESObjectBOOK* a1)
		{
			
			
			func(a1);

			//I really wish I didn't have to do this, but it seems there's no other sane option than to just do it again.
			if (FormExtraValueHandler::Initialized() == true) {
				//My hand has been pushed. I will resolve this shit later.
				FormExtraValueHandler::ProcessForm(a1);
			}

		}

		//If I can, find some way to simpli
		static inline REL::Relocation<decltype(thunk)> func;
	};


	//I think I should use this inste
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


	struct MagicMenu_AssociatedSkillHook
	{
		static void Install()
		{
			
			//SE: 0x89A7F0, AE: 0x8DD700, VR: ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID{51143, 52023}, REL::VariantOffset{0x67, 0x50, 0x67} };

			auto& tramp = SKSE::GetTrampoline();



			//TODO: Pattern make and if so, call func instead of oroginal.
			//FF  90 30 03 00 00

			//OR

			//TODO: Fish out all the associated skill hook locations, seems it's used in a few places. Maybe make a way to expand the possible entries it's
			//allowed to have.

			func = tramp.write_call<6>(hook.address(), thunk);
		}

		static RE::ActorValue thunk(RE::MagicItem* a_this)
		{

			//The original function is basically trash unless someone is doing a similar hook, so we prefer
			// to call the vfunc ourselves.
			auto result = a_this->GetAssociatedSkill();
			
			//return RE::ActorValue::kDestruction;

			
			auto info = ExtraValueInfo::GetValueInfoByAV(result);

			if (info) {
				//This info is a kinda proxy, setting it up with what it's supposed to replace now. This is a very temporary solution though, bare in mind.
				result = info->GetAliasID();
			}

			return result;
		}

		inline static REL::Relocation<decltype(thunk)> func;

	};



	//VTable
	struct InitValuesHook
	{
		static void Patch()
		{
			REL::Relocation<uintptr_t> Character__Actor_VTable{ RE::VTABLE_Character[0] };
			REL::Relocation<uintptr_t> PlayerCharacter__Actor_VTable{ RE::VTABLE_PlayerCharacter[0] };


			func[0] = Character__Actor_VTable.write_vfunc(0x118, thunk<0>);
			func[1] = PlayerCharacter__Actor_VTable.write_vfunc(0x118, thunk<1>);

			logger::info("InitValuesHook complete...");
		}

		
		static void Do(RE::Actor* a_this)
		{
			ExtraValueStorage* value_storage = ExtraValueStorage::GetStorage(a_this);

			if (!value_storage) {
				return;
			}
			logger::debug("Resetting actor '{}'.", a_this->GetDisplayFullName());

			value_storage->ResetStorage(a_this);
		}

		template <unsigned int I = 0>
		static void thunk(std::conditional_t<I == 0, RE::Character*, RE::PlayerCharacter*> a_this)
		{
			func[I](a_this);

			Do(a_this);
		}

		static inline REL::Relocation<decltype(thunk<0>)> func[2];
	};




	struct SkillCheckPatch
	{
		enum Register1
		{
			eax = 0xF8,
			ebp = 0xFD,
			r8b = 0xff,
			esi = 0xFE,
		};

		enum Register2
		{
			r8d = 0xF8,
			r10d = 0xFA,
		};

		static std::array<uint8_t, 3> GetInstruction(Register1 reg = eax)
		{
			return std::array<uint8_t, 3>{ 0x83, (uint8_t)reg, 0xFA };
		}

		static bool HandlePatch(uintptr_t address, Register1 reg = eax)
		{
			//83 XX 11
			if (REL::make_pattern<"83 F8 11">().match(address) == true) {
				//cmp reg, -8
				std::array<uint8_t, 3> instruction{ 0x83, (uint8_t)reg, 0xF8 };

				REL::safe_write(address, &instruction, 3);

				return true;
			}
			return false;

		}
		static bool HandlePatch(uintptr_t address, Register2 reg)
		{
			//41 83 XX 11
			if (REL::make_pattern<"41 83 FA 11">().match(address) == true) {
				//cmp reg, -8
				std::array<uint8_t, 4> instruction{ 0x41, 0x83, (uint8_t)reg, 0xF8 };

				REL::safe_write(address, &instruction, 4);

				return true;
			}
			return false;

		}


		static void Patch()
		{
			bool se = !IsAE();

			//SE: 0x0F8630, AE: 0x1044E0, VR:???
			REL::Relocation<std::uintptr_t>ActiveEffect__GetCost{ REL::RelocationID{ 10929, 11017 }, 0x1D };

			//SE: 0x1057D0, AE: 0x111650, VR:???
			REL::Relocation<std::uintptr_t>SpellItem__AdjustCost{ REL::RelocationID{ 11356, 11494 }, 0x29 };

			//SE: 0x3BDF20, AE: 0x3D7A40, VR:???
			REL::RelocationID getDamage{ 25847, 26410 };
			
			//SE: 0x229EF0, AE: 0x23B000, VR:???
			REL::RelocationID TESBook__ReadSkill{ 17439, 17842 };

			//SE: 0x86B980, AE: 0x8ABF40, VR:???
			REL::RelocationID CraftAlch{ 50449, 51354 };

			//SE: 0x739080, AE: 0x776E10, VR:???
			REL::Relocation<std::uintptr_t>Explosion__Damage{ REL::RelocationID{ 42672, 43844 }, 0x4B };

			
			//SE: 0x86C640, AE: 0x8ACCF0, VR:???
			REL::Relocation<std::uintptr_t>CraftEnch__Enchant{ REL::RelocationID{ 50450, 51355 }, se ? 0x256 : 0x254 };
			
			//SE: 0x86D830, AE: 0x8AE270, VR:???
			REL::Relocation<std::uintptr_t>CraftEnch__Disenchant{ REL::RelocationID{ 50459, 51363 }, 0xA4 };

			//SE: 0x9721C0, AE: 9AD6D0, VR: ???
			REL::Relocation<std::uintptr_t>PYRS_AdvanceSkill{ REL::RelocationID{ 54817, 55449 }, 0x28 };

			//SE: 0x979990, AE: 9B46A0, VR: ???
			REL::Relocation<std::uintptr_t>PYRS_IncrementSkill{ REL::RelocationID{ 55002, 55616 }, 0x2E };


			/*
			86E2C0+78 //CraftItem_Create, seems to be create generic, improve weapon, and improve armor all at once.
			86E490+FC//CraftSmith_Create seems to specifically be for smithing. Neat.
			874350+1AF//Needs investigation, seems to be something related to construction though. Shouldn't jump the gun, it's not an advance.
			9721C0+28//PYRS_AdvanceSkill. Needs to use r8d, which is slightly bigger, so please make adjustments. 41 83 F8  11 seems to be the arrangement.
			979990+2E//PYRS_IncrementSkill   cmp     r10d, 11h
			
			
			//Check other objects pls

			14037FAD0+125//AE, this is the check for NPCS, it spans 6 values long.
			1403667A0+CE//SE This is the check for NPCS also, it's a loop.
			367140+2C//GetActorValue for npcs
			*Look, in general I'm just going to avoid NPC shit for now.

			//*/

			REL::Relocation<std::uintptr_t>CraftAlch__Success{ CraftAlch, se ? 0x1EE : 0x1E9 };
			REL::Relocation<std::uintptr_t>CraftAlch__Failure{ CraftAlch, se ? 0x355 : 0x356 };

			
			REL::Relocation<std::uintptr_t>getDamage_1{ getDamage, se ? 0x147 : 0x149 };
			REL::Relocation<std::uintptr_t>getDamage_2{ getDamage,se ? 0x179 : 0x180 };

			REL::Relocation<std::uintptr_t>TESBook__ReadSkill_1{ TESBook__ReadSkill, 0x59 };
			REL::Relocation<std::uintptr_t>TESBook__ReadSkill_2{ TESBook__ReadSkill, REL::VariantOffset{0x14F, 0x186, 0x14F } };


			HandlePatch(PYRS_AdvanceSkill.address(), r8d);
			HandlePatch(PYRS_IncrementSkill.address(), r10d);
			HandlePatch(ActiveEffect__GetCost.address());
			HandlePatch(SpellItem__AdjustCost.address());
			HandlePatch(Explosion__Damage.address());
			//There are more of these, but I believe they ar ill fitted to handle this proper.
			//HandlePatch(CraftEnch__Enchant.address());
			//HandlePatch(CraftEnch__Disenchant.address());
			//HandlePatch(CraftAlch__Success.address());
			//HandlePatch(CraftAlch__Failure.address());
			HandlePatch(TESBook__ReadSkill_1.address());
			HandlePatch(TESBook__ReadSkill_2.address());
			HandlePatch(getDamage_1.address());
			HandlePatch(getDamage_2.address(), se ? eax : esi);

		}
	};




	//Write_call
	struct StatsMenu_InitSkillsHook
	{
		static void Install()
		{
			//36 71 111 108 100
			//SE: 0x8BE990, AE: 0x0000, VR: ???
			REL::Relocation<uintptr_t> hook{ REL::RelocationID{51636, 000}, 0x2F8 };

			auto& tramp = SKSE::GetTrampoline();



			//TODO: Pattern make and if so, call func instead of oroginal.
			//FF  90 30 03 00 00

			//OR

			//TODO: Fish out all the associated skill hook locations, seems it's used in a few places. Maybe make a way to expand the possible entries it's
			//allowed to have.

			func = tramp.write_call<5>(hook.address(), thunk);
		}

		static void thunk(RE::StatsMenu* a_this)
		{
			func(a_this);
			
			//This is what's causing things to double up
			a_this->GetRuntimeData().skillTrees.push_back(RE::ActorValue::kVariable01);
			
			a_this->GetRuntimeData().numSelectableTrees = 19;
		}

		inline static REL::Relocation<decltype(thunk)> func;

	};


	


	//ASM
	struct StatsMenu_UpdateTreeVisualHook
	{
		static void Patch()
		{
			//SE: 0x8BF360 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto address = REL::RelocationID(51638, 000000).address();
			
			auto hook_addr = address + 0x1027;
			
			auto return_addr = address + 0x10CF;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(ecx, edi);

					mov(rax, exec_addr);
					call(rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());

			logger::info("StatsMenu_UpdateTreeVisualHook complete...");
			//*/
		}




		static void thunk(uint32_t entry)
		{
			auto& tree = fakeTreeList[entry];

			if (tree.shader)
			{
				if (tree.state)
				{
					if (auto ms_time = Utility::GetRunTime(); ms_time - tree.timestampMS <= 500)
					{
						float alpha = (ms_time - tree.timestampMS) / 500.f;
						if (tree.state != RE::TreeStates::kEntering)
						{
							alpha = 1.0f - alpha;
						}

						tree.shader->SetMaterialAlpha(alpha);
					}
					else
					{
						float alpha = tree.state == RE::TreeStates::kEntering ? 1.0f : 0.0f;
						tree.shader->SetMaterialAlpha(alpha);
						tree.state = RE::TreeStates::kResting;
					}
				}
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	//ASM
	struct StatsMenu_EnteringHook
	{
		static void Patch()
		{
			//8C09E0+149
			//SE: 8C09E0 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto address = REL::RelocationID(51644, 000000).address();

			auto hook_addr = address + 0x149;

			auto return_addr = address + 0x16B;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(rcx, rbx);

					mov(rax, exec_addr);
					call(rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());

			logger::info("StatsMenu_EnteringHook complete...");
			//*/
		}




		static void thunk(RE::StatsMenu* a_this)
		{
			auto& tree = fakeTreeList[a_this->GetRuntimeData().selectedTree];

			tree.timestampMS = Utility::GetRunTime();
			tree.state = RE::TreeStates::kEntering;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct StatsMenu_EnteringMaybeHook
	{
		static void Patch()
		{
			
			//SE: 8C5B60, AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto hook_addr = REL::RelocationID(51661, 000000).address() + 0x3AA;


			auto return_addr = hook_addr + 0xE;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(rcx, rdi);

					mov(rax, exec_addr);
					call(rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());
			REL::safe_fill(hook_addr + 0x5, 0x90, return_addr - hook_addr - 5);


			logger::info("StatsMenu_EnteringMaybeHook complete...");
			//*/
		}




		static void thunk(RE::StatsMenu* a_this)
		{
			auto& tree = fakeTreeList[a_this->GetRuntimeData().selectedTree];

			tree.timestampMS = Utility::GetRunTime();
			tree.state = RE::TreeStates::kEntering;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	

	//ASM
	struct TreeData_Exiting1Hook
	{
		static void Patch()
		{
			//SE: 0x8C51B0 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto hook_addr = REL::RelocationID(51659, 000000).address() + 0x141;
			
			auto return_addr = hook_addr + 0x62;//to +0x1A3

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(rcx, rdi);

					mov(rax, exec_addr);
					call(rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());
			REL::safe_fill(hook_addr + 0x5, 0x90, return_addr - hook_addr - 5);

			logger::info("TreeData_Exiting1Hook complete...");
			//*/
		}




		static void thunk(RE::StatsMenu* a_this)
		{
			auto& tree = fakeTreeList[a_this->GetRuntimeData().selectedTree];


			if (tree.shader)
			{
				if (Utility::IsInBeastMode() == false)
				{
					if (tree.state || tree.shader->QMaterialAlpha() > 0.f)
					{
						tree.timestampMS = Utility::GetRunTime();
						tree.state = RE::TreeStates::kExiting;
					}
				}
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	//ASM
	struct TreeData_Exiting2Hook
	{
		static void Patch()
		{
			//SE: 0x8C51B0 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto hook_addr = REL::RelocationID(51659, 000000).address() + 0x2A7;

			auto return_addr = hook_addr + 0x4D;//to +0x2F4

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(rcx, rdi);

					mov(rax, exec_addr);
					call(rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());
			REL::safe_fill(hook_addr + 0x5, 0x90, return_addr - hook_addr - 5);

			logger::info("TreeData_Exiting2Hook complete...");
			//*/
		}




		static void thunk(RE::StatsMenu* a_this)
		{
			auto& tree = fakeTreeList[a_this->GetRuntimeData().selectedTree];


			
			if (tree.shader)
			{
				if (tree.state || tree.shader->QMaterialAlpha() > 0.f)
				{
					tree.timestampMS = Utility::GetRunTime();
					tree.state = RE::TreeStates::kExiting;
				}
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};




	//ASM
	struct StatsMenu_ChangeSelectedTreeHook
	{
		static void Patch()
		{
			//8C7110
			//SE: 8C7110 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto hook_addr = REL::RelocationID(51666, 000000).address() + 0xAB;

			auto return_addr = hook_addr + 0x5E;//to +0x109

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(rcx, rdi);
					mov(edx, esi);

					mov(rax, exec_addr);
					call(rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());
			REL::safe_fill(hook_addr + 0x5, 0x90, return_addr - hook_addr - 5);

			logger::info("StatsMenu_ChangeSelectedTreeHook complete...");
			//*/
		}




		static void thunk(RE::StatsMenu* a_this, uint32_t a2)
		{
			auto& tree = fakeTreeList[a_this->GetRuntimeData().selectedTree];
			
			if (tree.shader)//I don't know if this goes here.
			{
				if (tree.state || tree.shader->QMaterialAlpha() > 0.f)
				{
					tree.timestampMS = Utility::GetRunTime();
					tree.state = RE::TreeStates::kExiting;
				}
			}
			a_this->GetRuntimeData().selectedTree = a2;

			tree.timestampMS = Utility::GetRunTime();
			tree.state = RE::TreeStates::kEntering;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};






	//ASM
	struct StatsMenu_SetShader1Hook
	{
		static void Patch()
		{
			//This hook too, can just be inlined into the code.

			//SE: 8C4CA0 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto address = REL::Relocation<uintptr_t>{ REL::RelocationID{51658, 000000}, 0x159 }.address();

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					//push(rcx);
					//mov(ecx, esi);

					mov(edx, esi);

					mov(rax, exec_addr);
					
					//sub(rsp, 0x28);
					call(rax);
					//add(rsp, 0x28);

					//pop(rcx);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, address + 0x8 };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(address, (uintptr_t)code.getCode());
			REL::safe_fill(address + 0x5, 0x90, 0x3);

			logger::info("StatsMenu_SetShader1Hook complete...");
			//*/
		}




		static void thunk(RE::BSEffectShaderProperty* shader, uint32_t index)
		{
			auto& tree = fakeTreeList[index];

			tree.shader = shader;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};

	//ASM
	struct StatsMenu_SetShader2Hook
	{
		static void Patch()
		{
			//This hook too, can just be inlined into the code.

			//SE: 8C4CA0 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto address = REL::Relocation<uintptr_t>{ REL::RelocationID{51658, 000000}, 0x16D }.address();

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(ecx, ebx);

					mov(rax, exec_addr);
					call(rax);
					mov(rcx, rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, address + 0x8 };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(address, (uintptr_t)code.getCode());
			REL::safe_fill(address + 0x5, 0x90, 0x3);

			logger::info("StatsMenu_SetShader2Hook complete...");
			//*/
		}




		static RE::BSEffectShaderProperty* thunk(uint32_t index)
		{
			index /= 2;

			auto& tree = fakeTreeList[index];

			return tree.shader;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};




	//write_call
	struct StatsMenu_CtorHook
	{
		static void Patch()
		{
			//8CC7B0+5D
			//SE: 8CC7B0, AE: 0x000000, VR: ???
			auto address = REL::Relocation<uintptr_t>{ REL::RelocationID{51738, 000000}, 0x5D }.address();


			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(address, thunk);

			logger::info("StatsMenu_CtorHook complete...");
			//*/
		}




		static RE::StatsMenu* thunk(RE::StatsMenu* a_this)
		{
			auto result = func(a_this);

			//For now
			result->GetRuntimeData().numSelectableTrees = 19;

			return result;
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	//ASM
	struct StatsMenu_LoadSkillInfoHook
	{
		static void Patch()
		{
			//SE: 8C20C0 , AE: 0x000000, VR: ???
			//You can try not to use the trampoline here.
			auto hook_addr = REL::Relocation<uintptr_t>{ REL::RelocationID{51652, 000000}, 0x9D }.address();
			
			//SE: 0x6E3
			auto return_addr = hook_addr + 0x646;

			struct Code : Xbyak::CodeGenerator
			{
				Code(uintptr_t exec_addr, uintptr_t ret_addr)
				{
					mov(rcx, r13);

					mov(rax, exec_addr);
					call(rax);
					mov(rcx, rax);

					mov(rax, ret_addr);
					jmp(rax);
				}
			} static code{ (uintptr_t)thunk, return_addr };

			auto& trampoline = SKSE::GetTrampoline();

			trampoline.write_branch<5>(hook_addr, (uintptr_t)code.getCode());
			REL::safe_fill(hook_addr + 0x5, 0x90, return_addr - hook_addr - 5);

			logger::info("StatsMenu_LoadSkillInfoHook complete...");
		}



		static void thunk(RE::StatsMenu& a_this)
		{
			const auto numSkills = 19;
			a_this.uiMovie->SetVariable("_global.AnimatedSkillText.prototype.SKILLS",19);
			auto skillStats = std::vector<RE::GFxValue>(5 * numSkills);

			static auto iDifficultyLevelMax = 1;//"iDifficultyLevelMax"_gs;
			const bool legendaryAvailable = false;//iDifficultyLevelMax && *iDifficultyLevelMax >= 5;

			for (std::uint32_t i = 0; i < skillStats.size(); i += 5) {
				RE::GFxValue& level = skillStats[i + 0];
				RE::GFxValue& name = skillStats[i + 1];
				RE::GFxValue& percent = skillStats[i + 2];
				RE::GFxValue& color = skillStats[i + 3];
				RE::GFxValue& legendary = skillStats[i + 4];

				const RE::ActorValue actorValue = a_this.GetRuntimeData().skillTrees[i / 5];
				if ((int)actorValue - 6 > 0x11) {
					auto info = RE::ActorValueList::GetSingleton()->GetActorValue(actorValue);

					level.SetNumber(static_cast<std::uint32_t>(69));
					
					
					name.SetString(info->GetFullName());
					
					percent.SetNumber(0);

					color.SetString("#FFFFFF");


					//legendary.SetNumber(
					//	legendaryAvailable && skill->Legendary
					//	? static_cast<std::uint32_t>(skill->Legendary->value)
					//	: 0.0);

					legendary.SetNumber(0.0);
				}
				else {
					//name.SetString(Game::GetActorValueName(actorValue));
					//color.SetString(Game::GetActorValueColor(actorValue));

					auto info = RE::ActorValueList::GetSingleton()->GetActorValue(actorValue);

					name.SetString(info->GetFullName());
					color.SetString("#FFFFFF");

					const auto player = RE::PlayerCharacter::GetSingleton();
					const auto playerSkills = player ? player->GetInfoRuntimeData().skills : nullptr;
					const std::size_t idx = std::to_underlying(actorValue) - 6;
					if (playerSkills && idx < 18) {
						const auto& data = playerSkills->data->skills[idx];
						level.SetNumber(static_cast<std::int32_t>(player->AsActorValueOwner()->GetActorValue(actorValue)));
						percent.SetNumber((data.xp / data.levelThreshold) * 100);

						legendary.SetNumber(
							legendaryAvailable ? playerSkills->data->legendaryLevels[idx] : 0);
					}
					else if (
						actorValue == RE::ActorValue::kWerewolfPerks ||
						actorValue == RE::ActorValue::kVampirePerks) {
						level.SetString(""sv);
						percent.SetNumber(player->AsActorValueOwner()->GetActorValue(actorValue));
					}
				}
			}

			a_this.uiMovie->SetVariableArray(
				RE::GFxMovie::SetArrayType::kValue,
				"StatsMenu.SkillStatsA",
				0,
				skillStats.data(),
				static_cast<std::uint32_t>(skillStats.size()),
				RE::GFxMovie::SetVarType::kNormal);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};



	//write_call
	struct StatsMenu_SetDescriptionHook
	{
		static void Patch()
		{
			///8C2BA0+148F
			//SE: 8C2BA0, AE: 0x000000, VR: ???
			auto address = REL::Relocation<uintptr_t>{ REL::RelocationID{51654, 000000}, 0x148F }.address();


			auto& trampoline = SKSE::GetTrampoline();

			func = trampoline.write_call<5>(address, thunk);

			logger::info("StatsMenu_SetDescriptionHook complete...");
			//*/
		}




		static void thunk(RE::BSString& desc, RE::ActorValue skill)
		{
			if (skill == RE::ActorValue::kVariable01) {
				desc = "This is a test.";
			}
			else {
				func(desc, skill);
			}
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};


	//Another write_call needed to happen here, but I forgor what it was.




	struct Hooks
	{
		static void Install()
		{
#ifdef _DEBUG
			constexpr size_t reserve = 512;
#else
			constexpr size_t reserve = 360;//I forgor how much is being allocated.
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

			//V2
			MagicItemCtorHook::Patch();
			GetActorValueForCostHook::Patch();
			
			
			
			AdvanceSkillHook::Patch();
			IncrementSkillHook::Patch();
			SpellItem_GetSkillUsageDataHook::Patch();
			
			MagicMenu_AssociatedSkillHook::Install();

			SkillCheckPatch::Patch();

			InitValuesHook::Patch();
			Changable_InitAfterAllFileHook::Install();
			ReadFromFileStreamHook<RE::TESIdleForm, RE::BGSCameraPath, RE::TESTopicInfo>::Patch();


			//This should have less repetition and no need for a set
			// didn't work.
			//InitializeAfterAllFormsReadHook<RE::TESIdleForm, RE::BGSCameraPath, RE::TESTopicInfo>::Patch();
			
			//These are no longer included.
			//EffectSetting_InitializeAfterAllFormsReadHook::Patch();
#ifdef _DEBUG
//#define EXPERIMENTAL
#endif
		
#ifdef EXPERIMENTAL
			GetActorValueInfoHook::Patch();

			StatsMenu_InitSkillsHook::Install();

			StatsMenu_UpdateTreeVisualHook::Patch();
			//StatsMenu_CtorHook::Patch();
			StatsMenu_EnteringHook::Patch();
			StatsMenu_EnteringMaybeHook::Patch();
			StatsMenu_SetShader1Hook::Patch();
			StatsMenu_SetShader2Hook::Patch();
			

			TreeData_Exiting1Hook::Patch();
			TreeData_Exiting2Hook::Patch();

			StatsMenu_ChangeSelectedTreeHook::Patch();

			StatsMenu_LoadSkillInfoHook::Patch();
			StatsMenu_SetDescriptionHook::Patch();
#endif
			

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