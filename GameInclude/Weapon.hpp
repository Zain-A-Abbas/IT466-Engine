#ifndef __WEAPON__
#define __WEAPON__

#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"
#include "stdbool.h"

typedef void (*shoot)(void);

const char* WEAPON_LIST[] = {"Pistol"};

typedef struct Weapon
{
    const char        *name;
    int         cartridgeSize;
    float       reloadSpeed;
    int         maxReserveAmmo;
    int         currentAmmo;
    int         reserveAmmo;
    int         damage;
    shoot       *shoot;
} Weapon;

/**
 * @brief Loads the weapon and returns the struct.
 */
Weapon loadWeapon(char *weaponFile);

/**
 * @brief Pistol's fire function.
 */
void pistolFire();

#endif