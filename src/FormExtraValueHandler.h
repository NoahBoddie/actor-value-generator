#pragma once

#include "ValueAliasHandler.h"


#define IF_FORM(mc_form_type) if constexpr (std::is_same_v<FormType, mc_form_type>)
#define IF_LIKE_FORM(mc_form_type) if constexpr (std::derived_from<FormType, mc_form_type>)

#ifdef USE_UNSTABLE_TYPES
#define UNSTABLE_FORM_TYPES, RE::BGSMusicTrackFormWrapper
#else
#define UNSTABLE_FORM_TYPES
#endif

#define FORM_TYPES \
	RE::BGSPerk, \
	RE::EffectSetting, RE::SpellItem, RE::AlchemyItem, RE::EnchantmentItem, RE::IngredientItem, \
	RE::TESFaction, RE::TESTopicInfo, RE::TESIdleForm, RE::BGSCameraPath, RE::BGSMessage, RE::TESLoadScreen, RE::BGSConstructibleObject, \
	RE::TESPackage, RE::BGSStoryManagerNodeBase, RE::BGSScene, RE::TESObjectWEAP UNSTABLE_FORM_TYPES

namespace AVG
{
	//Move me
	template<class A, class B>
	union ConversionCaster
	{
		A _a;
		B _b;

		ConversionCaster() = default;
		ConversionCaster(A a) : _a(a) {}
		ConversionCaster(B b) : _b(b) {}

		ConversionCaster& operator=(A a) { _a = a; return *this; }
		ConversionCaster& operator=(B b) { _b = b; return *this; }

		operator A() { return _a; }
		operator B() { return _b; }
	};
	template <class B>
	using VoidCaster = ConversionCaster<void*, B>;

	class FormExtraValueHandler : public RE::BSTEventSink<SKSE::ModCallbackEvent>
	{
		using EventResult = RE::BSEventNotifyControl;


		inline static HMODULE KID{ nullptr };


		static auto* GetSingleton()
		{
			static FormExtraValueHandler singleton;
			return &singleton;
		}

		EventResult ProcessEvent(const SKSE::ModCallbackEvent* a_event, RE::BSTEventSource<SKSE::ModCallbackEvent>*) override
		{
			if (a_event && a_event->eventName == "KID_KeywordDistributionDone") {
				//logger::info("{:*^30}", "LOOKUP");
				logger::info("KID Distribution complete, processing forms...");

				Init<FORM_TYPES>();

				SKSE::GetModCallbackEventSource()->RemoveEventSink(GetSingleton());
			}

			return EventResult::kContinue;
		}

		//move me plz thx
		static void HandleCondition(RE::TESCondition& condition, RE::TESForm* form, bool& change)
		{
			if (!form) {
				return;
			}

			RE::TESConditionItem* item = condition.head;

			while (item != nullptr) {
				RE::FUNCTION_DATA& function_data = item->data.functionData;

				switch (*function_data.function) {
				default:
					break;

				case RE::FUNCTION_DATA::FunctionID::kGetActorValue:
				case RE::FUNCTION_DATA::FunctionID::kGetPermanentActorValue:
				case RE::FUNCTION_DATA::FunctionID::kGetActorValuePercent:
				case RE::FUNCTION_DATA::FunctionID::kGetBaseActorValue:
					//logger::info("THRILL, {}", function_data.params[0]);
				{
					VoidCaster<RE::ActorValue> stored_av = function_data.params[0];
					//if (stored_av._b > RE::ActorValue::kTotal || (int)stored_av._b < 0)
					//	logger::debug("processing {} form type {}, with a value of {}", form->GetName(), (int)form->GetFormType(), (int)stored_av._b);


					stored_av = ValueAliasHandler::AliasToValue(stored_av, form, change);
					function_data.params[0] = stored_av;
					
					//if (reinterpret_cast<uintptr_t>(function_data.params[0]) == static_cast<uintptr_t>(RE::ActorValue::kVariable01)) {
					//	function_data.params[0] = reinterpret_cast<void*>(new_av);
					//}
					break;
				}
				}

				item = item->next;
			}
		}
		
		template<std::derived_from<RE::TESForm> FormType>
		static bool ProcessForm(FormType* form)
		{
			//I'd like to use this as a helper to see how many forms are processed.
			bool success = false;


			IF_FORM(RE::EffectSetting)
			{
				//Gonna need an alias on this type.
				RE::EffectSetting::EffectSettingData& effect_data = form->data;

				using Archetype = RE::EffectArchetypes::ArchetypeID;

				switch (effect_data.archetype)
				{
				case Archetype::kDualValueModifier:
					effect_data.secondaryAV = ValueAliasHandler::AliasToValue(effect_data.secondaryAV, form, AliasQuerySettings::RequireSetting, success);
					[[fallthrough]];

				case Archetype::kValueModifier:
				case Archetype::kPeakValueModifier:
					effect_data.primaryAV = ValueAliasHandler::AliasToValue(effect_data.primaryAV, form, AliasQuerySettings::RequireSetting, success);
					[[fallthrough]];
				default:
					effect_data.resistVariable = ValueAliasHandler::AliasToValue(effect_data.resistVariable, form, AliasQuerySettings::AllowNone, success);
					break;
				}

				HandleCondition(form->conditions, form, success);
			} 
			else IF_FORM(RE::BGSPerk)
			{
				using PerkEntryFunction = RE::BGSEntryPointPerkEntry::EntryData::Function;
					
				HandleCondition(form->perkConditions, form, success);
				
				for (auto& entry : form->perkEntries)
				{
					if (entry->GetType() != RE::PERK_ENTRY_TYPE::kEntryPoint) {
						continue;
					}

					RE::BGSEntryPointPerkEntry* entry_point = skyrim_cast<RE::BGSEntryPointPerkEntry*>(entry);

					for (auto& condition : entry_point->conditions) {
						HandleCondition(condition, form, success);
					}


					switch (*entry_point->entryData.function) 
					{
					default:
						continue;

					case PerkEntryFunction::kAddActorValueMult:
					case PerkEntryFunction::kSetToActorValueMult:
					case PerkEntryFunction::kMultiplyActorValueMult:
					case PerkEntryFunction::kMultiply1PlusActorValueMult:
						break;
					}
					
					RE::BGSEntryPointFunctionDataTwoValue* function_data = skyrim_cast<RE::BGSEntryPointFunctionDataTwoValue*>(entry_point->functionData);
					
					if (!function_data)
						continue;

					RE::ActorValue new_av = ValueAliasHandler::AliasToValue(static_cast<RE::ActorValue>(function_data->data1), form, success);
					
					function_data->data1 = static_cast<float>(new_av);
					
				}
			}
			else IF_FORM(RE::TESFaction)
			{
				//Simple, but the condition is tucked away
				if (form->vendorData.vendorConditions)
					HandleCondition(*form->vendorData.vendorConditions, form, success);
			}
			else IF_FORM(RE::TESTopicInfo)
			{
				//Simple
				HandleCondition(form->objConditions, form, success);
			}
			else IF_FORM(RE::TESIdleForm)
			{
				//simple
				HandleCondition(form->conditions, form, success);
			}
			else IF_FORM(RE::BGSCameraPath)
			{
				//simple
				HandleCondition(form->conditions, form, success);
			}
			else IF_FORM(RE::BGSMessage)
			{
				//Needs to iterate through the button entries, make sure to null check first, they are optional
				for (auto& button : form->menuButtons) {
					if (button)
						HandleCondition(button->conditions, form, success);
				}
			}
			else IF_FORM(RE::TESObjectWEAP)
			{
				auto& weapon_data = form->weaponData;
				weapon_data.resistance = ValueAliasHandler::AliasToValue(*weapon_data.resistance, form, AliasQuerySettings::AllowNone, success);
				//weapon_data.skill = ValueAliasHandler::AliasToValue(*weapon_data.skill, form);//Disabled for now
				
				//Embed isn't used so maybe not needed. But just in case someone does have something of the like.
				//form->embeddedWeaponAV = ValueAliasHandler::AliasToValue(form->embeddedWeaponAV, form, false, true);
			}
			else IF_FORM(RE::TESLoadScreen)
			{
				//standard
				HandleCondition(form->conditions, form, success);
			}
			else IF_FORM(RE::BGSConstructibleObject)
			{
				//pretty standard
				HandleCondition(form->conditions, form, success);
			}
			else IF_FORM(RE::TESQuest)
			{
				//Packed with about 4 different data types we gotta deal with. So you know.
			}
			else IF_FORM(RE::TESPackage)
			{
				//This takes a fuck ton of conditions, templating might negate some of that so beware?
				HandleCondition(form->packConditions, form, success);
				//There's more, but I've yet to have the ability to handle that.
			}
			else IF_FORM(RE::BGSStoryManagerNodeBase)
			{
				//Seems to be the core
				HandleCondition(form->conditions, form, success);
			}
			else IF_FORM(RE::BGSScene)
			{
				//Has more than one, and may need to iterate.
				HandleCondition(form->conditions, form, success);
			}
			else IF_FORM(RE::BGSSoundDescriptorForm)
			{
				//Still not sure how to make this.
			}
			else IF_FORM(RE::BGSMusicTrackFormWrapper)
			{
				//Check all
				HandleCondition(form->track->conditions, form, success);
			}
			else IF_LIKE_FORM(RE::MagicItem)
			{
				//Iterate through and handle conditions.
				//At a time later it will also need patching with skill use data.
				for (auto& effect : form->effects) {
					HandleCondition(effect->conditions, form, success);
				}
			}
			else {
				static bool shown = false;

				if (!shown)
					logger::info("Processing for form type {} not found.", typeid(FormType).name());

				shown = true;
			}

			//BGSStandardSoundDef This is conditioned, but not seemingly used.

			return success;
		}

	

		template <size_t I, class... PatchTypes>
		static void NthProcess(RE::TESDataHandler* handler)
		{
			using Args = std::tuple<PatchTypes...>;

			using FormType = std::tuple_element<I, Args>::type;

			RE::BSTArray<RE::TESForm*>& form_array = handler->GetFormArray(FormType::FORMTYPE);

			//Move this as a const expression elsewhere
			constexpr bool I_teenth = (I >= 11 && I <= 13);
			constexpr auto I_remain = I_teenth ? 9 : (I % 10);

			constexpr std::string_view _th = I_remain == 1 ?
				"st" : I_remain == 2 ?
				"nd" : I_remain == 3 ?
				"rd" : "th";

			logger::info("Starting {}{} process '{}'...", I, _th, typeid(FormType).name());

			size_t process_count = 0;

			for (RE::TESForm* form : form_array) {
				FormType* true_form = form->As<FormType>();
				
				if (true_form)
					process_count += ProcessForm(true_form);
			}
			
			logger::info("'{}' complete, {} entr{} processed.", typeid(FormType).name(), process_count, process_count == 1 ? "y": "ies");

			if constexpr (I > 0)
				NthProcess<I - 1, PatchTypes...>(handler);
		}

		template <class... PatchTypes> requires(sizeof...(PatchTypes) > 0)
		static bool Init()
		{
			RE::TESDataHandler* data_handler = RE::TESDataHandler::GetSingleton();

			if (data_handler) {
				NthProcess<sizeof...(PatchTypes) - 1, PatchTypes...>(data_handler);
				logger::info("All forms processed.");
			}
			else {
				logger::error("Error occured, FormExtraValueHandler::Initialize failed to get RE::TEDataHandler.");
			}
			return data_handler;
		}

	public:
			
		static void Initialize()
		{
			if (KID == nullptr)
				Init<FORM_TYPES>();
		}



		static void Register()
		{
			//typedef int(__stdcall* f_funci)();
			//HINSTANCE hGetProcIDDLL = LoadLibrary(L"C:\\Documents and Settings\\User\\Desktop\\test.dll");
			///f_funci funci = (f_funci)GetProcAddress(hGetProcIDDLL, "funci");
			//return;
			KID = GetModuleHandle(L"po3_KeywordItemDistributor.dll");
			//if (!a_load || a_load->GetPluginInfo("po3_KeywordItemDistributor.dll") == nullptr) {
			if (KID == nullptr) {
				logger::info("KID not found, will initialize on DataLoaded.");
				return;
			}

			SKSE::GetModCallbackEventSource()->AddEventSink(GetSingleton());
		

			logger::info("KID found, will initialize on after ModEvent.");
		}


		FormExtraValueHandler() = default;
		FormExtraValueHandler(const FormExtraValueHandler&) = delete;
		FormExtraValueHandler(FormExtraValueHandler&&) = delete;

		~FormExtraValueHandler() override = default;

		FormExtraValueHandler& operator=(const FormExtraValueHandler&) = delete;
		FormExtraValueHandler& operator=(FormExtraValueHandler&&) = delete;
	};



}

#undef IF_FORM
#undef IF_LIKE_FORM
#undef FILE_STREAM_TYPES