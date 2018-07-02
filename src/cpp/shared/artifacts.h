#ifndef TIED_ARTIFACT_H
#define TIED_ARTIFACT_H

#include <vector>

#define MAX_ARTIFACTS 14

enum ARTIFACT {
  ARTIFACT_ULTIMATE_BOOK_OF_KNOWLEDGE = 0,
  ARTIFACT_ULTIMATE_SWORD_OF_DOMINION = 1,
  ARTIFACT_ULTIMATE_CLOAK_OF_PROTECTION = 2,
  ARTIFACT_ULTIMATE_WAND_OF_MAGIC = 3,
  ARTIFACT_ULTIMATE_SHIELD = 4,
  ARTIFACT_ULTIMATE_STAFF = 5,
  ARTIFACT_ULTIMATE_CROWN = 6,
  ARTIFACT_GOLDEN_GOOSE = 7,
  ARTIFACT_ARCANE_NECKLACE_OF_MAGIC = 8,
  ARTIFACT_CASTERS_BRACELET_OF_MAGIC = 9,
  ARTIFACT_MAGES_RING_OF_POWER = 10,
  ARTIFACT_WITCHS_BROACH_OF_MAGIC = 11,
  ARTIFACT_MEDAL_OF_VALOR = 12,
  ARTIFACT_MEDAL_OF_COURAGE = 13,
  ARTIFACT_MEDAL_OF_HONOR = 14,
  ARTIFACT_MEDAL_OF_DISTINCTION = 15,
  ARTIFACT_FIZBIN_OF_MISFOURTUNE = 16,
  ARTIFACT_THUNDER_MACE_OF_DOMINION = 17,
  ARTIFACT_ARMORED_GAUNTLETS_OF_PROTECTION = 18,
  ARTIFACT_DEFENDER_HELM_OF_PROTECTION = 19,
  ARTIFACT_GIANT_FLAIL_OF_DOMINION = 20,
  ARTIFACT_BALLISTA_OF_QUICKNESS = 21,
  ARTIFACT_STEALTH_SHIELD_OF_PROTECTION = 22,
  ARTIFACT_DRAGON_SWORD_OF_DOMINION = 23,
  ARTIFACT_POWER_AXE_OF_DOMINION = 24,
  ARTIFACT_DIVINE_BREASTPLATE_OF_PROTECTION = 25,
  ARTIFACT_MINOR_SCROLL_OF_KNOWLEDGE = 26,
  ARTIFACT_MAJOR_SCROLL_OF_KNOWLEDGE = 27,
  ARTIFACT_SUPERIOR_SCROLL_OF_KNOWLEDGE = 28,
  ARTIFACT_FOREMOST_SCROLL_OF_KNOWLEDGE = 29,
  ARTIFACT_ENDLESS_SACK_OF_GOLD = 30,
  ARTIFACT_ENDLESS_BAG_OF_GOLD = 31,
  ARTIFACT_ENDLESS_PURSE_OF_GOLD = 32,
  ARTIFACT_NOMAD_BOOTS_OF_MOBILITY = 33,
  ARTIFACT_TRAVELERS_BOOTS_OF_MOBILITY = 34,
  ARTIFACT_LUCKY_RABBITS_FOOT = 35,
  ARTIFACT_GOLDEN_HORSESHOE = 36,
  ARTIFACT_GAMBLERS_LUCKY_COIN = 37,
  ARTIFACT_FOUR_LEAF_CLOVER = 38,
  ARTIFACT_TRUE_COMPASS_OF_MOBILITY = 39,
  ARTIFACT_SAILORS_ASTROLABE_OF_MOBILITY = 40,
  ARTIFACT_EVIL_EYE = 41,
  ARTIFACT_ENCHANTED_HOURGLASS = 42,
  ARTIFACT_GOLD_WATCH = 43,
  ARTIFACT_SKULLCAP = 44,
  ARTIFACT_ICE_CLOAK = 45,
  ARTIFACT_FIRE_CLOAK = 46,
  ARTIFACT_LIGHTNING_HELM = 47,
  ARTIFACT_EVERCOLD_ICICLE = 48,
  ARTIFACT_EVERHOT_LAVA_ROCK = 49,
  ARTIFACT_LIGHTNING_ROD = 50,
  ARTIFACT_SNAKE_RING = 51,
  ARTIFACT_ANKH = 52,
  ARTIFACT_BOOK_OF_ELEMENTS = 53,
  ARTIFACT_ELEMENTAL_RING = 54,
  ARTIFACT_HOLY_PENDANT = 55,
  ARTIFACT_PENDANT_OF_FREE_WILL = 56,
  ARTIFACT_PENDANT_OF_LIFE = 57,
  ARTIFACT_SERENITY_PENDANT = 58,
  ARTIFACT_SEEING_EYE_PENDANT = 59,
  ARTIFACT_KINETIC_PENDANT = 60,
  ARTIFACT_PENDANT_OF_DEATH = 61,
  ARTIFACT_WAND_OF_NEGATION = 62,
  ARTIFACT_GOLDEN_BOW = 63,
  ARTIFACT_TELESCOPE = 64,
  ARTIFACT_STATESMANS_QUILL = 65,
  ARTIFACT_WIZARDS_HAT = 66,
  ARTIFACT_POWER_RING = 67,
  ARTIFACT_AMMO_CART = 68,
  ARTIFACT_TAX_LIEN = 69,
  ARTIFACT_HIDEOUS_MASK = 70,
  ARTIFACT_ENDLESS_POUCH_OF_SULFUR = 71,
  ARTIFACT_ENDLESS_VIAL_OF_MERCURY = 72,
  ARTIFACT_ENDLESS_POUCH_OF_GEMS = 73,
  ARTIFACT_ENDLESS_CORD_OF_WOOD = 74,
  ARTIFACT_ENDLESS_CART_OF_ORE = 75,
  ARTIFACT_ENDLESS_POUCH_OF_CRYSTAL = 76,
  ARTIFACT_SPIKED_HELM = 77,
  ARTIFACT_SPIKED_SHIELD = 78,
  ARTIFACT_WHITE_PEARL = 79,
  ARTIFACT_BLACK_PEARL = 80,
  ARTIFACT_MAGIC_BOOK = 81,
  ARTIFACT_NONE1 = 82,
  ARTIFACT_NONE2 = 83,
  ARTIFACT_NONE3 = 84,
  ARTIFACT_NONE4 = 85,
  ARTIFACT_SPELL_SCROLL = 86,
  ARTIFACT_ARM_OF_THE_MARTYR = 87,
  ARTIFACT_BREASTPLATE_OF_ANDURAN = 88,
  ARTIFACT_BROACH_OF_SHIELDING = 89,
  ARTIFACT_BATTLE_GARB_OF_ANDURAN = 90,
  ARTIFACT_CRYSTAL_BALL = 91,
  ARTIFACT_HEART_OF_FIRE = 92,
  ARTIFACT_HEART_OF_ICE = 93,
  ARTIFACT_HELMET_OF_ANDURAN = 94,
  ARTIFACT_HOLY_HAMMER = 95,
  ARTIFACT_LEGENDARY_SCEPTER = 96,
  ARTIFACT_MASTHEAD = 97,
  ARTIFACT_SPHERE_OF_NEGATION = 98,
  ARTIFACT_STAFF_OF_WIZARDRY = 99,
  ARTIFACT_SWORD_BREAKER = 100,
  ARTIFACT_SWORD_OF_ANDURAN = 101,
  ARTIFACT_SPADE_OF_NECROMANCY = 102,
  ARTIFACT_PANDORA_BOX = 103,
};

enum ArtifactLevel {
  ARTIFACT_LEVEL_ULTIMATE = 1,
  ARTIFACT_LEVEL_MAJOR = 2,
  ARTIFACT_LEVEL_MINOR = 4,
  ARTIFACT_LEVEL_TREASURE = 8,
  ARTIFACT_LEVEL_SPELLBOOK = 16,
  ARTIFACT_LEVEL_UNUSED = 32
};

const int MAX_BASE_ARTIFACT = 81;
const int MIN_EXPANSION_ARTIFACT = 86;
const int MAX_EXPANSION_ARTIFACT = 102;
const int NUM_SUPPORTED_ARTIFACTS = 256;

void LoadArtifacts();
int __fastcall IsCursedItem(int);
bool IsArtifactGenerated(int);
void GenerateArtifact(int);
void ResetGeneratedArtifacts();
void ResetGeneratedArtifacts(int);
int GetArtifactLevel(int);

void DeserializeGeneratedArtifacts(const std::vector<int> &);
const std::vector<int> & SerializeGeneratedArtifacts();

extern char *gArtifactNames[];

#endif