#pragma once



#include "Arthmetic/ArthmeticUtility.h"//Move to rogues gallery

using namespace Arthmetic;

struct Utility
{	//SE: ???, AE: 0x3006808
	inline static float* g_deltaTime = (float*)REL::RelocationID(523660, 410199).address();

	//SE: ???, AE: 0x3006808
	inline static float* g_runTime = (float*)REL::RelocationID(523662, 410201).address();

	static float GetDeltaTime() { return *g_deltaTime; }
	static float GetRunTime() { return *g_runTime; }

	//Reuse this to use numbers instead of just looking for exact value, gives more info that way.
	static bool CharCmpI(const char& c1, const char& c2)
	{
		//May only have to do one.
		return c1 == c2 || std::toupper(c1) == std::toupper(c2);
	}


	static bool StrCmpI(const std::string_view& str1, const std::string_view& str2)
	{
		return str1.size() == str2.size() &&
			std::equal(str1.begin(), str1.end(), str2.begin(), CharCmpI);
	}
	

	static constexpr std::uint8_t NoOperation1[0x1]{ 0x90 };
	static_assert(sizeof(NoOperation1) == 0x1);

	static constexpr std::uint8_t NoOperation2[0x2]{ 0x66, 0x90 };
	static_assert(sizeof(NoOperation2) == 0x2);

	static constexpr std::uint8_t NoOperation3[0x3]{ 0x0F, 0x1F, 0x00 };
	static_assert(sizeof(NoOperation3) == 0x3);

	static constexpr std::uint8_t NoOperation4[0x4]{ 0x0F, 0x1F, 0x40, 0x00 };
	static_assert(sizeof(NoOperation4) == 0x4);

	static constexpr std::uint8_t NoOperation5[0x5]{ 0x0F, 0x1F, 0x44, 0x00, 0x00 };
	static_assert(sizeof(NoOperation5) == 0x5);

	static constexpr std::uint8_t NoOperation6[0x6]{ 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 };
	static_assert(sizeof(NoOperation6) == 0x6);

	static constexpr std::uint8_t NoOperation7[0x7]{ 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 };
	static_assert(sizeof(NoOperation7) == 0x7);

	static constexpr std::uint8_t NoOperation8[0x8]{ 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static_assert(sizeof(NoOperation8) == 0x8);

	static constexpr std::uint8_t NoOperation9[0x9]{ 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static_assert(sizeof(NoOperation9) == 0x9);

	static constexpr std::uint8_t NoOperationA[0xA]{ 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static_assert(sizeof(NoOperationA) == 0xA);

	static constexpr std::uint8_t NoOperationB[0xB]{ 0x66, 0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static_assert(sizeof(NoOperationB) == 0xB);
	
	//A keepsake from Parapets, may need to do more condition work in the future with that condition functions idea.
	union ConditionParam
	{
		char c;
		std::int32_t i;
		float f;
		RE::TESForm* form;
	};


	int myPow(int x, unsigned int p)
	{
		if (p == 0) return 1;
		if (p == 1) return x;

		int tmp = myPow(x, p / 2);
		if (p % 2 == 0) return tmp * tmp;
		else return x * tmp * tmp;
	}

	template <unsigned int p>
	int constexpr IntPower(const int x)
	{
		if constexpr (p == 0) return 1;
		if constexpr (p == 1) return x;

		int tmp = IntPower<p / 2>(x);
		if constexpr ((p % 2) == 0) { return tmp * tmp; }
		else { return x * tmp * tmp; }
	}
#ifndef cant_be_fucking_arsed_too_tired
		

	template <class StringType>
	static StringType FormTypeTo(RE::FormType type)
	{
#define CASE_OF_RETURN(mc_case_name) case RE::FormType::mc_case_name: return #mc_case_name##_ih
		get_switch(type)
		{
			CASE_OF_RETURN(None);
			CASE_OF_RETURN(PluginInfo);
			CASE_OF_RETURN(FormGroup);
			CASE_OF_RETURN(GameSetting);
			CASE_OF_RETURN(Keyword);
			CASE_OF_RETURN(LocationRefType);
			CASE_OF_RETURN(Action);
			CASE_OF_RETURN(TextureSet);
			CASE_OF_RETURN(MenuIcon);
			CASE_OF_RETURN(Global);
			CASE_OF_RETURN(Class);
			CASE_OF_RETURN(Faction);
			CASE_OF_RETURN(HeadPart);
			CASE_OF_RETURN(Eyes);
			CASE_OF_RETURN(Race);
			CASE_OF_RETURN(Sound);
			CASE_OF_RETURN(AcousticSpace);
			CASE_OF_RETURN(Skill);
			CASE_OF_RETURN(MagicEffect);
			CASE_OF_RETURN(Script);
			CASE_OF_RETURN(LandTexture);
			CASE_OF_RETURN(Enchantment);
			CASE_OF_RETURN(Spell);
			CASE_OF_RETURN(Scroll);
			CASE_OF_RETURN(Activator);
			CASE_OF_RETURN(TalkingActivator);
			CASE_OF_RETURN(Armor);
			CASE_OF_RETURN(Book);
			CASE_OF_RETURN(Container);
			CASE_OF_RETURN(Door);
			CASE_OF_RETURN(Ingredient);
			CASE_OF_RETURN(Light);
			CASE_OF_RETURN(Misc);
			CASE_OF_RETURN(Apparatus);
			CASE_OF_RETURN(Static);
			CASE_OF_RETURN(StaticCollection);
			CASE_OF_RETURN(MovableStatic);
			CASE_OF_RETURN(Grass);
			CASE_OF_RETURN(Tree);
			CASE_OF_RETURN(Flora);
			CASE_OF_RETURN(Furniture);
			CASE_OF_RETURN(Weapon);
			CASE_OF_RETURN(Ammo);
			CASE_OF_RETURN(NPC);
			CASE_OF_RETURN(LeveledNPC);
			CASE_OF_RETURN(KeyMaster);
			CASE_OF_RETURN(AlchemyItem);
			CASE_OF_RETURN(IdleMarker);
			CASE_OF_RETURN(Note);
			CASE_OF_RETURN(ConstructibleObject);
			CASE_OF_RETURN(Projectile);
			CASE_OF_RETURN(Hazard);
			CASE_OF_RETURN(SoulGem);
			CASE_OF_RETURN(LeveledItem);
			CASE_OF_RETURN(Weather);
			CASE_OF_RETURN(Climate);
			CASE_OF_RETURN(ShaderParticleGeometryData);
			CASE_OF_RETURN(ReferenceEffect);
			CASE_OF_RETURN(Region);
			CASE_OF_RETURN(Navigation);
			CASE_OF_RETURN(Cell);
			CASE_OF_RETURN(Reference);
			CASE_OF_RETURN(ActorCharacter);
			CASE_OF_RETURN(ProjectileMissile);
			CASE_OF_RETURN(ProjectileArrow);
			CASE_OF_RETURN(ProjectileGrenade);
			CASE_OF_RETURN(ProjectileBeam);
			CASE_OF_RETURN(ProjectileFlame);
			CASE_OF_RETURN(ProjectileCone);
			CASE_OF_RETURN(ProjectileBarrier);
			CASE_OF_RETURN(PlacedHazard);
			CASE_OF_RETURN(WorldSpace);
			CASE_OF_RETURN(Land);
			CASE_OF_RETURN(NavMesh);
			CASE_OF_RETURN(TLOD);
			CASE_OF_RETURN(Dialogue);
			CASE_OF_RETURN(Info);
			CASE_OF_RETURN(Quest);
			CASE_OF_RETURN(Idle);
			CASE_OF_RETURN(Package);
			CASE_OF_RETURN(CombatStyle);
			CASE_OF_RETURN(LoadScreen);
			CASE_OF_RETURN(LeveledSpell);
			CASE_OF_RETURN(AnimatedObject);
			CASE_OF_RETURN(Water);
			CASE_OF_RETURN(EffectShader);
			CASE_OF_RETURN(TOFT);
			CASE_OF_RETURN(Explosion);
			CASE_OF_RETURN(Debris);
			CASE_OF_RETURN(ImageSpace);
			CASE_OF_RETURN(ImageAdapter);
			CASE_OF_RETURN(FormList);
			CASE_OF_RETURN(Perk);
			CASE_OF_RETURN(BodyPartData);
			CASE_OF_RETURN(AddonNode);
			CASE_OF_RETURN(ActorValueInfo);
			CASE_OF_RETURN(CameraShot);
			CASE_OF_RETURN(CameraPath);
			CASE_OF_RETURN(VoiceType);
			CASE_OF_RETURN(MaterialType);
			CASE_OF_RETURN(Impact);
			CASE_OF_RETURN(ImpactDataSet);
			CASE_OF_RETURN(Armature);
			CASE_OF_RETURN(EncounterZone);
			CASE_OF_RETURN(Location);
			CASE_OF_RETURN(Message);
			CASE_OF_RETURN(Ragdoll);
			CASE_OF_RETURN(DefaultObject);
			CASE_OF_RETURN(LightingMaster);
			CASE_OF_RETURN(MusicType);
			CASE_OF_RETURN(Footstep);
			CASE_OF_RETURN(FootstepSet);
			CASE_OF_RETURN(StoryManagerBranchNode);
			CASE_OF_RETURN(StoryManagerQuestNode);
			CASE_OF_RETURN(StoryManagerEventNode);
			CASE_OF_RETURN(DialogueBranch);
			CASE_OF_RETURN(MusicTrack);
			CASE_OF_RETURN(DialogueView);
			CASE_OF_RETURN(WordOfPower);
			CASE_OF_RETURN(Shout);
			CASE_OF_RETURN(EquipSlot);
			CASE_OF_RETURN(Relationship);
			CASE_OF_RETURN(Scene);
			CASE_OF_RETURN(AssociationType);
			CASE_OF_RETURN(Outfit);
			CASE_OF_RETURN(ArtObject);
			CASE_OF_RETURN(MaterialObject);
			CASE_OF_RETURN(MovementType);
			CASE_OF_RETURN(SoundRecord);
			CASE_OF_RETURN(DualCastData);
			CASE_OF_RETURN(SoundCategory);
			CASE_OF_RETURN(SoundOutputModel);
			CASE_OF_RETURN(CollisionLayer);
			CASE_OF_RETURN(ColorForm);
			CASE_OF_RETURN(ReverbParam);
			CASE_OF_RETURN(LensFlare);
			CASE_OF_RETURN(LensSprite);
			CASE_OF_RETURN(VolumetricLighting);

			default:
				return "Max";
		}

#undef CASE_OF_RETURN
	}


	//Most common use would be for string views, given the static nature of these.
	template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
	static StringType ActorValueTo(RE::ActorValue av)
	{
#define CASE_OF_RETURN(mc_case_name) case RE::ActorValue::k##mc_case_name: return #mc_case_name;

		get_switch(av)
		{
			CASE_OF_RETURN(None);
			CASE_OF_RETURN(Aggression);
			CASE_OF_RETURN(Confidence);
			CASE_OF_RETURN(Energy);
			CASE_OF_RETURN(Morality);
			CASE_OF_RETURN(Mood);
			CASE_OF_RETURN(Assistance);
			CASE_OF_RETURN(OneHanded);
			CASE_OF_RETURN(TwoHanded);
			case RE::ActorValue::kArchery: return "Marksman";//Not using marksman is ridiculous
			//CASE_OF_RETURN(Archery);
			CASE_OF_RETURN(Block);
			CASE_OF_RETURN(Smithing);
			CASE_OF_RETURN(HeavyArmor);
			CASE_OF_RETURN(LightArmor);
			CASE_OF_RETURN(Pickpocket);
			CASE_OF_RETURN(Lockpicking);
			CASE_OF_RETURN(Sneak);
			CASE_OF_RETURN(Alchemy);
			CASE_OF_RETURN(Speech);
			CASE_OF_RETURN(Alteration);
			CASE_OF_RETURN(Conjuration);
			CASE_OF_RETURN(Destruction);
			CASE_OF_RETURN(Illusion);
			CASE_OF_RETURN(Restoration);
			CASE_OF_RETURN(Enchanting);
			CASE_OF_RETURN(Health);
			CASE_OF_RETURN(Magicka);
			CASE_OF_RETURN(Stamina);
			CASE_OF_RETURN(HealRate);
			CASE_OF_RETURN(MagickaRate);
			CASE_OF_RETURN(StaminaRate);
			CASE_OF_RETURN(SpeedMult);
			CASE_OF_RETURN(InventoryWeight);
			CASE_OF_RETURN(CarryWeight);
			CASE_OF_RETURN(CriticalChance);
			CASE_OF_RETURN(MeleeDamage);
			CASE_OF_RETURN(UnarmedDamage);
			CASE_OF_RETURN(Mass);
			CASE_OF_RETURN(VoicePoints);
			CASE_OF_RETURN(VoiceRate);
			CASE_OF_RETURN(DamageResist);
			CASE_OF_RETURN(PoisonResist);
			CASE_OF_RETURN(ResistFire);
			CASE_OF_RETURN(ResistShock);
			CASE_OF_RETURN(ResistFrost);
			CASE_OF_RETURN(ResistMagic);
			CASE_OF_RETURN(ResistDisease);
			CASE_OF_RETURN(PerceptionCondition);
			CASE_OF_RETURN(EnduranceCondition);
			CASE_OF_RETURN(LeftAttackCondition);
			CASE_OF_RETURN(RightAttackCondition);
			CASE_OF_RETURN(LeftMobilityCondition);
			CASE_OF_RETURN(RightMobilityCondition);
			CASE_OF_RETURN(BrainCondition);
			CASE_OF_RETURN(Paralysis);
			CASE_OF_RETURN(Invisibility);
			CASE_OF_RETURN(NightEye);
			CASE_OF_RETURN(DetectLifeRange);
			CASE_OF_RETURN(WaterBreathing);
			CASE_OF_RETURN(WaterWalking);
			CASE_OF_RETURN(IgnoreCrippledLimbs);
			CASE_OF_RETURN(Fame);
			CASE_OF_RETURN(Infamy);
			CASE_OF_RETURN(JumpingBonus);
			CASE_OF_RETURN(WardPower);
			CASE_OF_RETURN(RightItemCharge);
			CASE_OF_RETURN(ArmorPerks);
			CASE_OF_RETURN(ShieldPerks);
			CASE_OF_RETURN(WardDeflection);
			CASE_OF_RETURN(Variable01);
			CASE_OF_RETURN(Variable02);
			CASE_OF_RETURN(Variable03);
			CASE_OF_RETURN(Variable04);
			CASE_OF_RETURN(Variable05);
			CASE_OF_RETURN(Variable06);
			CASE_OF_RETURN(Variable07);
			CASE_OF_RETURN(Variable08);
			CASE_OF_RETURN(Variable09);
			CASE_OF_RETURN(Variable10);
			CASE_OF_RETURN(BowSpeedBonus);
			CASE_OF_RETURN(FavorActive);
			CASE_OF_RETURN(FavorsPerDay);
			CASE_OF_RETURN(FavorsPerDayTimer);
			CASE_OF_RETURN(LeftItemCharge);
			CASE_OF_RETURN(AbsorbChance);
			CASE_OF_RETURN(Blindness);
			CASE_OF_RETURN(WeaponSpeedMult);
			CASE_OF_RETURN(ShoutRecoveryMult);
			CASE_OF_RETURN(BowStaggerBonus);
			CASE_OF_RETURN(Telekinesis);
			CASE_OF_RETURN(FavorPointsBonus);
			CASE_OF_RETURN(LastBribedIntimidated);
			CASE_OF_RETURN(LastFlattered);
			CASE_OF_RETURN(MovementNoiseMult);
			CASE_OF_RETURN(BypassVendorStolenCheck);
			CASE_OF_RETURN(BypassVendorKeywordCheck);
			CASE_OF_RETURN(WaitingForPlayer);
			CASE_OF_RETURN(OneHandedModifier);
			CASE_OF_RETURN(TwoHandedModifier);
			CASE_OF_RETURN(MarksmanModifier);
			CASE_OF_RETURN(BlockModifier);
			CASE_OF_RETURN(SmithingModifier);
			CASE_OF_RETURN(HeavyArmorModifier);
			CASE_OF_RETURN(LightArmorModifier);
			CASE_OF_RETURN(PickpocketModifier);
			CASE_OF_RETURN(LockpickingModifier);
			CASE_OF_RETURN(SneakingModifier);
			CASE_OF_RETURN(AlchemyModifier);
			CASE_OF_RETURN(SpeechcraftModifier);
			CASE_OF_RETURN(AlterationModifier);
			CASE_OF_RETURN(ConjurationModifier);
			CASE_OF_RETURN(DestructionModifier);
			CASE_OF_RETURN(IllusionModifier);
			CASE_OF_RETURN(RestorationModifier);
			CASE_OF_RETURN(EnchantingModifier);
			CASE_OF_RETURN(OneHandedSkillAdvance);
			CASE_OF_RETURN(TwoHandedSkillAdvance);
			CASE_OF_RETURN(MarksmanSkillAdvance);
			CASE_OF_RETURN(BlockSkillAdvance);
			CASE_OF_RETURN(SmithingSkillAdvance);
			CASE_OF_RETURN(HeavyArmorSkillAdvance);
			CASE_OF_RETURN(LightArmorSkillAdvance);
			CASE_OF_RETURN(PickpocketSkillAdvance);
			CASE_OF_RETURN(LockpickingSkillAdvance);
			CASE_OF_RETURN(SneakingSkillAdvance);
			CASE_OF_RETURN(AlchemySkillAdvance);
			CASE_OF_RETURN(SpeechcraftSkillAdvance);
			CASE_OF_RETURN(AlterationSkillAdvance);
			CASE_OF_RETURN(ConjurationSkillAdvance);
			CASE_OF_RETURN(DestructionSkillAdvance);
			CASE_OF_RETURN(IllusionSkillAdvance);
			CASE_OF_RETURN(RestorationSkillAdvance);
			CASE_OF_RETURN(EnchantingSkillAdvance);
			CASE_OF_RETURN(LeftWeaponSpeedMultiply);
			CASE_OF_RETURN(DragonSouls);
			CASE_OF_RETURN(CombatHealthRegenMultiply);
			CASE_OF_RETURN(OneHandedPowerModifier);
			CASE_OF_RETURN(TwoHandedPowerModifier);
			CASE_OF_RETURN(MarksmanPowerModifier);
			CASE_OF_RETURN(BlockPowerModifier);
			CASE_OF_RETURN(SmithingPowerModifier);
			CASE_OF_RETURN(HeavyArmorPowerModifier);
			CASE_OF_RETURN(LightArmorPowerModifier);
			CASE_OF_RETURN(PickpocketPowerModifier);
			CASE_OF_RETURN(LockpickingPowerModifier);
			CASE_OF_RETURN(SneakingPowerModifier);
			CASE_OF_RETURN(AlchemyPowerModifier);
			CASE_OF_RETURN(SpeechcraftPowerModifier);
			CASE_OF_RETURN(AlterationPowerModifier);
			CASE_OF_RETURN(ConjurationPowerModifier);
			CASE_OF_RETURN(DestructionPowerModifier);
			CASE_OF_RETURN(IllusionPowerModifier);
			CASE_OF_RETURN(RestorationPowerModifier);
			CASE_OF_RETURN(EnchantingPowerModifier);
			CASE_OF_RETURN(DragonRend);
			CASE_OF_RETURN(AttackDamageMult);
			CASE_OF_RETURN(HealRateMult);
			CASE_OF_RETURN(MagickaRateMult);
			CASE_OF_RETURN(StaminaRateMult);
			CASE_OF_RETURN(WerewolfPerks);
			CASE_OF_RETURN(VampirePerks);
			CASE_OF_RETURN(GrabActorOffset);
			CASE_OF_RETURN(Grabbed);
			CASE_OF_RETURN(DEPRECATED05);
			CASE_OF_RETURN(ReflectDamage);

		default:
			return "Total";
		}

#undef CASE_OF_RETURN
#undef kStaminaRate
	}


	//template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
	static RE::ActorValue StringToActorValue(std::string str)
	{
#define CASE_OF_RETURN(mc_case_name) case #mc_case_name##_ih: return RE::ActorValue::k##mc_case_name

		get_switch(Arthmetic::hash<true>(str))
		{

			CASE_OF_RETURN(None);
			CASE_OF_RETURN(Aggression);
			CASE_OF_RETURN(Confidence);
			CASE_OF_RETURN(Energy);
			CASE_OF_RETURN(Morality);
			CASE_OF_RETURN(Mood);
			CASE_OF_RETURN(Assistance);
			CASE_OF_RETURN(OneHanded);
			CASE_OF_RETURN(TwoHanded);
			
			case"Marksman"_ih:
			CASE_OF_RETURN(Archery);
			CASE_OF_RETURN(Block);
			CASE_OF_RETURN(Smithing);
			CASE_OF_RETURN(HeavyArmor);
			CASE_OF_RETURN(LightArmor);
			CASE_OF_RETURN(Pickpocket);
			CASE_OF_RETURN(Lockpicking);
			CASE_OF_RETURN(Sneak);
			CASE_OF_RETURN(Alchemy);
			CASE_OF_RETURN(Speech);
			CASE_OF_RETURN(Alteration);
			CASE_OF_RETURN(Conjuration);
			CASE_OF_RETURN(Destruction);
			CASE_OF_RETURN(Illusion);
			CASE_OF_RETURN(Restoration);
			CASE_OF_RETURN(Enchanting);
			CASE_OF_RETURN(Health);
			CASE_OF_RETURN(Magicka);
			CASE_OF_RETURN(Stamina);
			CASE_OF_RETURN(HealRate);
			CASE_OF_RETURN(MagickaRate);
			CASE_OF_RETURN(StaminaRate);
			CASE_OF_RETURN(SpeedMult);
			CASE_OF_RETURN(InventoryWeight);
			CASE_OF_RETURN(CarryWeight);
			CASE_OF_RETURN(CriticalChance);
			CASE_OF_RETURN(MeleeDamage);
			CASE_OF_RETURN(UnarmedDamage);
			CASE_OF_RETURN(Mass);
			CASE_OF_RETURN(VoicePoints);
			CASE_OF_RETURN(VoiceRate);
			CASE_OF_RETURN(DamageResist);
			CASE_OF_RETURN(PoisonResist);
			CASE_OF_RETURN(ResistFire);
			CASE_OF_RETURN(ResistShock);
			CASE_OF_RETURN(ResistFrost);
			CASE_OF_RETURN(ResistMagic);
			CASE_OF_RETURN(ResistDisease);
			CASE_OF_RETURN(PerceptionCondition);
			CASE_OF_RETURN(EnduranceCondition);
			CASE_OF_RETURN(LeftAttackCondition);
			CASE_OF_RETURN(RightAttackCondition);
			CASE_OF_RETURN(LeftMobilityCondition);
			CASE_OF_RETURN(RightMobilityCondition);
			CASE_OF_RETURN(BrainCondition);
			CASE_OF_RETURN(Paralysis);
			CASE_OF_RETURN(Invisibility);
			CASE_OF_RETURN(NightEye);
			CASE_OF_RETURN(DetectLifeRange);
			CASE_OF_RETURN(WaterBreathing);
			CASE_OF_RETURN(WaterWalking);
			CASE_OF_RETURN(IgnoreCrippledLimbs);
			CASE_OF_RETURN(Fame);
			CASE_OF_RETURN(Infamy);
			CASE_OF_RETURN(JumpingBonus);
			CASE_OF_RETURN(WardPower);
			CASE_OF_RETURN(RightItemCharge);
			CASE_OF_RETURN(ArmorPerks);
			CASE_OF_RETURN(ShieldPerks);
			CASE_OF_RETURN(WardDeflection);
			CASE_OF_RETURN(Variable01);
			CASE_OF_RETURN(Variable02);
			CASE_OF_RETURN(Variable03);
			CASE_OF_RETURN(Variable04);
			CASE_OF_RETURN(Variable05);
			CASE_OF_RETURN(Variable06);
			CASE_OF_RETURN(Variable07);
			CASE_OF_RETURN(Variable08);
			CASE_OF_RETURN(Variable09);
			CASE_OF_RETURN(Variable10);
			CASE_OF_RETURN(BowSpeedBonus);
			CASE_OF_RETURN(FavorActive);
			CASE_OF_RETURN(FavorsPerDay);
			CASE_OF_RETURN(FavorsPerDayTimer);
			CASE_OF_RETURN(LeftItemCharge);
			CASE_OF_RETURN(AbsorbChance);
			CASE_OF_RETURN(Blindness);
			CASE_OF_RETURN(WeaponSpeedMult);
			CASE_OF_RETURN(ShoutRecoveryMult);
			CASE_OF_RETURN(BowStaggerBonus);
			CASE_OF_RETURN(Telekinesis);
			CASE_OF_RETURN(FavorPointsBonus);
			CASE_OF_RETURN(LastBribedIntimidated);
			CASE_OF_RETURN(LastFlattered);
			CASE_OF_RETURN(MovementNoiseMult);
			CASE_OF_RETURN(BypassVendorStolenCheck);
			CASE_OF_RETURN(BypassVendorKeywordCheck);
			CASE_OF_RETURN(WaitingForPlayer);
			CASE_OF_RETURN(OneHandedModifier);
			CASE_OF_RETURN(TwoHandedModifier);
			CASE_OF_RETURN(MarksmanModifier);
			CASE_OF_RETURN(BlockModifier);
			CASE_OF_RETURN(SmithingModifier);
			CASE_OF_RETURN(HeavyArmorModifier);
			CASE_OF_RETURN(LightArmorModifier);
			CASE_OF_RETURN(PickpocketModifier);
			CASE_OF_RETURN(LockpickingModifier);
			CASE_OF_RETURN(SneakingModifier);
			CASE_OF_RETURN(AlchemyModifier);
			CASE_OF_RETURN(SpeechcraftModifier);
			CASE_OF_RETURN(AlterationModifier);
			CASE_OF_RETURN(ConjurationModifier);
			CASE_OF_RETURN(DestructionModifier);
			CASE_OF_RETURN(IllusionModifier);
			CASE_OF_RETURN(RestorationModifier);
			CASE_OF_RETURN(EnchantingModifier);
			CASE_OF_RETURN(OneHandedSkillAdvance);
			CASE_OF_RETURN(TwoHandedSkillAdvance);
			CASE_OF_RETURN(MarksmanSkillAdvance);
			CASE_OF_RETURN(BlockSkillAdvance);
			CASE_OF_RETURN(SmithingSkillAdvance);
			CASE_OF_RETURN(HeavyArmorSkillAdvance);
			CASE_OF_RETURN(LightArmorSkillAdvance);
			CASE_OF_RETURN(PickpocketSkillAdvance);
			CASE_OF_RETURN(LockpickingSkillAdvance);
			CASE_OF_RETURN(SneakingSkillAdvance);
			CASE_OF_RETURN(AlchemySkillAdvance);
			CASE_OF_RETURN(SpeechcraftSkillAdvance);
			CASE_OF_RETURN(AlterationSkillAdvance);
			CASE_OF_RETURN(ConjurationSkillAdvance);
			CASE_OF_RETURN(DestructionSkillAdvance);
			CASE_OF_RETURN(IllusionSkillAdvance);
			CASE_OF_RETURN(RestorationSkillAdvance);
			CASE_OF_RETURN(EnchantingSkillAdvance);
			CASE_OF_RETURN(LeftWeaponSpeedMultiply);
			CASE_OF_RETURN(DragonSouls);
			CASE_OF_RETURN(CombatHealthRegenMultiply);
			CASE_OF_RETURN(OneHandedPowerModifier);
			CASE_OF_RETURN(TwoHandedPowerModifier);
			CASE_OF_RETURN(MarksmanPowerModifier);
			CASE_OF_RETURN(BlockPowerModifier);
			CASE_OF_RETURN(SmithingPowerModifier);
			CASE_OF_RETURN(HeavyArmorPowerModifier);
			CASE_OF_RETURN(LightArmorPowerModifier);
			CASE_OF_RETURN(PickpocketPowerModifier);
			CASE_OF_RETURN(LockpickingPowerModifier);
			CASE_OF_RETURN(SneakingPowerModifier);
			CASE_OF_RETURN(AlchemyPowerModifier);
			CASE_OF_RETURN(SpeechcraftPowerModifier);
			CASE_OF_RETURN(AlterationPowerModifier);
			CASE_OF_RETURN(ConjurationPowerModifier);
			CASE_OF_RETURN(DestructionPowerModifier);
			CASE_OF_RETURN(IllusionPowerModifier);
			CASE_OF_RETURN(RestorationPowerModifier);
			CASE_OF_RETURN(EnchantingPowerModifier);
			CASE_OF_RETURN(DragonRend);
			CASE_OF_RETURN(AttackDamageMult);
			CASE_OF_RETURN(HealRateMult);
			CASE_OF_RETURN(MagickaRateMult);
			CASE_OF_RETURN(StaminaRateMult);
			CASE_OF_RETURN(WerewolfPerks);
			CASE_OF_RETURN(VampirePerks);
			CASE_OF_RETURN(GrabActorOffset);
			CASE_OF_RETURN(Grabbed);
			CASE_OF_RETURN(DEPRECATED05);
			CASE_OF_RETURN(ReflectDamage);


	default:
		return RE::ActorValue::kTotal;
		}

#undef CASE_OF_RETURN
#undef kStaminaRate
	}

	static bool IsValidValue(RE::ActorValue av, bool include_extra = false)
	{
		switch (av)
		{
		case RE::ActorValue::kNone:
		case RE::ActorValue::kTotal:
			return false;
		default:
			return (uint32_t)av < (uint32_t)RE::ActorValue::kTotal;
		}
	}

		
#endif
	
};


namespace RGL
{
	/*
	template <typename T>
	concept pointer_type = std::is_pointer_v<T> ||
		(requires(T pointer) {
			{ pointer.operator->() } -> std::same_as<typename std::remove_reference_t<T>::pointer >;
	}
	&& (requires(T pointer) {
		{ pointer.operator*() } -> std::same_as<typename std::add_lvalue_reference<typename std::remove_reference_t<T>::element_type>::type>;
	})
		);


	template <pointer_type T>
	struct RemovePtrRef { using type = std::pointer_traits<T>::element_type; };//std::remove_pointer_t<PointerType>;//

	template <class T>
	using RemovePtrRefT = RemovePtrRef<T>::Type;
	//*/
	
	
	//A wrapper class for the type_info that makes it easier to use types as
	// The idea is that you use a template type so you can do stuff like this
	//last_process == Type<ParsingObject>
	struct Type
	{
		const type_info& info = typeid(void);

		Type() = default;

		constexpr Type(const type_info& i) : info(i) {}
		
		//Cant get this to work properly, so I'll not be doing it
		//template <class T>
		//constexpr Type(T& i) : info(typeid(RemovePtrRefT<T>)) {}
	
		//template <class T>
		//constexpr Type(T&& i) : info(typeid(RemovePtrRefT<T>)) {}

		bool operator ==(const Type& a_rhs) const noexcept { return info.hash_code() == a_rhs.info.hash_code(); }
	};

	//template <class T>
	//struct TypeInfo : Type
	//{
	//	constexpr Type() : Type(typeid(T)) {}
	//};

	template <class T>
	constexpr Type TypeInfo = Type(typeid(T));

	inline bool TestFunc_Type()
	{

		Type test_type = Type(TypeInfo<int>);

		if (test_type != TypeInfo<int>)
			return false;

		return true;
	}
}

namespace std
{
	//template
	//std::array
}
