#ifndef __WEAPON__
#define __WEAPON__

#include "simple_logger.h"
#include "simple_json.h"

typedef void (*shoot)(void);

typedef struct Weapon
{
    char        *name;
    int         cartridgeSize;
    float       reloadSpeed;
    int         maxReserveAmmo;
    int         currentAmmo;
    int         reserveAmmo;
    int         damage;
};

/**
 * 
 */
Weapon createWeapon() {

}

#endif