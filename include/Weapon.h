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

/**
* @brief Returns an entity hit by a raycast. Does not work if the entity's origin is more than 2 meters away from the edge of its collision
*/
Entity* shotCollided(GFC_Edge3D raycast, GFC_Box boundingBox);
/**
 * @brief Pistol's fire function.
 */
void pistolFire(Weapon * weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition);

#endif