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
        damage
    };

    slog("Weapon successfully created!");
    slog(newWeapon.name);

    return newWeapon;



}