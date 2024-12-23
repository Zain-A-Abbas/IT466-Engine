#ifndef __UI__
#define __UI__

#include "simple_logger.h"
#include "gf2d_actor.h"
#include "Player.h"
#include "Weapon.h"

typedef struct {
	PlayerData*		playerData;
	Actor*			bgActor;
	Actor*			currentWeaponActor;
	Actor*			hpBorderActor;
	Actor*			hpBarActor;
	Actor			*moneyActor;
} UIData;

void initializeUI();

void actorLoad(Actor** actorptr, const char* actorFile);

void assignPlayer(PlayerData* playerData);

void playerSwitchWeapon(Weapon *weapon);

void drawUI();

void drawPlayerUI(GFC_Vector2D resolution);

void drawEnemyKills();

void drawLevelClear();

#endif