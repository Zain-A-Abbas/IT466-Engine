#ifndef __WEAPON__
#define __WEAPON__

#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"
#include "stdbool.h"
#include <string.h>
#include "Entity.h"



typedef struct Weapon_S
{
    const char        *name;
    int         cartridgeSize;
    float       reloadSpeed;
    int         maxReserveAmmo;
    int         currentAmmo;
    int         reserveAmmo;
    int         damage;
    // Behavior
    void (*shoot)   (
        struct Weapon_S * weapon,
        GFC_Vector3D playerPosition,
        GFC_Vector3D playerRotation,
        GFC_Vector3D cameraPosition
        ); // The weapon's fire function
} Weapon;

/**
 * @brief Loads the weapon and returns the struct.
 */
Weapon loadWeapon(const char *weaponFile);

Entity* shotCollided(GFC_Edge3D raycast);
/**
 * @brief Pistol's fire function.
 */
void pistolFire(Weapon * weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition);

#endif