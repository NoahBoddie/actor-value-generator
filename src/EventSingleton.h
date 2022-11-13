#pragma once

#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"

namespace AVG
{
	RE::EffectSetting* the_setting = nullptr;

	using EventResult = RE::BSEventNotifyControl;

	class EventSingleton :
		public RE::BSTEventSink<RE::TESHitEvent>,
		public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>
	{
	public:
		static EventSingleton* GetSingleton()
		{
			static EventSingleton singleton;
			return &singleton;
		}

		static void Install()
		{
			//At a later point I would like to break this thing up into it's own set of respective classes.
			// While still having a patch events here to do the work in this regard.
			//
			auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton();

			if (!sourceHolder) {
				//report no source
				return;
			}

			auto event_s = GetSingleton();
			//auto effectManager = EffectManager::GetSingleton();//should do the events here.
			if (!event_s) {  // || !effectManager) {
				//report this thing no exist?
				return;
			}

			sourceHolder->AddEventSink<RE::TESHitEvent>(event_s);
			sourceHolder->AddEventSink<RE::TESMagicEffectApplyEvent>(event_s);
			//sourceHolder->AddEventSink<RE::TESFormDeleteEvent>(event_s);

			logger::trace("Event Singleton successfully initialized.");
		}

		
		static RE::ActiveEffect* GetEffectFromSetting(RE::MagicTarget* magicTarget, RE::EffectSetting* setting)
		{
			if (!magicTarget)
				return nullptr;

			RE::BSSimpleList<RE::ActiveEffect*>* activeEffects = magicTarget->GetActiveEffectList();

			if (!activeEffects || activeEffects->empty() == true)
				return nullptr;

			for (auto effect : *activeEffects) {
				if (effect->GetBaseObject() == setting)
					return effect;
			}

			return nullptr;
		}

		//*
		//Want to move this to a hook IF I CAN so I can perhaps get and manipulate more information about hit detection.
		EventResult ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override
		{	
			auto target = a_event->target.get();

			RE::Actor* target_actor = target->As<RE::Actor>();
			
			if (!target_actor)
				return EventResult::kContinue;
			
		
			
			RE::ActorValue forced_av = static_cast<RE::ActorValue>(256);
			//This has a threading issue rn, I'll move on, because my main goal is just the hooks, but I'd like to fix this at some point.
			
			//target_actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, forced_av, 69);

			/*
			
					
			if (target_actor->IsPlayerRef() == true)
				if (!the_setting)
					logger::info("<No setting>");
				else if (the_setting->data.primaryAV == static_cast<RE::ActorValue>(256))
					logger::info("Setting intact");
				else
					logger::error("setting is not intact");
		
			float old_value = target_actor->GetActorValue(forced_av);//Psuedo::GetExtraValue(target_actor, "HitsTaken");//

			float new_value = old_value + 1;

			target_actor->SetActorValue(forced_av, new_value);//Psuedo::SetExtraValue(target_actor, "HitsTaken", new_value);//
			//*/
			float new_value = target_actor->GetActorValue(forced_av);//Psuedo::GetExtraValue(target_actor, "HitsTaken");//


			//float should_crash = target_actor->GetActorValue(static_cast<RE::ActorValue>(256));
			
			logger::info("{} has taken {} hits so far.", target_actor->GetName(), new_value);
			
			

			return EventResult::kContinue;
		}
		
		EventResult ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource) override
		{
			return EventResult::kContinue;

			//This shit will not work, I need a hook.
			if (the_setting->formID != a_event->magicEffect)
				return EventResult::kContinue;

			auto target = a_event->target.get();

			RE::Actor* target_actor = target->As<RE::Actor>();

			if (!target_actor)
				return EventResult::kContinue;
			
			
			
			RE::ActiveEffect* ae = GetEffectFromSetting(target_actor, the_setting);

			RE::ValueModifierEffect* vme = skyrim_cast<RE::ValueModifierEffect*>(ae);


			if (!vme) {
				logger::error("ERROR");
				return EventResult::kContinue;
			}

			logger::info("actor value is {}", static_cast<uint32_t>(vme->actorValue));


			return EventResult::kContinue;
		}

		//*/
		//*
	protected:
		EventSingleton() = default;
		EventSingleton(const EventSingleton&) = delete;
		EventSingleton(EventSingleton&&) = delete;
		virtual ~EventSingleton() = default;

		auto operator=(const EventSingleton&) -> EventSingleton& = delete;
		auto operator=(EventSingleton&&) -> EventSingleton& = delete;
		//*/
	};
}