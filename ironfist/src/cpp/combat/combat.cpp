#include "base.h"
#include "adventure/adv.h"
#include "combat/combat.h"
#include "game/game.h"
#include "resource/resourceManager.h"
#include "artifacts.h"
#include "skills.h"
#include "sound/sound.h"
#include "spells.h"

#include "gui/dialog.h"

#include "scripting/hook.h"
#include <string>

extern int giNextAction;
extern signed char gbCombatSurrender;
extern signed char gbRetreatWin;
extern char *cMonFilename[]; // it's inside creature.cpp
extern char *cArmyFrameFileNames[]; // it's inside creature.cpp
extern icon *gCurLoadedSpellIcon;
extern unsigned __int8 giNumPowFrames[32];
extern int gCurSpellEffectFrame;
extern int gCurLoadedSpellEffect;

char *gCombatFxNames[33] =
{
	"",
	"magic01.icn",
	"magic02.icn",
	"magic03.icn",
	"magic04.icn",
	"magic05.icn",
	"magic06.icn",
	"magic07.icn",
	"magic08.icn",
	"rainbluk.icn",
	"cloudluk.icn",
	"moraleg.icn",
	"moraleb.icn",
	"reddeath.icn",
	"redfire.icn",
	"sparks.icn",
	"electric.icn",
	"physical.icn",
	"bluefire.icn",
	"icecloud.icn",
	"lichclod.icn",
	"bless.icn",
	"berzerk.icn",
	"shield.icn",
	"haste.icn",
	"paralyze.icn",
	"hypnotiz.icn",
	"dragslay.icn",
	"blind.icn",
	"curse.icn",
	"stonskin.icn",
	"stelskin.icn",
	"plasmblast.icn"
};

int squaresAroundCaster[2][3] = {
	{14,27,40},
	{11,24,37}
};

void combatManager::InitNonVisualVars() {
	combatManager::InitNonVisualVars_orig();
        
        ScriptSignal(SCRIPT_EVT_BATTLE_START, "");

	for(int i = 0; i < 2; i++) {
		HandlePandoraBox(i);
	}
}

/*
* What happens when a hero wins a battle using Pandora's Box, but loses their
* main army? They walk around with stacks of 0 creatures, of course! 
* 0-creature stacks are still useful in combat though, since all attacks do at least 1 damage.
*
* On a related note, in the original game, what happens when a hero wins a battle
* using temporarily-resurrected creatures, but has no army left at the end? They
* walk around with no creatures, and instantly lose their next battle.
*
* Winning a battle with nothing but summoned elementals remaining works, however.
*
* TL, DR: There is a bug when winning a battle with nothing but temporary creatures left,
* but it's also present in the original game.
*/
void combatManager::HandlePandoraBox(int side) {
	if(this->heroes[side] && this->heroes[side]->HasArtifact(ARTIFACT_PANDORA_BOX)) {

		//The HoMM II code appears to lack a definition of creature tier. This deserves investigation.
		//We temporarily hardcode the tier-1 creatures
		int creatChoices[] = {
			CREATURE_PEASANT, CREATURE_SPRITE,CREATURE_HALFLING, CREATURE_GOBLIN,
			CREATURE_SKELETON, CREATURE_CENTAUR, CREATURE_ROGUE, CREATURE_BLOODSUCKER
		};
		int creat = creatChoices[SRandom(0, ELEMENTS_IN(creatChoices)-1)];

		int hex = -1;
		int poss = ELEMENTS_IN(squaresAroundCaster[side]);
		int tryFirst = SRandom(0, poss-1);
		for(int i = 0; i < poss; i++) {
			int square = squaresAroundCaster[side][(i+tryFirst)%poss];
			if(gMonsterDatabase[creat].creature_flags & TWO_HEXER) {
				int dir = side == 0 ? 1 : -1;
				if(this->combatGrid[square+dir].unitOwner != -1)
					continue;
			}
			if(this->combatGrid[square].unitOwner == -1)
				hex = square;
		}

		if(hex==-1)
			return;

		int amt = gpGame->GetRandomNumTroops(creat);
		AddArmy(side, creat, amt, hex, 0x8000, 0);

		hexcell* cell = &this->combatGrid[hex];
		this->creatures[cell->unitOwner][cell->stackIdx].temporaryQty = amt;
	}
}

void army::MoveTo(int hexIdx) {
	if(this->creature.creature_flags & FLYER) {
		this->targetHex = hexIdx;
		if(ValidFlight(this->targetHex, 0))
			FlyTo(this->targetHex);
	} else {
		WalkTo(hexIdx);
	}
}

void army::MoveAttack(int targHex, int x) {
	int startHex = this->occupiedHex;
	this->MoveAttack_orig(targHex, x);

	if( !(this->creature.creature_flags & DEAD) &&
		CreatureHasAttribute(this->creatureIdx, STRIKE_AND_RETURN)) {
		MoveTo(startHex);
	}
}

void army::DoAttack(int x) {
  army* primaryTarget = &gpCombatManager->creatures[gpCombatManager->combatGrid[targetHex].unitOwner][gpCombatManager->combatGrid[targetHex].stackIdx];
  if (gpCombatManager->combatGrid[targetHex].unitOwner < 0 || gpCombatManager->combatGrid[targetHex].stackIdx < 0)
	  primaryTarget = this;
  ScriptSetSpecialVariableData("__attackingStack", this);
  ScriptSetSpecialVariableData("__targetStack", primaryTarget);
  std::string tmp = std::to_string(this->creatureIdx) + "," + std::to_string(primaryTarget->creatureIdx);
  ScriptSignal(SCRIPT_EVT_BATTLE_ATTACK_M, tmp);
  this->DoAttack_orig(x);
 }

/*
void army::SpecialAttack() {
	army* primaryTarget = &gpCombatManager->creatures[gpCombatManager->combatGrid[targetHex].unitOwner][gpCombatManager->combatGrid[targetHex].stackIdx];
	ScriptSetSpecialVariableData("__attackingStack", this);
	ScriptSetSpecialVariableData("__targetStack", primaryTarget);
	std::string tmp = std::to_string(this->creatureIdx) + "," + std::to_string(primaryTarget->creatureIdx);
	ScriptSignal(SCRIPT_EVT_BATTLE_ATTACK_S, tmp);
	if (this->creatureIdx == CREATURE_MAGE) { // temporary creature. cyber behemoth attack
		gpCombatManager->CastSpell(SPELL_FIREBLAST, primaryTarget->occupiedHex, 1, 0);
	}
	else this->SpecialAttack_orig();
}
*/
// We don't actually change anything in sElevationOverlay, but the disasm was causing some problems

#pragma pack(push, 1)
struct SElevationOverlay {
  __int16 terrains;
  char coveredHexes[15];
};
#pragma pack(pop)

SElevationOverlay sElevationOverlay[25] =
{
  {
    0,
    {
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255,
      255
    }
  },
  { 2, { 30, 31, 32, 33, 47, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255 } },
  {
    2,
    { 56, 57, 58, 59, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  { 2, { 42, 55, 56, 57, 58, 59, 60, 48, 255, 255, 255, 255, 255, 255, 255 } },
  { 2, { 69, 70, 71, 72, 73, 60, 48, 255, 255, 255, 255, 255, 255, 255, 255 } },
  { 2, { 29, 30, 31, 32, 33, 34, 35, 81, 69, 70, 71, 72, 73, 74, 87 } },
  { 2, { 29, 17, 18, 19, 20, 21, 81, 95, 96, 97, 98, 99, 255, 255, 255 } },
  { 4, { 30, 31, 32, 33, 47, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255 } },
  {
    4,
    { 56, 57, 58, 59, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  { 4, { 42, 55, 56, 57, 58, 59, 47, 255, 255, 255, 255, 255, 255, 255, 255 } },
  { 4, { 69, 70, 71, 72, 73, 60, 48, 255, 255, 255, 255, 255, 255, 255, 255 } },
  { 4, { 18, 30, 43, 84, 85, 73, 60, 255, 255, 255, 255, 255, 255, 255, 255 } },
  { 4, { 21, 34, 48, 70, 83, 97, 98, 255, 255, 255, 255, 255, 255, 255, 255 } },
  {
    64,
    { 30, 31, 32, 33, 47, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  {
    64,
    { 56, 57, 58, 59, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  { 64, { 42, 55, 56, 57, 58, 59, 60, 48, 255, 255, 255, 255, 255, 255, 255 } },
  { 64, { 69, 70, 71, 72, 73, 60, 48, 255, 255, 255, 255, 255, 255, 255, 255 } },
  { 64, { 29, 30, 31, 32, 33, 34, 35, 81, 69, 70, 71, 72, 73, 74, 87 } },
  { 64, { 29, 17, 18, 19, 20, 21, 81, 95, 96, 97, 98, 99, 255, 255, 255 } },
  {
    128,
    { 30, 31, 32, 33, 47, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  {
    128,
    { 56, 57, 58, 59, 60, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  { 128, { 42, 55, 56, 57, 58, 59, 60, 48, 255, 255, 255, 255, 255, 255, 255 } },
  {
    128,
    { 69, 70, 71, 72, 73, 60, 48, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  {
    128,
    { 43, 30, 18, 84, 85, 73, 60, 255, 255, 255, 255, 255, 255, 255, 255 }
  },
  {
    128,
    { 21, 34, 48, 70, 83, 97, 98, 255, 255, 255, 255, 255, 255, 255, 255 }
  }
};

void SpecialAttackBattleMessage(army *attacker, army *target, int creaturesKilled, int damageDone) {
	char *attackingCreature;
	if (creaturesKilled <= 0) {
		if (attacker->quantity <= 1)
			attackingCreature = GetCreatureName(attacker->creatureIdx);
		else
			attackingCreature = GetCreaturePluralName(attacker->creatureIdx);
		sprintf(gText, "%s %s %d damage.", attackingCreature, (attacker->quantity > 1) ? "do" : "does", damageDone);
		gText[0] = toupper(gText[0]);
	}
	else {
		if (damageDone == -1) {
			sprintf(gText, "The mirror image is destroyed!");
		} else {
			char *targetCreature;
			if (creaturesKilled <= 1)
				targetCreature = GetCreatureName(target->creatureIdx);
			else
				targetCreature = GetCreaturePluralName(target->creatureIdx);
			if (attacker->quantity <= 1)
				attackingCreature = GetCreatureName(attacker->creatureIdx);
			else
				attackingCreature = GetCreaturePluralName(attacker->creatureIdx);
			sprintf(
				gText,
				"%s %s %d damage.\n%d %s %s.",
				attackingCreature,
				(attacker->quantity > 1) ? "do" : "does",
				damageDone,
				creaturesKilled,
				targetCreature,
				(creaturesKilled > 1) ? "perish" : "perishes");
			gText[0] = toupper(gText[0]);
		}
	}
	gpCombatManager->CombatMessage(gText, 1, 1, 0);
}

void OccupyHexes(army *a) {
	if (a->facingRight == 1)
		a->occupiedHex--;
	else
		a->occupiedHex++;
}

void ProcessSecondAttack(army *attacker, army *target) {
	if (!bSecondAttack && target->quantity > 0) {
		bSecondAttack = 1;
		attacker->SpecialAttack();
		bSecondAttack = 0;
	}
}

void SpecialAttackGraphics(army *attacker, army *target) {
	gpCombatManager->ResetLimitCreature();
	gpCombatManager->limitCreature[attacker->owningSide][attacker->stackIdx]++;
	gpCombatManager->DrawFrame(0, 1, 0, 1, 75, 1, 1);

	int targMidX = target->MidX();
	int targMidY = target->MidY();
	if (attacker->creatureIdx == CREATURE_LICH || attacker->creatureIdx == CREATURE_POWER_LICH)	{
		targMidX = gpCombatManager->combatGrid[target->occupiedHex].centerX;
		targMidY = gpCombatManager->combatGrid[target->occupiedHex].occupyingCreatureBottomY - 17;
	}
	int projStartX;
	if (attacker->facingRight == 1)
		projStartX = attacker->frameInfo.projectileStartOffset[1][0] + gpCombatManager->combatGrid[attacker->occupiedHex].centerX;
	else
		projStartX = gpCombatManager->combatGrid[attacker->occupiedHex].centerX - attacker->frameInfo.projectileStartOffset[1][0];
	int projStartY = attacker->frameInfo.projectileStartOffset[1][1] + gpCombatManager->combatGrid[attacker->occupiedHex].occupyingCreatureBottomY;
	int totXDiff = targMidX - projStartX;
	char firingLeft = 0;
	if (targMidX - projStartX < 0) {
		firingLeft = 1;
		totXDiff = -totXDiff;
	}
	int yDiff = targMidY - projStartY;

	float angleDeg;
	int spriteIdx;
	if (totXDiff) {
		float slope = (double)-yDiff / (double)totXDiff;
		angleDeg = atan(slope) * 180.0 / 3.14159;
		int i;
		for (i = 1;	attacker->frameInfo.numMissileDirs > i &&
			(*(float *)((char *)&attacker->frameInfo.projectileStartOffset[2 * i + 4] + 1) +
				attacker->frameInfo.projDirAngle[i]) / 2.0 >= angleDeg;	++i)
			;
		if (attacker->frameInfo.numMissileDirs <= i)
			spriteIdx = attacker->frameInfo.numMissileDirs - 1;
		else
			spriteIdx = i - 1;
	} else {
		if (yDiff <= 0)
			spriteIdx = 0;
		else
			spriteIdx = attacker->frameInfo.numMissileDirs - 1;
		angleDeg = (double)(yDiff <= 0 ? 90 : -90);
	}

	int attackDirectionAnimationIdx;
	if (angleDeg <= 25.0) {
		if (angleDeg <= -25.0) {
			attacker->animationType = ANIMATION_TYPE_RANGED_ATTACK_DOWNWARDS;
			attackDirectionAnimationIdx = 2;
		}
		else {
			attacker->animationType = ANIMATION_TYPE_RANGED_ATTACK_FORWARDS;
			attackDirectionAnimationIdx = 1;
		}
	} else {
		attacker->animationType = ANIMATION_TYPE_RANGED_ATTACK_UPWARDS;
		attackDirectionAnimationIdx = 0;
	}
	for (attacker->animationFrame = 0;
		attacker->frameInfo.animationLengths[attacker->animationType] > attacker->animationFrame;
		++attacker->animationFrame)
	{
		if (attacker->frameInfo.animationLengths[attacker->animationType] - 1 == attacker->animationFrame)
			gpCombatManager->DrawFrame(0, 1, 0, 0, 75, 1, 1);
		else
			gpCombatManager->DrawFrame(1, 1, 0, 0, 75, 1, 1);
		glTimers = (signed __int64)((double)KBTickCount()
			+ (double)attacker->frameInfo.shootingTime
			* gfCombatSpeedMod[giCombatSpeed]
			/ (double)attacker->frameInfo.animationLengths[attacker->animationType]);
	}
	attacker->animationFrame = attacker->frameInfo.animationLengths[attacker->animationType] - 1;
	int v27 = 25;
	int v18 = 25;
	int v35 = 31;
	int v22 = 25;
	if (attacker->creatureIdx == CREATURE_LICH || attacker->creatureIdx == CREATURE_POWER_LICH) {
		v35 = 26;
		v22 = 7;
		v27 = 10;
		v18 = 10;
	}
	if (attacker->creatureIdx == CREATURE_CYBER_BEHEMOTH) {
		v27 = 100;
		v18 = 100;
		v35 = 31;
		v22 = 25;
	}
	int v20 = 0;
	int offsetX = 639;
	int v15 = 0;
	int offsetY = 479;
	int startX;
	if (attacker->facingRight == 1)
		startX = attacker->frameInfo.projectileStartOffset[attackDirectionAnimationIdx][0] + gpCombatManager->combatGrid[attacker->occupiedHex].centerX;
	else
		startX = gpCombatManager->combatGrid[attacker->occupiedHex].centerX - attacker->frameInfo.projectileStartOffset[attackDirectionAnimationIdx][0];
	
	int startY = attacker->frameInfo.projectileStartOffset[attackDirectionAnimationIdx][1] + gpCombatManager->combatGrid[attacker->occupiedHex].occupyingCreatureBottomY;

	int endX = target->MidX();
	int endY = target->MidY();
	int diffX = endX - startX;
	int diffY = endY - startY;
	int distance = (signed __int64)sqrt((double)(diffY * diffY + diffX * diffX));
	int v52 = (distance + (v35 / 2)) / v35;
	if (attacker->creatureIdx != CREATURE_MAGE && attacker->creatureIdx != CREATURE_ARCHMAGE) {
		int v37;
		int v43;
		if (v52 <= 1) {
			v43 = diffX;
			v37 = diffY;
		} else {
			v43 = diffX / (v52 - 1);
			v37 = diffY / (v52 - 1);
		}
		int v44 = startX;
		int v38 = startY;
		//from = (bitmap *)operator new(26);
		bitmap *from = nullptr;
		from = new bitmap(33, 2 * v27, 2 * v18);
		from->GrabBitmapCareful(gpWindowManager->screenBuffer, v44 - v27, v38 - v18);
		int v59 = v44;
		int v53 = v38;
		int x = 0;
		int y = 0;
		for (int i = 0; i < v52; ++i) {
			if (v59 - v27 < offsetX)
				offsetX = v59 - v27;
			if (offsetX < 0)
				offsetX = 0;
			if (v27 + v59 > v20)
				v20 = v27 + v59;
			if (v20 > 639)
				v20 = 639;
			if (v53 - v18 < offsetY)
				offsetY = v53 - v18;
			if (offsetY < 0)
				offsetY = 0;
			if (v18 + v53 > v15)
				v15 = v18 + v53;
			if (v15 > 442)
				v15 = 442;
			if (i) {
				from->DrawToBufferCareful(x, y);
			} else {
				if (giMinExtentX > offsetX)
					giMinExtentX = offsetX;
				if (v20 > giMaxExtentX)
					giMaxExtentX = v20;
				if (offsetY < giMinExtentY)
					giMinExtentY = offsetY;
				if (v15 > giMaxExtentY)
					giMaxExtentY = v15;
			}
			x = v44 - v27;
			if (v44 - v27 < 0)
				x = 0;
			if (x + (signed int)from->width > 640)
				x = 640 - from->width;
			y = v38 - v18;
			if (v38 - v18 < 0)
				y = 0;
			if (y + (signed int)from->height > 640)
				y = 640 - from->height;
			from->GrabBitmapCareful(gpWindowManager->screenBuffer, x, y);
			attacker->missileIcon->DrawToBuffer(v44, v38, spriteIdx, firingLeft);
			if (i) {
				DelayTil(&glTimers);
				gpWindowManager->UpdateScreenRegion(offsetX, offsetY, v20 - offsetX + 1, v15 - offsetY + 1);
			} else {
				gpWindowManager->UpdateScreenRegion(giMinExtentX, giMinExtentY, giMaxExtentX - giMinExtentX + 1, giMaxExtentY - giMinExtentY + 1);
			}

			glTimers = (signed __int64)((double)KBTickCount() + (double)v22 * gfCombatSpeedMod[giCombatSpeed]);
			v59 = v44;
			v53 = v38;
			v44 += v43;
			v38 += v37;
			offsetX = v44 - v27;
			v20 = v27 + v44;
			offsetY = v38 - v18;
			v15 = v18 + v38;
		}
		from->DrawToBuffer(x, y);
		gpWindowManager->UpdateScreenRegion(v59 - v27, v53 - v18, 2 * v27, 2 * v18);
		if (from)
			from->~bitmap();
	} else {
		gpWindowManager->UpdateScreenRegion(giMinExtentX, giMinExtentY, giMaxExtentX - giMinExtentX + 1, giMaxExtentY - giMinExtentY + 1);
		DelayMilli((signed __int64)(gfCombatSpeedMod[giCombatSpeed] * 115.0));
		gpCombatManager->DoBolt(1, startX, startY, endX, endY, 0, 0, 5, 4, 302, 0, 0, distance / 15 + 15, 1, 0, 10, 0);
	}
}

void army::SpecialAttack() {
	int damageDone = 0;
	int creaturesKilled = 0;
	this->field_125 = 0;
	army *target = &gpCombatManager->creatures[this->targetOwner][this->targetStackIdx];
	char targetColumnHex = target->occupiedHex % 13;
	char targetRowHex = target->occupiedHex / 13; // unused
	char sourceColumnHex = this->occupiedHex % 13;
	char sourceRowHex = this->occupiedHex / 13;
	int tmpFacingRight = this->facingRight;
	this->facingRight = targetColumnHex > sourceColumnHex || !(sourceRowHex & 1) && targetColumnHex == sourceColumnHex;

	if (this->facingRight != tmpFacingRight) {
		if (this->creature.creature_flags & TWO_HEXER) {
			OccupyHexes(this);
		}
		gpCombatManager->DrawFrame(1, 0, 0, 0, 75, 1, 1);
	}

	this->CheckLuck();
	gpSoundManager->MemorySample(this->combatSounds[3]);

	SpecialAttackGraphics(this, target);

	// Decrease the number of shots left
	if (!gpCombatManager->heroes[this->owningSide] || !gpCombatManager->heroes[this->owningSide]->HasArtifact(ARTIFACT_AMMO_CART))
		--this->creature.shots;

	int animIdx = -1;
	int a4 = -1;
	int a5 = -1;
		
	if (this->creatureIdx == CREATURE_LICH || this->creatureIdx == CREATURE_POWER_LICH) {
		int possibleTarget;
		gpCombatManager->ClearEffects();
		for (int i = 0; i < 7; ++i)	{
			if (i >= 6)
				possibleTarget = target->occupiedHex;
			else
				possibleTarget = target->GetAdjacentCellIndex(target->occupiedHex, i);
			if (possibleTarget != -1) {
				hexcell *targetHex = &gpCombatManager->combatGrid[possibleTarget];
				if (targetHex->unitOwner != -1) {
					army *targ = &gpCombatManager->creatures[targetHex->unitOwner][targetHex->stackIdx];
					if (!gArmyEffected[targ->owningSide][targ->stackIdx]) {
						if (target != targ || i == 6) {
							gArmyEffected[targ->owningSide][targ->stackIdx] = 1;
							this->DamageEnemy(targ, &damageDone, &creaturesKilled, 1, 0);
						}
					}
				}
			}
		}
		this->field_FA = 0;
		animIdx = 20;
		a4 = gpCombatManager->combatGrid[possibleTarget].centerX;
		a5 = gpCombatManager->combatGrid[possibleTarget].occupyingCreatureBottomY - 17;
		gpSoundManager->MemorySample(combatSounds[5]);
	} else if (CreatureHasAttribute(this->creatureIdx, PLASMA_BLAST)) {

		int cyberBehemothAttackMask[] = {
			-27,-26,-25,
			-14,-13,-12,-11,
			-2,-1,1,2,
			12,13,14,15,
			25,26,27
		};

		gpCombatManager->ClearEffects();

		int possibleTarget;
		for (int j = 0; j < 18; j++) {
			possibleTarget = target->occupiedHex + cyberBehemothAttackMask[j];
			if (possibleTarget >= 0 && possibleTarget < 116) {
				hexcell *targetHex = &gpCombatManager->combatGrid[possibleTarget];
				if (targetHex->unitOwner != -1) {
					army *targ = &gpCombatManager->creatures[targetHex->unitOwner][targetHex->stackIdx];
					if (!gArmyEffected[targ->owningSide][targ->stackIdx]) {
						if (target != targ) {
							gArmyEffected[targ->owningSide][targ->stackIdx] = 1;
							this->DamageEnemy(targ, &damageDone, &creaturesKilled, 1, 0);
						}
					}
				}
			}
		}
		possibleTarget = target->occupiedHex;
		if (possibleTarget != -1) {
			hexcell *targetHex = &gpCombatManager->combatGrid[possibleTarget];
			if (targetHex->unitOwner != -1) {
				army *targ = &gpCombatManager->creatures[targetHex->unitOwner][targetHex->stackIdx];
				if (!gArmyEffected[targ->owningSide][targ->stackIdx]) {
					gArmyEffected[targ->owningSide][targ->stackIdx] = 1;
					this->DamageEnemy(targ, &damageDone, &creaturesKilled, 1, 0);
				}
			}
		}
		this->field_FA = 0;
		animIdx = 32;
		a4 = gpCombatManager->combatGrid[target->occupiedHex].centerX;
		a5 = gpCombatManager->combatGrid[target->occupiedHex].occupyingCreatureBottomY - 17;
		//gpSoundManager->MemorySample(combatSounds[5]);
	} else {
		this->DamageEnemy(target, &damageDone, &creaturesKilled, 1, 0);
	}

	SpecialAttackBattleMessage(this, target, creaturesKilled, damageDone);

	if (this->creatureIdx == CREATURE_ARCHMAGE) {
		if (SRandom(1, 100) < 20) {
			if (target)	{
				if (target->SpellCastWorks(SPELL_ARCHMAGI_DISPEL))
					target->spellEnemyCreatureAbilityIsCasting = 102;
			}
		}
	}

	this->PowEffect(animIdx, 0, a4, a5);

	if (this->facingRight != tmpFacingRight) {
		if (this->creature.creature_flags & TWO_HEXER) {
			OccupyHexes(this);
		}
		this->facingRight = tmpFacingRight;
	}

	if (this->creatureIdx == CREATURE_ELF || this->creatureIdx == CREATURE_GRAND_ELF || this->creatureIdx == CREATURE_RANGER)
		ProcessSecondAttack(this, target);
	
	// Block mind spells
	if (this->effectStrengths[5] || this->effectStrengths[7]) {
		this->CancelSpellType(1);
		gpCombatManager->DrawFrame(1, 0, 0, 0, 75, 1, 1);
	}
}

void army::LoadResources() {
	if (!gbNoShowCombat) {
		int creatureID = this->creatureIdx;
		int formFileID = gpResourceManager->MakeId(cArmyFrameFileNames[creatureID], 1);
		gpResourceManager->PointToFile(formFileID);
		gpResourceManager->ReadBlock((signed char*)&this->frameInfo, 821u);
		ModifyFrameInfo(&this->frameInfo, (CREATURES)creatureID);
		this->field_B2 = this->frameInfo.stepTime;

		std::string shortName = this->creature.short_name;
		this->combatSounds[0] = gpResourceManager->GetSample(shortName + "move.82M");
		this->combatSounds[1] = gpResourceManager->GetSample(shortName + "attk.82M");
		this->combatSounds[2] = gpResourceManager->GetSample(shortName + "wnce.82M");
		this->combatSounds[4] = gpResourceManager->GetSample(shortName + "kill.82M");
		if (this->creature.creature_flags & SHOOTER) {
			this->combatSounds[3] = gpResourceManager->GetSample(shortName + "shot.82M");
		}
		switch (creatureID) {
			case CREATURE_VAMPIRE: case CREATURE_VAMPIRE_LORD: {
				this->combatSounds[5] = gpResourceManager->GetSample(shortName + "ext1.82M");
				this->combatSounds[6] = gpResourceManager->GetSample(shortName + "ext2.82M");
				break;
			}
			case CREATURE_LICH: case CREATURE_POWER_LICH: {
				this->combatSounds[5] = gpResourceManager->GetSample(shortName + "expl.82M");
			}
		}
		
		this->creatureIcon = gpResourceManager->GetIcon(cMonFilename[creatureID]);

		// Loading projectiles
		if (this->creature.creature_flags & SHOOTER) {
			switch (creatureID) {
				case CREATURE_HALFLING: {
					sprintf(gText, "halflmsl.icn");
					break;
				}
				case CREATURE_GIANT: case CREATURE_TITAN: {
					sprintf(gText, "titanmsl.icn");
					break;
				}
				case CREATURE_ARCHER: case CREATURE_RANGER: {
					sprintf(gText, "arch_msl.icn");
					break;
				}
				case CREATURE_LICH: case CREATURE_POWER_LICH: {
					sprintf(gText, "lich_msl.icn");
					break;
				}
				case CREATURE_ORC: case CREATURE_ORC_CHIEF: {
					sprintf(gText, "orc__msl.icn");
					break;
				}
				case CREATURE_DRUID: case CREATURE_GREATER_DRUID: {
					sprintf(gText, "druidmsl.icn");
					break;
				}
				case CREATURE_TROLL: case CREATURE_WAR_TROLL: {
					sprintf(gText, "trollmsl.icn");
					break;
				}
				case CREATURE_CYBER_BEHEMOTH: {
					sprintf(gText, "cbeh_msl.icn");
					break;
				}
				default: {
					sprintf(gText, "elf__msl.icn");
				}
			}
			this->missileIcon = gpResourceManager->GetIcon(gText);
		} else {
			this->combatSounds[3] = 0;
			this->missileIcon = 0;
		}

		for (int i = 0; i < 5; ++i) {
			if (this->combatSounds[i]) {
				this->combatSounds[i]->field_28 = 64;
				this->combatSounds[i]->codeThing = 3;
				this->combatSounds[i]->loopCount = 1;
			}
		}
	}
}

void army::PowEffect(int animIdx, int a3, int a4, int a5)
{
	int tmp1; // eax@35
	int tmp2; // eax@37
	int tmp3; // eax@39
	int tmp4; // eax@43
	int tmp5; // eax@45
	int tmp6; // eax@47
	int v11; // eax@70
	int v12; // eax@72
	int v13; // eax@74
	int v14; // eax@76
	int v15; // eax@79
	int v16; // eax@81
	signed int v17; // eax@83
	signed int v18; // eax@85
	IconEntry *animICNHeader; // [sp+20h] [bp-44h]@70
	int maxAnyCreatureAnimLen; // [sp+24h] [bp-40h]@45
	int maxAnyCreatureAnimLena; // [sp+24h] [bp-40h]@47
	signed int i; // [sp+28h] [bp-3Ch]@10
	signed int k; // [sp+28h] [bp-3Ch]@25
	signed int m; // [sp+28h] [bp-3Ch]@51
	signed int jj; // [sp+28h] [bp-3Ch]@88
	signed int mm; // [sp+28h] [bp-3Ch]@110
	signed int i1; // [sp+28h] [bp-3Ch]@161
	signed int i3; // [sp+28h] [bp-3Ch]@173
	signed int i5; // [sp+28h] [bp-3Ch]@210
	signed int i7; // [sp+28h] [bp-3Ch]@220
	signed int i9; // [sp+28h] [bp-3Ch]@229
	int fromAnimLen; // [sp+2Ch] [bp-38h]@1
	int maxOneWayAnimLen; // [sp+30h] [bp-34h]@1
	army *othstack; // [sp+38h] [bp-2Ch]@29
	army *thisd; // [sp+38h] [bp-2Ch]@114
	army *thisg; // [sp+38h] [bp-2Ch]@224
	signed int v41; // [sp+3Ch] [bp-28h]@171
	int j; // [sp+40h] [bp-24h]@12
	int l; // [sp+40h] [bp-24h]@27
	int n; // [sp+40h] [bp-24h]@53
	signed int ii; // [sp+40h] [bp-24h]@68
	int kk; // [sp+40h] [bp-24h]@90
	int nn; // [sp+40h] [bp-24h]@112
	int i2; // [sp+40h] [bp-24h]@163
	int i4; // [sp+40h] [bp-24h]@175
	int i6; // [sp+40h] [bp-24h]@212
	int i8; // [sp+40h] [bp-24h]@222
	int i10; // [sp+40h] [bp-24h]@231
	signed int doCreatureEffect; // [sp+44h] [bp-20h]@1
	int ll; // [sp+48h] [bp-1Ch]@108
	int oneWayAnimLen; // [sp+4Ch] [bp-18h]@1
	int maxToAnimLen; // [sp+50h] [bp-14h]@1
	int maxFromAnimLen; // [sp+54h] [bp-10h]@1
	int toAnimLen; // [sp+58h] [bp-Ch]@1
	signed int specialCaseOverlapWeirdness; // [sp+5Ch] [bp-8h]@1
	int creatureEffectNumFrames; // [sp+60h] [bp-4h]@1
	int maxAnimLen; // [sp+60h] [bp-4h]@49

	maxToAnimLen = 0;
	maxFromAnimLen = 0;
	maxOneWayAnimLen = 0;
	creatureEffectNumFrames = 0;
	toAnimLen = 0;
	fromAnimLen = 0;
	oneWayAnimLen = 0;
	doCreatureEffect = 0;
	specialCaseOverlapWeirdness = 1;
	if (this->creatureIdx == CREATURE_PALADIN || this->creatureIdx == CREATURE_CRUSADER)
		specialCaseOverlapWeirdness = 0;
	if (this->creatureIdx == CREATURE_DWARF || this->creatureIdx == CREATURE_BATTLE_DWARF)
		specialCaseOverlapWeirdness = 2;
	if (a4 == -1)
	{
		if (animIdx != -1)
		{
			for (i = 0; i < 2; ++i)
			{
				for (j = 0; gpCombatManager->numCreatures[i] > j; ++j)
				{
					if (gpCombatManager->creatures[i][j].probablyIsNeedDrawSpellEffect)
						doCreatureEffect = 1;
				}
			}
		}
	}
	else
	{
		doCreatureEffect = 1;
	}
	if (!gbNoShowCombat && animIdx != -1 && doCreatureEffect && animIdx != gCurLoadedSpellEffect)
	{
		gpResourceManager->Dispose((resource *)gCurLoadedSpellIcon);
		gCurLoadedSpellIcon = gpResourceManager->GetIcon(gCombatFxNames[animIdx]);
		gCurLoadedSpellEffect = animIdx;
	}
	if (doCreatureEffect)
		creatureEffectNumFrames = giNumPowFrames[gCurLoadedSpellEffect];
	for (k = 0; k < 2; ++k)
	{
		for (l = 0; gpCombatManager->numCreatures[k] > l; ++l)
		{
			othstack = &gpCombatManager->creatures[k][l];
			if (gpCombatManager->creatures[k][l].mightBeIsAttacking)
			{
				toAnimLen = othstack->frameInfo.animationLengths[this->mightBeAttackAnimIdx];
				fromAnimLen = othstack->frameInfo.animationLengths[this->mightBeAttackAnimIdx + 1] + 1;
			}
			else if (gpCombatManager->creatures[k][l].dead)
			{
				oneWayAnimLen = gpCombatManager->creatures[k][l].frameInfo.animationLengths[13];
			}
			else if (gpCombatManager->creatures[k][l].damageTakenDuringSomeTimePeriod)
			{
				oneWayAnimLen = gpCombatManager->creatures[k][l].frameInfo.animationLengths[15]
					+ gpCombatManager->creatures[k][l].frameInfo.animationLengths[14]
					+ 1;
			}
			tmp1 = maxToAnimLen;
			if (maxToAnimLen <= toAnimLen)
				tmp1 = toAnimLen;
			maxToAnimLen = tmp1;
			tmp2 = maxFromAnimLen;
			if (maxFromAnimLen <= fromAnimLen)
				tmp2 = fromAnimLen;
			maxFromAnimLen = tmp2;
			tmp3 = maxOneWayAnimLen;
			if (maxOneWayAnimLen <= oneWayAnimLen)
				tmp3 = oneWayAnimLen;
			maxOneWayAnimLen = tmp3;
		}
	}
	tmp4 = maxOneWayAnimLen + maxToAnimLen - specialCaseOverlapWeirdness;
	if (tmp4 <= maxFromAnimLen + maxToAnimLen)
		tmp4 = maxFromAnimLen + maxToAnimLen;
	maxAnyCreatureAnimLen = tmp4;
	tmp5 = maxOneWayAnimLen;
	if (maxOneWayAnimLen <= maxAnyCreatureAnimLen)
		tmp5 = maxAnyCreatureAnimLen;
	maxAnyCreatureAnimLena = tmp5;
	tmp6 = creatureEffectNumFrames;
	if (creatureEffectNumFrames <= maxAnyCreatureAnimLena)
		tmp6 = maxAnyCreatureAnimLena;
	maxAnimLen = tmp6;
	if (a3)
		gpCombatManager->ResetLimitCreature();
	for (m = 0; m < 2; ++m)
	{
		for (n = 0; gpCombatManager->numCreatures[m] > n; ++n)
		{
			gpCombatManager->creatures[m][n].animatingRangedAttack = gpCombatManager->creatures[m][n].animationType == ANIMATION_TYPE_RANGED_ATTACK_UPWARDS
				|| gpCombatManager->creatures[m][n].animationType == ANIMATION_TYPE_RANGED_ATTACK_FORWARDS
				|| gpCombatManager->creatures[m][n].animationType == ANIMATION_TYPE_RANGED_ATTACK_DOWNWARDS;
			if ((gpCombatManager->creatures[m][n].damageTakenDuringSomeTimePeriod
				|| gpCombatManager->creatures[m][n].mightBeIsAttacking
				|| gpCombatManager->creatures[m][n].animatingRangedAttack)
				&& !gpCombatManager->limitCreature[m][n])
				++gpCombatManager->limitCreature[m][n];
		}
	}
	gpCombatManager->DrawFrame(0, 1, 0, 1, 75, 1, 1);
	if (a4 != -1)
	{
		for (ii = 0; gCurLoadedSpellIcon->numSprites > ii; ++ii)
		{
			animICNHeader = &gCurLoadedSpellIcon->headersAndImageData[ii];
			v11 = a4 + animICNHeader->offsetX;
			if (v11 >= giMinExtentX)
				v11 = giMinExtentX;
			giMinExtentX = v11;
			v12 = a5 + animICNHeader->offsetY;
			if (v12 >= giMinExtentY)
				v12 = giMinExtentY;
			giMinExtentY = v12;
			v13 = a4 + animICNHeader->offsetX + animICNHeader->width - 1;
			if (v13 <= giMaxExtentX)
				v13 = giMaxExtentX;
			giMaxExtentX = v13;
			v14 = a5 + animICNHeader->height + animICNHeader->offsetY - 1;
			if (v14 <= giMaxExtentY)
				v14 = giMaxExtentY;
			giMaxExtentY = v14;
		}
		v15 = giMinExtentX;
		if (giMinExtentX <= 0)
			v15 = 0;
		giMinExtentX = v15;
		v16 = giMinExtentY;
		if (giMinExtentY <= 0)
			v16 = 0;
		giMinExtentY = v16;
		v17 = giMaxExtentX;
		if (giMaxExtentX >= 639)
			v17 = 639;
		giMaxExtentX = v17;
		v18 = giMaxExtentY;
		if (giMaxExtentY >= 442)
			v18 = 442;
		giMaxExtentY = v18;
	}
	for (jj = 0; jj < 2; ++jj)
	{
		for (kk = 0; gpCombatManager->numCreatures[jj] > kk; ++kk)
		{
			army *thisc = &gpCombatManager->creatures[jj][kk];
			gpCombatManager->creatures[jj][kk].field_3 = -1;
			thisc->field_4 = -1;
			thisc->effectStrengths[15] = 0;
			if (thisc->damageTakenDuringSomeTimePeriod || thisc->mightBeIsAttacking)
			{
				if (thisc->mightBeIsAttacking)
				{
					thisc->field_3 = this->mightBeAttackAnimIdx;
					thisc->field_4 = this->mightBeAttackAnimIdx + 1;
				}
				else if (thisc->dead)
				{
					thisc->field_3 = ANIMATION_TYPE_DYING;
				}
				else
				{
					thisc->field_3 = ANIMATION_TYPE_WINCE;
					thisc->field_4 = ANIMATION_TYPE_WINCE_RETURN;
				}
				if (thisc->field_3 == 13)
					thisc->field_5 = thisc->frameInfo.animationLengths[13];
				else
					thisc->field_5 = thisc->frameInfo.animationLengths[thisc->field_3]
					+ thisc->frameInfo.animationLengths[thisc->field_3 + 1];
				if (thisc->field_3 == thisc->animationType)
					--thisc->field_5;
				if (this->field_6 < 2)
					this->field_6 = 2;
			}
		}
	}
	for (ll = 0; maxAnimLen > ll; ++ll)
	{
		for (mm = 0; mm < 2; ++mm)
		{
			for (nn = 0; gpCombatManager->numCreatures[mm] > nn; ++nn)
			{
				thisd = &gpCombatManager->creatures[mm][nn];
				if (gpCombatManager->creatures[mm][nn].animatingRangedAttack)
				{
					if (gpCombatManager->creatures[mm][nn].animationType != ANIMATION_TYPE_RANGED_ATTACK_UPWARDS
						&& gpCombatManager->creatures[mm][nn].animationType != ANIMATION_TYPE_RANGED_ATTACK_FORWARDS
						&& gpCombatManager->creatures[mm][nn].animationType != ANIMATION_TYPE_RANGED_ATTACK_DOWNWARDS)
					{
						if (gpCombatManager->creatures[mm][nn].animationType != ANIMATION_TYPE_STANDING)
						{
							if (gpCombatManager->creatures[mm][nn].frameInfo.animationLengths[gpCombatManager->creatures[mm][nn].animationType] <= gpCombatManager->creatures[mm][nn].animationFrame + 1)
							{
								gpCombatManager->creatures[mm][nn].animationType = ANIMATION_TYPE_STANDING;
								thisd->animationFrame = 0;
							}
							else
							{
								++gpCombatManager->creatures[mm][nn].animationFrame;
							}
						}
					}
					else
					{
						++gpCombatManager->creatures[mm][nn].animationType;
						thisd->animationFrame = 0;
					}
				}
				if (thisd->field_3 != -1
					&& !thisd->effectStrengths[15]
					&& (thisd->mightBeIsAttacking
						|| thisd->field_5 >= maxAnimLen - ll - 1
						|| maxToAnimLen && maxToAnimLen - 1 <= ll
						|| !maxToAnimLen
						&& thisd->animationType != ANIMATION_TYPE_WINCE_RETURN
						&& (thisd->animationType != ANIMATION_TYPE_WINCE
							|| thisd->frameInfo.animationLengths[thisd->animationType] > thisd->animationFrame + 1)))
				{
					if (thisd->field_3 == thisd->animationType || thisd->field_4 == thisd->animationType)
					{
						if (thisd->frameInfo.animationLengths[thisd->animationType] <= thisd->animationFrame + 1)
						{
							if (thisd->field_4 == thisd->animationType || thisd->field_4 == -1)
							{
								if (thisd->animationType != ANIMATION_TYPE_STANDING && thisd->animationType != ANIMATION_TYPE_DYING)
								{
									thisd->animationType = ANIMATION_TYPE_STANDING;
									thisd->animationFrame = 0;
									thisd->effectStrengths[15] = 1;
								}
							}
							else
							{
								thisd->animationType = thisd->field_4;
								thisd->animationFrame = 0;
							}
						}
						else
						{
							++thisd->animationFrame;
						}
					}
					else
					{
						if (!gbNoShowCombat && thisd->field_3 == 14)
							gpSoundManager->MemorySample(gpCombatManager->creatures[mm][nn].combatSounds[2]);
						if (!gbNoShowCombat && thisd->field_3 == 13)
							gpSoundManager->MemorySample(gpCombatManager->creatures[mm][nn].combatSounds[4]);
						thisd->animationType = thisd->field_3;
						thisd->animationFrame = 0;
					}
				}
			}
		}
		glTimers = (signed __int64)((double)KBTickCount() + (double)120 * gfCombatSpeedMod[giCombatSpeed]);
		if (doCreatureEffect && giNumPowFrames[gCurLoadedSpellEffect] > ll)
			gCurSpellEffectFrame = ll;
		gpCombatManager->DrawFrame(0, 1, 0, 0, 75, 1, 1);
		if (a4 != -1 && giNumPowFrames[gCurLoadedSpellEffect] > ll)
			gCurLoadedSpellIcon->CombatClipDrawToBuffer(a4,	a5 + this->field_FA, gCurSpellEffectFrame, &this->effectAnimationBounds, 0, 0, 0, 0);
		gpWindowManager->UpdateScreenRegion(giMinExtentX, giMinExtentY, giMaxExtentX - giMinExtentX + 1, giMaxExtentY - giMinExtentY + 1);
	}
	for (i1 = 0; i1 < 2; ++i1)
	{
		for (i2 = 0; gpCombatManager->numCreatures[i1] > i2; ++i2)
		{
			army *thise = &gpCombatManager->creatures[i1][i2];
			if (gpCombatManager->creatures[i1][i2].damageTakenDuringSomeTimePeriod
				&& gpCombatManager->creatures[i1][i2].spellEnemyCreatureAbilityIsCasting != -1
				&& gpCombatManager->creatures[i1][i2].spellEnemyCreatureAbilityIsCasting != 101)
			{
				gpCombatManager->CastSpell(
					(Spell)gpCombatManager->creatures[i1][i2].spellEnemyCreatureAbilityIsCasting,
					gpCombatManager->creatures[i1][i2].occupiedHex,
					1,
					-1);
				thise->spellEnemyCreatureAbilityIsCasting = -1;
			}
		}
	}
	v41 = 1;
	while (v41)
	{
		v41 = 0;
		for (i3 = 0; i3 < 2; ++i3)
		{
			for (i4 = 0; gpCombatManager->numCreatures[i3] > i4; ++i4)
			{
				army *thisf = &gpCombatManager->creatures[i3][i4];
				if (gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_WINCE
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_MELEE_ATTACK_UPWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_MELEE_ATTACK_FORWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_MELEE_ATTACK_DOWNWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_TWO_HEX_ATTACK_UPWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_TWO_HEX_ATTACK_FORWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_TWO_HEX_ATTACK_DOWNWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_RANGED_ATTACK_UPWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_RANGED_ATTACK_FORWARDS
					&& gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_RANGED_ATTACK_DOWNWARDS)
				{
					if (gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_DYING
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_WINCE_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_MELEE_ATTACK_UPWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_MELEE_ATTACK_FORWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_MELEE_ATTACK_DOWNWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_TWO_HEX_ATTACK_UPWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_TWO_HEX_ATTACK_FORWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_TWO_HEX_ATTACK_DOWNWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_RANGED_ATTACK_UPWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_RANGED_ATTACK_FORWARDS_RETURN
						|| gpCombatManager->creatures[i3][i4].animationType == ANIMATION_TYPE_RANGED_ATTACK_DOWNWARDS_RETURN)
					{
						if (gpCombatManager->creatures[i3][i4].frameInfo.animationLengths[gpCombatManager->creatures[i3][i4].animationType] <= gpCombatManager->creatures[i3][i4].animationFrame + 1)
						{
							if (gpCombatManager->creatures[i3][i4].animationType != ANIMATION_TYPE_DYING)
							{
								gpCombatManager->creatures[i3][i4].animationType = ANIMATION_TYPE_STANDING;
								thisf->animationFrame = 0;
								v41 = 1;
							}
						}
						else
						{
							++gpCombatManager->creatures[i3][i4].animationFrame;
							v41 = 1;
						}
					}
				}
				else
				{
					++gpCombatManager->creatures[i3][i4].animationType;
					thisf->animationFrame = 0;
					v41 = 1;
				}
			}
		}
		if (v41)
		{
			glTimers = (signed __int64)((double)KBTickCount() + (double)120 * gfCombatSpeedMod[giCombatSpeed]);
			gpCombatManager->DrawFrame(1, 1, 0, 0, 75, 1, 1);
		}
	}
	if (a3)
		gpCombatManager->ResetLimitCreature();
	memset(gpCombatManager->shouldVanish, 0, 0x28u);
	gpCombatManager->anyStacksShouldVanish = 0;
	for (i5 = 0; i5 < 2; ++i5)
	{
		for (i6 = 0; gpCombatManager->numCreatures[i5] > i6; ++i6)
		{
			if (gpCombatManager->creatures[i5][i6].dead)
				gpCombatManager->creatures[i5][i6].ProcessDeath(0);
		}
	}
	if (gpCombatManager->anyStacksShouldVanish)
		gpCombatManager->MakeCreaturesVanish();
	for (i7 = 0; i7 < 2; ++i7)
	{
		for (i8 = 0; gpCombatManager->numCreatures[i7] > i8; ++i8)
		{
			thisg = &gpCombatManager->creatures[i7][i8];
			if (gpCombatManager->creatures[i7][i8].damageTakenDuringSomeTimePeriod
				&& gpCombatManager->creatures[i7][i8].spellEnemyCreatureAbilityIsCasting == 101)
			{
				gpCombatManager->CastSpell(
					(Spell)gpCombatManager->creatures[i7][i8].spellEnemyCreatureAbilityIsCasting,
					gpCombatManager->creatures[i7][i8].occupiedHex,
					1,
					-1);
				thisg->spellEnemyCreatureAbilityIsCasting = -1;
			}
			thisg->probablyIsNeedDrawSpellEffect = 0;
			thisg->damageTakenDuringSomeTimePeriod = 0;
			thisg->hasTakenLosses = 0;
			thisg->field_6 = 1;
			thisg->mightBeIsAttacking = 0;
			thisg->previousQuantity = -1;
		}
	}
	gpCombatManager->DrawFrame(1, 0, 0, 0, 75, 1, 1);
	for (i9 = 0; i9 < 2; ++i9)
	{
		for (i10 = 0; gpCombatManager->numCreatures[i9] > i10; ++i10)
			army::WaitSample(2);
	}
}