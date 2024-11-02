#include "Player.h"
#include "gf2d_mouse.h"
#include "Character3D.h"
#include "UI.h"
#include "TypesExtra.h"

const float PLAYER_SPEED = 16;
const float HORIZONTAL_MOUSE_SENSITIVITY = 2.0;
const float VERTICAL_MOUSE_SENSITIVITY = 1.6;
const int MAX_RELATIVE_MOUSE_X = 10;
const int HIGHEST_X_DEGREES = 40;
const int LOWEST_X_DEGREES = -20;

const float INTERACT_DISTANCE = 8;

const GFC_Vector3D BASE_CAMERA_OFFSET = { -4, 20, 10.5 };
GFC_Vector3D actualCameraOffset;
GFC_Vector3D zoomCameraOffset;

float previousFloorAngle = 0.0;
float snapZ = 0.0;
bool snapToSnapZ = false;

bool reloading = false;
bool aimZoom = false;

Entity * createPlayer() {
    Entity * playerEntity = entityNew();
    playerEntity->think = think;
    playerEntity->update = update;
    playerEntity->model = gf3d_model_load("models/dino.model");

    PlayerData * playerData = (PlayerData*) malloc(sizeof(PlayerData));
    if (!playerData) {
        slog("Failed to allocate memory for player data.");
        return NULL;
    }

    memset(playerData, 0, sizeof(PlayerData));
    playerEntity->data = playerData;
    playerData->playerWeapons = gfc_list_new();

    
    playerData->weaponsUnlocked = 0;
    giveWeapon(playerEntity, playerData, "GameData/WeaponData/Pistol.json");

    playerData->cameraTrauma = gfc_vector3d(0, 0, 0);
    playerData->cameraTraumaDecay = gfc_vector3d(0, 0, 0);
    actualCameraOffset = BASE_CAMERA_OFFSET;
    zoomCameraOffset = gfc_vector3d_multiply(BASE_CAMERA_OFFSET, gfc_vector3d(0.95, 0.85, 0.85));

    playerData->character3dData = newCharacter3dData();
    playerData->character3dData->gravityRaycastHeight = 1;

    playerData->attackCooldown = 0;

    GFC_Box boxTest = { 0 };
    playerData->boundingBoxTest = boxTest;

    // UI setup
    assignPlayer(playerData);

    return playerEntity;
}

PlayerData * getPlayerData(Entity * self){

    if (self == NULL || self->data == NULL) {
        return NULL;
    }
    return (PlayerData*) self->data;
}

void assignCamera(Entity * self, Camera * newCam) {
    PlayerData * playerData = getPlayerData(self);
    playerData->camera = newCam;
    gf3d_camera_look_at(self->position, NULL);
}

void playerFree(Entity * self) {
    PlayerData * playerData = getPlayerData(self);
    if (playerData){
        free(playerData);
    }
}

void think(Entity * self, float delta) {
    _playerControls(self, delta);
}


void update(Entity * self, float delta) {
    _playerUpdate(self, delta);
}   

void _playerControls(Entity * self, float delta) {
    PlayerData * playerData = getPlayerData(self);
    Character3DData* character3dData = playerData->character3dData;
    
    if (gfc_input_command_pressed("interact")) {
        interact(self, playerData);
    }

    // Horizontal movement
    GFC_Vector2D inputVector = gfc_vector2d(
        gfc_input_command_down("walkright") - gfc_input_command_down("walkleft"),
        gfc_input_command_down("walkforward") - gfc_input_command_down("walkback")
    );
    inputVector = gfc_vector2d_rotate(inputVector, character3dData->rotation.z);

    GFC_Vector3D movementVelocity = gfc_vector3d(inputVector.x, inputVector.y, 0);

    movementVelocity.x *= PLAYER_SPEED * delta;
    movementVelocity.y *= PLAYER_SPEED * delta;
    movementVelocity.x *= -1;
    movementVelocity.y *= -1;

    playerData->character3dData->velocity.x = movementVelocity.x;
    playerData->character3dData->velocity.y = movementVelocity.y;

    // Mouse rotation
    int xRel, yRel;
    float mouseX, mouseY;
    SDL_GetRelativeMouseState(&xRel, &yRel);
    mouseX = xRel * 0.1;
    mouseY = yRel * 0.1;
    if (mouseX > 10) {
        mouseX = 10;
    } else if (mouseX < -10) {
        mouseX = -10;
    }

    if (aimZoom) {
        mouseX *= 0.7;
        mouseY *= 0.7;
    }

    playerData->character3dData->rotation.z -= mouseX * HORIZONTAL_MOUSE_SENSITIVITY * delta;
    playerData->character3dData->rotation.x += mouseY * VERTICAL_MOUSE_SENSITIVITY * delta;
    if (playerData->character3dData->rotation.x > HIGHEST_X_DEGREES * GFC_DEGTORAD) playerData->character3dData->rotation.x = HIGHEST_X_DEGREES * GFC_DEGTORAD;
    if (playerData->character3dData->rotation.x < LOWEST_X_DEGREES * GFC_DEGTORAD) playerData->character3dData->rotation.x = LOWEST_X_DEGREES * GFC_DEGTORAD;
    // Camera shake

    // Zoom
    aimZoom = gf2d_mouse_button_held(2);

    // Attacking
    if (gf2d_mouse_button_held(0)) {
        attack(self, playerData, character3dData);
    }

    if (gfc_input_command_pressed("reload")) {
        reload(self, playerData);
    }
}

void _playerUpdate(Entity * self, float delta) {


    // Movement
    PlayerData * playerData = getPlayerData(self);
    
    Character3DData* character3dData = playerData->character3dData;


    moveAndSlide(self, character3dData);
    horizontalWallSlide(self, character3dData, delta);
    verticalVectorMovement(self, character3dData, delta);


    playerData->attackCooldown = fMoveTowards(playerData->attackCooldown, 0, delta);

    // Zoom
    GFC_Vector3D cameraMove;
    if (aimZoom) {
        cameraMove = gfc_vector3d_subbed(actualCameraOffset, zoomCameraOffset);
    } else {
        cameraMove = gfc_vector3d_subbed(actualCameraOffset, BASE_CAMERA_OFFSET);
    }
    cameraMove = gfc_vector3d_multiply(cameraMove, gfc_vector3d(delta * 16, delta * 16, delta * 16));
    actualCameraOffset = gfc_vector3d_subbed(actualCameraOffset, cameraMove);

    // Camera trauma reduction
    playerData->cameraTrauma.x = fMoveTowards(playerData->cameraTrauma.x, 0, playerData->cameraTraumaDecay.x * delta);
    playerData->cameraTrauma.y = fMoveTowards(playerData->cameraTrauma.y, 0, playerData->cameraTraumaDecay.y * delta);
    playerData->cameraTrauma.z = fMoveTowards(playerData->cameraTrauma.z, 0, playerData->cameraTraumaDecay.z * delta);

    if (playerData->cameraTrauma.x == 0) {
        playerData->cameraTraumaDecay.x = 0;
    }
    if (playerData->cameraTrauma.y == 0) {
        playerData->cameraTraumaDecay.y = 0;
    }
    if (playerData->cameraTrauma.z == 0) {
        playerData->cameraTraumaDecay.z = 0;
    }

    // Mouse update
    if (playerData->camera) {

        // Gets the camera offset, rotates it around the player's Z and X rotations, then adds it to the player's position
        gf3d_camera_set_position(getCameraPosition(self));

        // Takes the base camera rotation, and adds together its Z rotation and the player's Z rotation
        GFC_Vector3D baseCamRotation = gfc_vector3d_added(CAMERA_ROTATION, gfc_vector3d(0, 0, character3dData->rotation.z));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, gfc_vector3d(character3dData->rotation.x, 0, 0));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, playerData->cameraTrauma);
        gf3d_camera_set_rotation(baseCamRotation);

        // Move the player model in the direction facing
        float targetRotation = self->rotation.z + (character3dData->rotation.z - self->rotation.z) * 0.1;
        self->rotation.z = targetRotation;
    }

    interactScan(self);

}

void interactScan(Entity* self) {
    GFC_Vector3D interactPoint = gfc_vector3d(0, -INTERACT_DISTANCE, 0);
    GFC_Vector3D interactPosition;
    Interactable* interactable = NULL;
    Interactable* tempInteractable = NULL;
    Entity* currEntity;
    PlayerData* playerData = getPlayerData(self);
    gfc_vector3d_rotate_about_z(&interactPoint, self->rotation.z);
    interactPoint = gfc_vector3d_added(interactPoint, entityGlobalPosition(self));

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-interactables, and interactables out of range
        currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }

        if (currEntity->type != INTERACTABLE) {
            continue;
        }

        tempInteractable = (Interactable*)currEntity->data;
        interactPosition = gfc_vector3d_added(entityGlobalPosition(currEntity), tempInteractable->interactOrigin);
        if (!gfc_vector3d_distance_between_less_than(interactPoint, interactPosition, INTERACT_DISTANCE)) {
            continue;
        }
        interactable = tempInteractable;

    }

    playerData->currentInteractable = interactable;

}

void interact(Entity * self, PlayerData* playerData) {
    if (playerData->currentInteractable) {
        playerData->currentInteractable->interact(self, playerData->currentInteractable->interactEntity, playerData->currentInteractable);
    }
}

GFC_Vector3D getCameraPosition(Entity *self) {
    PlayerData * playerData = getPlayerData(self);
    Character3DData* character3dData = playerData->character3dData;
    GFC_Vector3D newCamPosition = actualCameraOffset;
    gfc_vector3d_rotate_about_x(&newCamPosition, character3dData->rotation.x);
    gfc_vector3d_rotate_about_z(&newCamPosition, character3dData->rotation.z);


    newCamPosition = gfc_vector3d_added(newCamPosition, self->position);
    return newCamPosition;
}

void addCameraTrauma(PlayerData* playerData, GFC_Vector3D trauma, GFC_Vector3D traumaDecay) {
    playerData->cameraTrauma = gfc_vector3d_added(playerData->cameraTrauma, trauma);
    playerData->cameraTraumaDecay = traumaDecay;
}

void attack(Entity * self, PlayerData * playerData, Character3DData * character3dData) {
    if (playerData->attackCooldown != 0) {
        return;
    }

    Weapon* weaponData = gfc_list_get_nth(playerData->playerWeapons, playerData->currentWeapon);
    if (weaponData->currentAmmo <= 0) {
        return;
    }

    weaponData->currentAmmo -= 1;

    addCameraTrauma(playerData, gfc_vector3d(-0.08, 0, 0.04), gfc_vector3d(1.0, 0, 2.0));
    if (gf2d_mouse_button_held(2)) {
        playerData->cameraTrauma = gfc_vector3d_multiply(playerData->cameraTrauma, gfc_vector3d(0.5, 0.5, 0.5));
    }
    playerData->attackCooldown = 0.2;
    weaponData->shoot(self, weaponData, self->position, character3dData->rotation, getCameraPosition(self));
}

void reload(Entity * self, PlayerData * playerData) {
    if (reloading) {
        return;
    }
    reloading = true;
    Weapon* weaponData = gfc_list_get_nth(playerData->playerWeapons, playerData->currentWeapon);


    int reloadAmount = min(weaponData->cartridgeSize - weaponData->currentAmmo, min(weaponData->cartridgeSize, weaponData->reserveAmmo));
    
    if (reloadAmount == 0) {
        reloading = false;
        return;
    }

    weaponData->reserveAmmo -= reloadAmount;
    weaponData->currentAmmo += reloadAmount;
    
    reloading = false;

}

void giveWeapon(Entity* self, PlayerData* playerData, const char *weapon) {
    int newWeaponIndex = playerData->weaponsUnlocked;
    playerData->weaponsUnlocked += 1;
    gfc_list_append(playerData->playerWeapons, loadWeapon(weapon));
    setWeapon(playerData, newWeaponIndex);
}

void setWeapon(PlayerData* playerData, int weaponIndex) {
    playerData->currentWeapon = weaponIndex;
    playerSwitchWeapon((Weapon*)gfc_list_get_nth(playerData->playerWeapons, weaponIndex));
}