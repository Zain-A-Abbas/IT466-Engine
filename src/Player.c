#include "Player.h"
#include "gf2d_mouse.h"
#include "Interactable.h"
#include "Character3D.h"

const float PLAYER_SPEED = 10;
const float HORIZONTAL_MOUSE_SENSITIVITY = 1.28;
const float VERTICAL_MOUSE_SENSITIVITY = 0.96;
const int MAX_RELATIVE_MOUSE_X = 10;
const int HIGHEST_X_DEGREES = 48;
const int LOWEST_X_DEGREES = -20;

const float INTERACT_DISTANCE = 8;

const GFC_Vector3D BASE_CAMERA_OFFSET = { -4, 20, 4 };
GFC_Vector3D actualCameraOffset;
GFC_Vector3D zoomCameraOffset;

float previousFloorAngle = 0.0;
float snapZ = 0.0;
bool snapToSnapZ = false;

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
    
    playerData->playerWeapons = (Weapon*) malloc(10 * sizeof(Weapon));
    memset(playerData->playerWeapons, 0, 10 * sizeof(Weapon));
    playerData->playerWeapons[0] = loadWeapon("GameData/WeaponData/Pistol.json");

    actualCameraOffset = BASE_CAMERA_OFFSET;
    zoomCameraOffset = gfc_vector3d_multiply(BASE_CAMERA_OFFSET, gfc_vector3d(0.95, 0.75, 0.75));

    playerData->character3dData = newCharacter3dData();
    playerData->character3dData->gravityRaycastHeight = 6.5;

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
        interact(self);
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
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
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

    // Zoom
    aimZoom = gf2d_mouse_button_held(2);

    // Attacking
    if (gf2d_mouse_button_held(0)) {
        playerData->playerWeapons[0].shoot(&playerData->playerWeapons[0], self->position, character3dData->rotation, getCameraPosition(self));

    }
}

void _playerUpdate(Entity * self, float delta) {

    // Movement
    PlayerData * playerData = getPlayerData(self);
    Character3DData* character3dData = playerData->character3dData;


    horizontalWallSlide(self, character3dData, delta);
    verticalVectorMovement(self, character3dData, delta);

    moveAndSlide(self, character3dData);

    // Zoom
    GFC_Vector3D cameraMove;
    if (aimZoom) {
        cameraMove = gfc_vector3d_subbed(actualCameraOffset, zoomCameraOffset);
    } else {
        cameraMove = gfc_vector3d_subbed(actualCameraOffset, BASE_CAMERA_OFFSET);
    }
    cameraMove = gfc_vector3d_multiply(cameraMove, gfc_vector3d(delta * 10, delta * 10, delta * 10));
    actualCameraOffset = gfc_vector3d_subbed(actualCameraOffset, cameraMove);

    // Mouse update
    if (playerData->camera) {

        // Gets the camera offset, rotates it around the player's Z and X rotations, then adds it to the player's position
        gf3d_camera_set_position(getCameraPosition(self));

        // Takes the base camera rotation, and adds together its Z rotation and the player's Z rotation
        GFC_Vector3D baseCamRotation = gfc_vector3d_added(CAMERA_ROTATION, gfc_vector3d(0, 0, character3dData->rotation.z));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, gfc_vector3d(character3dData->rotation.x, 0, 0));
        gf3d_camera_set_rotation(baseCamRotation);

        // Move the player model in the direction facing
        float targetRotation = self->rotation.z + (character3dData->rotation.z - self->rotation.z) * 0.1;
        self->rotation.z = targetRotation;
    }

    // Floor raycast
    if (playerData != NULL) {
        GFC_Triangle3D t = { 0 };
        GFC_Vector3D gravityRaycastDir = gfc_vector3d(0, 0, -6.5);
        GFC_Edge3D gravityRaycast = gfc_edge3d_from_vectors(self->position, gfc_vector3d_added(self->position, gravityRaycastDir));
        gf3d_draw_edge_3d(
            gravityRaycast,
            gfc_vector3d(0, 0, 0),
            gfc_vector3d(0, 0, 0),
            gfc_vector3d(1, 1, 1),
            0.5,
            gfc_color(1.0, 1.0, 0.0, 1.0)
        );
    }

}



void interact(Entity* self) {
    GFC_Vector3D interactPoint = gfc_vector3d(0, -INTERACT_DISTANCE, 0);
    gfc_vector3d_rotate_about_z(&interactPoint, self->rotation.z);
    interactPoint = gfc_vector3d_added(interactPoint, self->position);

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-interactables, and interactables out of range
        Entity* currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }

        if (currEntity->type != INTERACTABLE) {
            continue;
        }

        if (!gfc_vector3d_distance_between_less_than(interactPoint, currEntity->position, INTERACT_DISTANCE)) {
            continue;
        }

        InteractableData* interactData = (InteractableData*)currEntity->data;

        interactData->interact(currEntity, interactData);
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