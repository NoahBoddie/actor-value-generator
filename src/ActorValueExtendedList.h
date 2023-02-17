#pragma once

#include "xbyak/xbyak.h"

namespace AVG
{
	//*
	//please move me
	enum class RegisterEnum
	{
		rax,
		rcx,
		rdx,
		r8,
		r9,
	};


	template <RegisterEnum Register>
	struct AveListCode : Xbyak::CodeGenerator
	{
		//uintptr_t list_addr = nullptr;

		//uint8_t* GetCode(uintptr_t list_addr)


		constexpr const Xbyak::Reg64& reg()
		{
			if constexpr (Register == RegisterEnum::rax)
				return rax;
			else if constexpr (Register == RegisterEnum::rcx)
				return rcx;
			else if constexpr (Register == RegisterEnum::rdx)
				return rdx;
			else if constexpr (Register == RegisterEnum::r8)
				return r8;
			else if constexpr (Register == RegisterEnum::r9)
				return r9;
			else
				return rax;//Too lazy to make an error.
		}
		
		AveListCode(uintptr_t list_addr)// requires(Register == RegisterEnum::rax)
		{
			//uintptr_t arg_8 = qword ptr 10h
			mov(reg(), list_addr);
			ret();
		}


	};


	struct ActorValueExtendedList
	{
		//I don't want to have to keep the vector, use this to make the list then delete it.
		inline static std::vector<uintptr_t> avi_list{};

		//Give these fake av's names.
		inline static RE::ActorValueInfo* false_avi = nullptr;
		
		inline static uintptr_t* begin = nullptr;


		static RE::ActorValueInfo* GetOrCreateStandIn()
		{
			if (!false_avi) {
				//This shit actually works believe it or not.
				auto* avi_factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::ActorValueInfo>();

				RE::ActorValueInfo* info_query = avi_factory->Create();

				if (!info_query)
					logger::error("No info created, sadge.");
				else
					logger::info("success, {:08X} formID, addr {:X}", info_query->formID, (uintptr_t)info_query);

				info_query->enumName = "GeneratedExtraValue";
				info_query->abbreviation = "GEV";


				false_avi = info_query;
			}

			return false_avi;
		}

		//Should be a friend function, additionally, this must be sent ONLY, when data is loaded.
		// I would like this to have methods to keep track of its processes.
		//This shit should pull the value itself, no real reason not to.
		static bool Create(uint32_t extended_size)
		{

			//This is some doo doo ass code, but I want this done.
			
			//Core issue to be is the fact that I was using the pointer directly, there's like 8 bytes
			//Why did the normal ones work then....?
			
			if (extended_size == 0) {
				logger::warn("Size is 0, no new list needs to be created.");
				return true;
			}
			
			uint32_t base_size = static_cast<uint32_t>(RE::ActorValue::kTotal);
			//Total size is too large???
			logger::info("total size {}", extended_size + base_size);
			auto* avi = GetOrCreateStandIn();

			RE::ActorValueList* copy_list = RE::ActorValueList::GetSingleton();

			uintptr_t rc_avi = reinterpret_cast<uintptr_t>(avi);

			avi_list = std::vector(base_size + extended_size + 1, rc_avi);

			logger::info("Regarded entry {:X} v {:X}", avi_list.back(), rc_avi);

			size_t copy_size = sizeof(RE::ActorValueInfo*) * base_size;

			std::memcpy(&avi_list[1], &copy_list->actorValues, copy_size);

			uintptr_t first_entry = (copy_list->unk00 << 4) | (copy_list->pad04 << 0);
			
			avi_list[0] = first_entry;

			begin = &avi_list[0];
			
			logger::info("{:X} location", (uintptr_t)begin);
			
			Hook();

			return true;
		}

		//This patches all uses of the other actor value info's into ours, then placing.
		static void Hook()
		{
			//Experimental pointer patch, this seems to be preferable however.
			// 
			//This is a test to see if I can just replace the other version.
			
			//REL::ID old_list{ 514139 };
			REL::RelocationID avi_list_ptr{ 514139, 400267 };//SE: 0x1EBE418, AE: 0x1F58128, VR: ???
			REL::safe_write(avi_list_ptr.address(), &begin, 0x8);

			return;

			auto& trampoline = SKSE::GetTrampoline();
			//140567495 is the location I'm hooking into, remember.
			//567470
			REL::ID call_1{ 34274 };  //0x567470



			//94a4c0 + 0x4F = DamageActorValue_Papyrus
			//0x94a940+ 0x41 = ForceActorValue_Papyrus

			//Turn this into a clean ol bit of function please.
			static AveListCode<RegisterEnum::rax> code_rax{ reinterpret_cast<uintptr_t>(begin) };
			static AveListCode<RegisterEnum::rcx> code_rcx{ reinterpret_cast<uintptr_t>(begin) };
			static AveListCode<RegisterEnum::rdx> code_rdx{ reinterpret_cast<uintptr_t>(begin) };
			static AveListCode<RegisterEnum::r8> code_r8{ reinterpret_cast<uintptr_t>(begin) };
			static AveListCode<RegisterEnum::r9> code_r9{ reinterpret_cast<uintptr_t>(begin) };

			//auto size = code_rax.getSize();
			//logger::info("{} loc", reinterpret_cast<uintptr_t>(avi_list.data()));
			//auto func_rax = trampoline.allocate(size);
			//std::memcpy(func_rax, code_rax.getCode(), size);

			//Not really sure why I don't just store the code the write it. But hey. Test later.

			trampoline.write_call<5>(call_1.address() + 0x25, code_rax.getCode());

			constexpr std::uint8_t NoOperation2[0x2]{ 0x66, 0x90 };
			static_assert(sizeof(NoOperation2) == 0x2);

			//constexpr std::uint8_t NoOperation3[0x3]{ 0x0F, 0x1F, 0x00 };
			//static_assert(sizeof(NoOperation3) == 0x3);

			//Should be the size of a 5 byte call, we're trying to clear possibly interpreted instructions after
			// Should write a function to handle this a bit better too.
			REL::safe_write(call_1.address() + 0x25 + 0x5, &NoOperation2, 0x2);
		}
	};

	//*/
}