#pragma once

#include "ExtraValue.h"

namespace AVG::Legacy
{

	/*
	external float GetActorValue(this Actor, string actor_value, float modifier_flags);
	external float GetCrimeGold(this Actor, float crime_type);
	external float GetGameSetting(this Actor, string setting_name);
	external float GetGlobalValue(this Actor);//removing this
	external float GetGlobalValueParam(this Actor, Form global_variable);
	external float GetHasMagicEffect(this Actor, Form magic_effect, float should_count, float regard_ability);
	external float GetItemCount(this Actor, Form item);
	external float GetKeyword(this Actor, Form keyword, float match_all);
	external float GetKeywordString(this Actor, string keyword_name);
	external float GetLevel(this Actor);
	external float GetPlayerLevelDistance(this Actor);
	external float GetRandomRange(this Actor, float min, float max);
	external float HasPerks(this Actor, string av_name, float should_count);
	external float IsInFaction(this Actor, Form faction, float min_rank, float max_rank);
	external float IsRace(this Actor, Form race, float use_original);

	//*/



	float GetRandomRange(RE::Actor* target, float min, float max)
	{
		if (target) {
			//We remove the load order defined part of the seed. This basically allows
			// the specific forms to have different but deterministic results.
			uint32_t seed = target->GetFormID() & 0x00FFFFFF;

			std::srand(seed);
		}

		//auto str = args[2]->GetStringParam();

		//logger::info("SentString is  \"{}\", min:{}, max:{}", str, min, max);

		//Flawed implementation, do not care.
		int range = max - min + 1;

		int num = rand() % range + min;

		//cout << "Special Function: min = " << min << ", max = " << max << ", result is num " << num << ";";
		logger::debug("Special Function: min = {}, max = {}, result is num {};", min, max, num);
		return num;
	};


	float GetActorValue(RE::Actor* target, std::string av_name, float a_input)
	{
		//Now in this sort of situation, I think it's pro
		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor)
			return 0;//Actually throw an exception.

		ExtraValueInput input = static_cast<ExtraValueInput>(a_input);

		float value = Psuedo::GetEitherValue(target_actor, av_name, input);

		logger::trace("{}'s check of AV:'{}'({}) is {}", target_actor->GetName(), av_name, (int)input, value);

		if (isnan(value) == true)
			return 0;//Actually throw an exception.

		return value;
	};


	float GetCrimeGold(RE::Actor* target, float a_flag)
	{
		//Now in this sort of situation, I think it's pro
		RE::Actor* target_actor = target->As<RE::Actor>();
		RE::TESFaction* faction = nullptr;

		float result = 0;

		int flag = (int)a_flag;


		if (!target_actor)
			faction = target->As<RE::TESFaction>();
		else if (target_actor->IsPlayerRef() == true) {
			RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

			for (auto& [faction, crime_struct] : player->GetCrimeValue().crimeGoldMap)
			{
				if (flag != 2)//IE if its ab
					result += crime_struct.nonViolentCur;

				if (flag != 1)//IE if its 2
					result += crime_struct.violentCur;
			}

			return result;
		}
		else
			faction = target_actor->GetCrimeFaction();

		if (!faction)
			return 0;



		//if any other value than 1 or 2, gets both.
		if (flag != 2)//IE if its 1
			result += faction->GetCrimeGoldNonViolent();

		if (flag != 1)//IE if its 2
			result += faction->GetCrimeGoldViolent();

		return result;
	};


	float GetLevel(RE::Actor* target)
	{
		if (!target) {
			//Report
			return 0;
		}

		auto result = target->GetLevel();

		//logger::info("result?? {}", result);

		return (float)result;
	};

	float GetPlayerLevelDistance(RE::Actor* target)
	{
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		if (!player)
			return 0;

		//bool abs_value = args[0]->GetNumberParam();

		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor) {
			//Report
			return 0;//Or should it just return player....
		}


		if (!target_actor->IsPlayerRef()) {
			//Report
			return 0;
		}



		float value = player->GetLevel() - target_actor->GetLevel();

		//return abs_value ? abs(value) : value;
		return value;
	};


	float HasPerks(RE::Actor* target, std::string av_name, float should_count)
	{
		//String, number, maybe number. Name, should count all, maybe skill requirement.
		//could be interesting if I introduce skill requirements
		//skill requirements could work like this. Negative number/0 means anything under or equal the absolute value, while positive number
		// means anything above or equal to that value.


		RE::Actor* target_actor = target->As<RE::Actor>();

		if (!target_actor) {
			//Report
			logger::error("Target is null.");
			return 0;
		}

		//BGSSkillPerkTreeNode* perkTree

		RE::ActorValueList* singleton = RE::ActorValueList::GetSingleton();

		if (!singleton) {
			//report
			logger::error("Actor Value List singleton has not loaded yet.");
			return 0;
		}

		RE::ActorValue av = singleton->LookupActorValueByName(av_name);



		if (av == RE::ActorValue::kNone) {
			//report
			logger::error("Actor Value '{}' couldn't be found", av_name);
			return 0;
		}


		float count = 0;

		if (av < RE::ActorValue::kTotal)
		{
			RE::ActorValueInfo* info = singleton->GetActorValue(av);

			if (!info) {
				logger::error("Actor Value Info '{}' couldn't be found", av_name);
				return 0;
			}

			if (!info->perkTree) {
				logger::error("Actor Value '{}' has no perks.", av_name);
				return 0;
			}

			//No way to get the exact amount of what we are gonna get so I'm just gonna make it twenty.
			std::vector<RE::BGSSkillPerkTreeNode*> visited_nodes{};

			std::function<int(RE::BGSSkillPerkTreeNode*)> node_check = [&](RE::BGSSkillPerkTreeNode* node, int direction = 0) -> int
				{
					int count = 0;
					int add = 0;

					add = target_actor->HasPerk(node->perk);

					count += add;

					if (add && !should_count)
						return count;

					//It would seem the first entry is empty. Makes sense to tell if youve reached the first I guess?
					RE::BGSPerk* next_perk = node->perk ? node->perk->nextPerk : nullptr;

					while (next_perk)
					{
						add = target_actor->HasPerk(next_perk);

						count += add;

						if (add && !should_count)
							return count;


						next_perk = next_perk->nextPerk;
					}

					for (auto& child_node : node->children)
					{
						if (std::find(visited_nodes.begin(), visited_nodes.end(), child_node) != visited_nodes.end())
							continue;

						add = node_check(child_node);

						visited_nodes.push_back(child_node);

						if (add)
						{
							count += add;

							if (!should_count)
								return count;
						}
					}

					return count;
				};

			return node_check(info->perkTree);
			//RE::BGSSkillPerkTreeNode* 
		}
		else
		{
			//Skills implementation, holding off on.
		}

		return 0;
	};

	//This remains unusable at nearly all levels.
	//float GetGlobalValue(RE::Actor* target)
	//{
	//	RE::TESGlobal* global = target->As<RE::TESGlobal>();
	//	//This should be guarded, but it's me being lazy.
	//	return global ? global->value : 0;
	//};

	
	float GetGlobalValueParam(RE::Actor* target, RE::TESForm* param)
	{
		if (param)
		{
			if (RE::TESGlobal* global = param->As<RE::TESGlobal>())
				return global->value;
		}

		//LEX::report::runtime::warn("Null global found.");
		logger::warn("Null global found.");

		return 0;
	};

	
	float GetItemCount(RE::Actor* target, RE::TESForm* item)
	{
		if (!target) {
			//Report
			return 0;
		}

		RE::TESBoundObject* object = item->As<RE::TESBoundObject>();

		if (!object) {
			return 0;
		}
		auto inventory_counts = target->GetInventoryCounts();

		return inventory_counts[object];
	};


	float GetHasMagicEffect(RE::Actor* target, RE::TESForm* base, float should_count, float regard_ability)
	{
		RE::Actor* actor = target->As<RE::Actor>();

		if (!actor) {
			return 0;
		}

		if (!base) {
			logger::error("base object is null.");
			return 0;
		}
		logger::info("{} {} AV", (int)base->GetFormType(), base->GetName());

		RE::EffectSetting* effect = base->As<RE::EffectSetting>();

		if (!effect) {
			logger::error("effect is not an effect.");
			return 0;
		}


		if (!should_count && !regard_ability) {
			logger::info("firing short hand");//make debug when done producing.
			return actor->AsMagicTarget()->HasMagicEffect(effect);
		}


		float count = 0;


		RE::BSSimpleList<RE::ActiveEffect*>* effect_list = actor->AsMagicTarget()->GetActiveEffectList();

		if (!effect_list || effect_list->empty() == true) {
			logger::error("{} doesn't have any active effects.", actor->GetName());
			return 0;
		}

		for (auto active_effect : *effect_list)
		{
			bool active = active_effect->conditionStatus == RE::ActiveEffect::ConditionStatus::kTrue;

			//All other values are any.
			if (regard_ability == 1 && !active || regard_ability == 2 && active)
				continue;

			if (active_effect->GetBaseObject() == effect) {
				count++;

				if (!should_count)
					return count;
			}

		}

		return count;
	};

	
	float GetGameSetting(RE::Actor* target, std::string setting_name)
	{
		auto* singleton = RE::GameSettingCollection::GetSingleton();

		if (!singleton) {
			//report
			return 0;
		}

		RE::Setting* setting = singleton->GetSetting(setting_name.c_str());

		if (!setting) {
			//report
			return 0;
		}

		switch (setting->GetType())
		{
		case RE::Setting::Type::kBool:
			return setting->GetBool();

		case RE::Setting::Type::kFloat:
			return setting->GetFloat();

		case RE::Setting::Type::kSignedInteger:
			return setting->GetSInt();

		default:
			//report.
			logger::error("Setting type invalid.");
		}
		return 0;
	};

	//These 2 should account for context I think.
	float IsInFaction(RE::Actor* target, RE::TESForm* query_form, float min_rank, float max_rank)
	{
		if (!target)
			return 0;

		bool inverted = min_rank > max_rank;

		if (inverted) {
			std::swap(min_rank, max_rank);
		}

		if (!query_form)
			return 0;

		//logger::info("TEST, {}, {}", target->As<RE::BGSKeywordForm>() != nullptr, query_form->formType == RE::FormType::Keyword);

		if (query_form->formType != RE::FormType::Faction)
			return 0;
		//If the range is equal, it will only get what's current.
		if (min_rank == -1 && max_rank == -1) {
			return target->As<RE::Actor>()->IsInFaction(query_form->As<RE::TESFaction>());
		}
		else
		{
			RE::TESFaction* faction = query_form->As<RE::TESFaction>();
			RE::Actor* actor = target->As<RE::Actor>();

			auto* faction_changes = actor->extraList.GetByType<RE::ExtraFactionChanges>();

			auto fact_lamba = [=](auto& fact_data) {
				return fact_data.faction == faction &&
					fact_data.rank != -1 &&
					(fact_data.rank > min_rank - 1) == !inverted &&
					(fact_data.rank < max_rank + 1) == !inverted;
				};

			if (faction_changes)
			{
				auto end = faction_changes->factionChanges.end();
				auto it = std::find_if(faction_changes->factionChanges.begin(), end, fact_lamba);

				if (it != end)
					return 1;
			}
			auto baseNPC = actor->GetActorBase();
			if (baseNPC)
			{
				auto end = baseNPC->factions.end();
				auto it = std::find_if(baseNPC->factions.begin(), end, fact_lamba);

				if (it != end)
					return 1;
			}
		}
		//It shouldn't reach this point but it's whatever right now.

		return 0;
	};

	//external float IsRace(this Actor, Form race, float use_original);
	float IsRace(RE::Actor* target, RE::TESForm* query_form, float use_original)
	{
		if (!target || target->formType != RE::FormType::ActorCharacter)
			return 0;

		//if (!target || target->formType != RE::FormType::NPC)
		//	return 0;


		if (!query_form || query_form->formType != RE::FormType::Race)
			return 0;

		//we have 
		//actor->GetActorBase()->originalRace
		//and if player, charGenRace
		// For now I'm going to use original race. Unsure if it'll work.

		RE::TESNPC* base = target->GetActorBase();

		RE::TESRace* act_race = use_original && base ? base->originalRace : target->GetActorRuntimeData().race;

		return act_race == query_form->As<RE::TESRace>();

	};



	//These 2 should account for context I think.
	float GetKeyword(RE::Actor* target, RE::TESForm* query_form, float match_all)
	{
		if (!target)
			return 0;

		if (!query_form)
			return 0;

		//logger::info("TEST, {}, {}", target->As<RE::BGSKeywordForm>() != nullptr, query_form->formType == RE::FormType::Keyword);

		if (query_form->formType != RE::FormType::Keyword)
			return 0;

		if (query_form->formType == RE::FormType::FormList)
			return target->HasKeywordInList(query_form->As<RE::BGSListForm>(), match_all);

		if (RE::BGSKeywordForm* keyword_form = target->As<RE::BGSKeywordForm>(); keyword_form)
			return keyword_form->HasKeyword(query_form->As<RE::BGSKeyword>());

		if (RE::TESObjectREFR* refr = target->As<RE::TESObjectREFR>(); refr)
			return refr->HasKeyword(query_form->As<RE::BGSKeyword>());

		//It shouldn't reach this point but it's whatever right now.

		return 0;
	};


	float GetKeywordString(RE::Actor* target, std::string keyword_string)
	{

		if (!target)
			return 0;


		if (RE::BGSKeywordForm* keyword_form = target->As<RE::BGSKeywordForm>();
			keyword_form && keyword_string != "")
			return keyword_form->HasKeywordString(keyword_string);

		if (RE::TESObjectREFR* refr = target->As<RE::TESObjectREFR>(); refr)
			if (RE::BGSKeyword* keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(keyword_string); keyword)
				return refr->HasKeyword(keyword);


		return 0;
	};




	



	struct RegisterDump
	{
		inline static int no = 0;

		RegisterDump& operator=(bool result)
		{
			logger::info("#{} = {}", ++no, result);
			return *this;
		}
	};

	void FormulaRegister()
	{

		RegisterDump dump;


		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetRandomRange, "ActorValueGenerator::__Legacy__::GetRandomRange");					//01
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetActorValue, "ActorValueGenerator::__Legacy__::GetActorValue");						//02
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetCrimeGold, "ActorValueGenerator::__Legacy__::GetCrimeGold");						//03
		dump = LEX::ProcedureHandler::instance->RegisterFunction(IsRace, "ActorValueGenerator::__Legacy__::IsRace");									//04
		dump = LEX::ProcedureHandler::instance->RegisterFunction(IsInFaction, "ActorValueGenerator::__Legacy__::IsInFaction");							//05
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetKeyword, "ActorValueGenerator::__Legacy__::GetKeyword");							//06
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetHasMagicEffect, "ActorValueGenerator::__Legacy__::GetHasMagicEffect");				//07
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetItemCount, "ActorValueGenerator::__Legacy__::GetItemCount");						//08
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetGlobalValueParam, "ActorValueGenerator::__Legacy__::GetGlobalValueParam");			//09
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetKeywordString, "ActorValueGenerator::__Legacy__::GetKeywordString");				//10
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetGameSetting, "ActorValueGenerator::__Legacy__::GetGameSetting");					//11
		dump = LEX::ProcedureHandler::instance->RegisterFunction(HasPerks, "ActorValueGenerator::__Legacy__::HasPerks");								//12
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetPlayerLevelDistance, "ActorValueGenerator::__Legacy__::GetPlayerLevelDistance");	//13
		dump = LEX::ProcedureHandler::instance->RegisterFunction(GetLevel, "ActorValueGenerator::__Legacy__::GetLevel");								//14
	}

}