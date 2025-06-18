#pragma once

#include "ValueAliasHandler.h"


#define IF_FORM(mc_form_type) if constexpr (std::is_same_v<FormType, mc_form_type>)
#define IF_LIKE_FORM(mc_form_type) if constexpr (std::derived_from<FormType, mc_form_type>)

//#define RightActorValue() pad74
//#define LeftActorValue() pad84



#ifdef USE_UNSTABLE_TYPES
#define UNSTABLE_FORM_TYPES, RE::BGSMusicTrackFormWrapper, RE::BGSStoryManagerNodeBase
#else
#define UNSTABLE_FORM_TYPES
#endif

#define FORM_TYPES \
	RE::BGSPerk, \
	RE::EffectSetting, RE::SpellItem, RE::AlchemyItem, RE::EnchantmentItem, RE::IngredientItem, \
	RE::TESFaction, RE::TESTopicInfo, RE::TESIdleForm, RE::BGSCameraPath, RE::BGSMessage, RE::TESLoadScreen, RE::BGSConstructibleObject, \
	RE::TESPackage, RE::BGSScene, RE::TESObjectWEAP UNSTABLE_FORM_TYPES

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

		inline static std::map<RE::FormType, std::set<RE::TESForm*>> unrepresentedForms;

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
		static void HandleCondition(RE::TESCondition& condition, RE::TESForm* form, int& change_num)
		{
			if (!form) {
				return;
			}

			RE::TESConditionItem* item = condition.head;
			logger::info("~{:X}", (uintptr_t)item);
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
					

					//if (log)
					//	logger::debug("AV {} in info", stored_av._b);

					
					//if (stored_av._b > RE::ActorValue::kTotal || (int)stored_av._b < 0)
					//	logger::debug("processing {} form type {}, with a value of {}", form->GetName(), (int)form->GetFormType(), (int)stored_av._b);


					stored_av = ValueAliasHandler::AliasToValue(stored_av, form, change_num);
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
		static int ProcessForm(FormType* form)
		{
			//I'd like to use this as a helper to see how many forms are processed.
			int successes = 0;


			IF_FORM(RE::EffectSetting)
			{
				//Gonna need an alias on this type.
				RE::EffectSetting::EffectSettingData& effect_data = form->data;

				using Archetype = RE::EffectArchetypes::ArchetypeID;

				switch (effect_data.archetype)
				{
				case Archetype::kDualValueModifier:
					effect_data.secondaryAV = ValueAliasHandler::AliasToValue(effect_data.secondaryAV, form, successes, AliasQuerySettings::RequireSetting);
					[[fallthrough]];
				//Forgot accumulating.
				case Archetype::kAbsorb:
				case Archetype::kValueAndParts:
				case Archetype::kValueModifier:
				case Archetype::kPeakValueModifier:
				case Archetype::kAccumulateMagnitude:
					effect_data.primaryAV = ValueAliasHandler::AliasToValue(effect_data.primaryAV, form, successes, AliasQuerySettings::RequireSetting);
					[[fallthrough]];
				default:
					effect_data.resistVariable = ValueAliasHandler::AliasToValue(effect_data.resistVariable, form, successes, AliasQuerySettings::AllowNone);
					break;
				}

				HandleCondition(form->conditions, form, successes);
			} 
			else IF_FORM(RE::BGSPerk)
			{
				using PerkEntryFunction = RE::BGSEntryPointPerkEntry::EntryData::Function;
					
				HandleCondition(form->perkConditions, form, successes);
				
				for (auto& entry : form->perkEntries)
				{
					if (entry->GetType() != RE::PERK_ENTRY_TYPE::kEntryPoint) {
						continue;
					}

					RE::BGSEntryPointPerkEntry* entry_point = skyrim_cast<RE::BGSEntryPointPerkEntry*>(entry);
					//*
					if (!entry_point) {
						logger::warn("No entry point in {} at {:08X}", form->GetName(), form->GetFormID());
						continue;
					}
					//*/
					for (auto& condition : entry_point->conditions) {
						HandleCondition(condition, form, successes);
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

					RE::ActorValue new_av = ValueAliasHandler::AliasToValue(static_cast<RE::ActorValue>(function_data->data1), form, successes);
					
					function_data->data1 = static_cast<float>(new_av);
					
				}
			}
			else IF_FORM(RE::TESFaction)
			{
				//Simple, but the condition is tucked away
				if (form->vendorData.vendorConditions)
					HandleCondition(*form->vendorData.vendorConditions, form, successes);
			}
			else IF_FORM(RE::TESTopicInfo)
			{
				//Simple
				HandleCondition(form->objConditions, form, successes);
			}
			else IF_FORM(RE::TESIdleForm)
			{
				//simple
				HandleCondition(form->conditions, form, successes);
			}
			else IF_FORM(RE::BGSCameraPath)
			{
				//simple
				HandleCondition(form->conditions, form, successes);
			}
			else IF_FORM(RE::BGSMessage)
			{
				//Needs to iterate through the button entries, make sure to null check first, they are optional
				for (auto& button : form->menuButtons) {
					if (button)
						HandleCondition(button->conditions, form, successes);
				}
			}
			else IF_FORM(RE::TESObjectWEAP)
			{
				auto& weapon_data = form->weaponData;
				weapon_data.resistance = ValueAliasHandler::AliasToValue(*weapon_data.resistance, form, successes,
					AliasQuerySettings::AllowNone);
				//weapon_data.skill = ValueAliasHandler::AliasToValue(*weapon_data.skill, form);//Disabled for now
				
				//Embed isn't used so maybe not needed. But just in case someone does have something of the like.
				//form->embeddedWeaponAV = ValueAliasHandler::AliasToValue(form->embeddedWeaponAV, form, false, true);
			}
			else IF_FORM(RE::TESLoadScreen)
			{
				//standard
				HandleCondition(form->conditions, form, successes);
			}
			else IF_FORM(RE::BGSConstructibleObject)
			{
				//pretty standard
				HandleCondition(form->conditions, form, successes);
			}
			else IF_FORM(RE::TESQuest)
			{
				//Packed with about 4 different data types we gotta deal with. So you know.
			}
			else IF_FORM(RE::TESPackage)
			{
				//This takes a fuck ton of conditions, templating might negate some of that so beware?
				HandleCondition(form->packConditions, form, successes);
				//There's more, but I've yet to have the ability to handle that.
			}
			else IF_FORM(RE::BGSStoryManagerNodeBase)
			{
				//Seems to be the core
				HandleCondition(form->conditions, form, successes);
			}
			else IF_FORM(RE::BGSScene)
			{
				//Has more than one, and may need to iterate.
				HandleCondition(form->conditions, form, successes);
			}
			else IF_FORM(RE::BGSSoundDescriptorForm)
			{
				//Still not sure how to make this.
			}
			else IF_FORM(RE::BGSMusicTrackFormWrapper)
			{
				//Check all
				HandleCondition(form->track->conditions, form, successes);
			}
			else IF_LIKE_FORM(RE::MagicItem)
			{
				//Iterate through and handle conditions.
				//At a time later it will also need patching with skill use data.
				for (auto& effect : form->effects) {
					HandleCondition(effect->conditions, form, successes);
				}

#ifndef _DEBUG
				return successes;
#endif

				//RE::MagicItem* magic = nullptr;
				//I would like something where you can use a keyword to control the primary alias, then from there it searches the costliest
				//Secondarily, I would like this to be able to do something where it can search the spell itself for keywords.
				RE::Effect* effect_item = form->GetCostliestEffectItem();
				
				if (!effect_item)
					return successes;

				RE::EffectSetting* cost_effect = effect_item->baseEffect;

				constexpr bool is_enchant = std::is_same_v<FormType, RE::EnchantmentItem>;
				constexpr bool is_spell = std::is_same_v<FormType, RE::SpellItem>;
				//GetActorValueForCost

				if constexpr (is_enchant || is_spell)
				{
					constexpr RE::ActorValue right_expect = is_enchant ? RE::ActorValue::kRightItemCharge : RE::ActorValue::kMagicka;
					constexpr RE::ActorValue left_expect = is_enchant ? RE::ActorValue::kLeftItemCharge : RE::ActorValue::kMagicka;

					RE::ActorValue right_hand;
					RE::ActorValue left_hand;

					if constexpr (is_enchant) {
						if (form->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect)
							return successes;
						//RequireSetting
						right_hand = ValueAliasHandler::AliasToValue(RE::ActorValue::kRightItemCharge, cost_effect, successes,
							AliasQuerySettings::IgnorePlugin, AliasQuerySettings::RequireCost);
						left_hand = ValueAliasHandler::AliasToValue(RE::ActorValue::kLeftItemCharge, cost_effect, successes,
							AliasQuerySettings::IgnorePlugin, AliasQuerySettings::RequireCost);
					}
					else {
						right_hand = ValueAliasHandler::AliasToValue(RE::ActorValue::kMagicka, cost_effect, successes,
							AliasQuerySettings::IgnorePlugin, AliasQuerySettings::RequireCost);
						left_hand = right_hand;
					}
					
					if (right_hand != right_expect)
						form->pad74 = static_cast<uint32_t>(right_hand);

					if (left_hand != left_expect)
						form->pad84 = static_cast<uint32_t>(left_hand);
				}
				
				//form->GetSkillUsageData(SkillUsageData & a_data)
			}
			else {
				static bool shown = false;

				if (!shown)
					logger::info("Processing for form type {} not found.", typeid(FormType).name());

				shown = true;
			}

			//BGSStandardSoundDef This is conditioned, but not seemingly used.

			return successes;
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
			constexpr std::array<std::string_view, 2> _y{ "y", "ies" };
			constexpr std::array<std::string_view, 2> _s{ "", "s" };

			//Add process count here
			logger::info("Starting {}{} process '{}'...", I, _th, typeid(FormType).name());

			size_t form_count = 0;
			size_t entry_count = 0;
			
			size_t list_size = form_array.size();


			if (list_size)
			{
				for (RE::TESForm* form : form_array) {
					FormType* true_form = form->As<FormType>();

					if (true_form) {
						size_t process_count = ProcessForm(true_form);

						form_count += process_count != 0;
						entry_count += process_count;
					}
				}
			}
			else
			{
				logger::warn("Native form array for {} is empty. Using custom count.", typeid(FormType).name());

				auto it = unrepresentedForms.find(FormType::FORMTYPE);

				if (unrepresentedForms.end() != it)
				{
					list_size = it->second.size();
					
					for (RE::TESForm* form : it->second) {
						FormType* true_form = form->As<FormType>();

						if (true_form) {
							size_t process_count = ProcessForm(true_form);

							form_count += process_count != 0;
							entry_count += process_count;
						}
					}
				}
				else {
					logger::warn("No custom cache found.");
				}


			}
			//I'd like to change this from just entries to avs changed.
			logger::info("'{}' complete\n Processed {} form{} and {} entr{}, of {} total.", 
				typeid(FormType).name(), 
				form_count, _s[form_count != 1], 
				entry_count, _y[entry_count != 1],
				list_size);

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

			//There's a lot of forms in this, so you may want to clear it.
			unrepresentedForms.clear();

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


		static void AddUnrepresentedForm(RE::TESForm* form)
		{
			if (!form)
				return;

			unrepresentedForms[*form->formType].emplace(form);
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