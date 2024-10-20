#include "Weapon.hpp"

static const char* WEAPON_LIST[] = { "Pistol" };

Weapon loadWeapon(const char *weaponFile) {
    SJson *weaponJson;
    weaponJson = sj_load(weaponFile);
    if (!weaponJson) {
        slog("No weapon file");
    }

    SJson *SJweaponName = sj_object_get_value(weaponJson, "Name");
    const char *weaponName = sj_get_string_value(SJweaponName);

    bool weaponExists = false;

    for (int i = 0; i < (sizeof(WEAPON_LIST) / sizeof(WEAPON_LIST[0])); i++) {
        const char* weaponName = WEAPON_LIST[i];
        if (strcmp(weaponName, (const char*) weaponName) == 0) {
            weaponExists = true;
        }
    }
    
    if (!weaponExists) {
        slog("Weapon does not exist.");
    }

    SJson *SJcartridgeSize = sj_object_get_value(weaponJson, "CartridgeSize");
    SJson *SJreloadSpeed = sj_object_get_value(weaponJson, "ReloadSpeed");
    SJson *SJmaxReserveAmmo = sj_object_get_value(weaponJson, "MaxReserveAmmo");
    SJson *SJdamage = sj_object_get_value(weaponJson, "Damage");
    
    int cartridgeSize = sj_get_integer_value(SJcartridgeSize, &cartridgeSize);
    float reloadSpeed = sj_get_float_value(SJreloadSpeed, &reloadSpeed);
    int maxReserveAmmo = sj_get_integer_value(SJmaxReserveAmmo, &maxReserveAmmo);
    int damage = sj_get_integer_value(SJdamage, &damage);
    

    Weapon newWeapon = {
        weaponName,
        cartridgeSize,
        reloadSpeed,
        maxReserveAmmo,
        0,
        0,
        damage,
    };

    newWeapon.shoot = pistolFire;

    slog("Weapon successfully created!");
    slog(newWeapon.name);

    return newWeapon;
}

int shotCollided(Entity* self, GFC_Edge3D raycast) {
    GFC_Triangle3D t = { 0 };
    for (int i = 0; i < entityManager.entityMax; i++) {
        // Get non-player entities
        if (entityManager.entityList[i]._in_use) {
            //slog("In the house like carpet");
            if (&entityManager.entityList[i] == self) continue;
            //slog("Entity position: %f, %f, %f", entityManager.entityList[i].position.x, entityManager.entityList[i].position.y, entityManager.entityList[i].position.z);
            if (entityRaycastTest(&entityManager.entityList[i], raycast, NULL, &t)) {
                return true;
            }
        }
    }
    return false;
}

void pistolFire(Weapon* weapon, GFC_Vector3D playerPosition, GFC_Vector3D playerRotation, GFC_Vector3D cameraPosition) {
    GFC_Vector3D raycastStart = cameraPosition;
    GFC_Vector3D raycastAdd = gfc_vector3d(0, -32, 0);
    gfc_vector3d_rotate_about_x(&raycastAdd, playerRotation.x);
    gfc_vector3d_rotate_about_z(&raycastAdd, playerRotation.z);
    raycastAdd = gfc_vector3d_added(raycastStart, raycastAdd);
    GFC_Edge3D raycast = gfc_edge3d_from_vectors(raycastStart, raycastAdd);
    slog("Raycast start: %f, %f, %f", raycastStart.x, raycastStart.y, raycastStart.z);
    slog("Raycast end: %f, %f, %f", raycastAdd.x, raycastAdd.y, raycastAdd.z);

}