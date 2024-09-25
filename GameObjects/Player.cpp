#include "Player.hpp"

Entity * createPlayer() {
    Entity * playerEntity = entityNew();
    playerEntity->update = update;
    playerEntity->model = gf3d_model_load("models/dino.model");

    PlayerData * playerData = (PlayerData*) malloc(sizeof(PlayerData));
    if (!playerData) {
        slog("Failed to allocate memory for player data.");
        return NULL;
    }


    memset(playerData, 0, sizeof(PlayerData));
    playerEntity->data = playerData;
    playerData->playerRotation = gfc_vector3d(M_PI, 0, 0);
    

    return playerEntity;
}

PlayerData * getPlayerData(Entity * self){
    return (PlayerData*) self->data;
}

void assignCamera(Entity * self, Camera * newCam) {
    PlayerData * playerData = getPlayerData(self);
    playerData->camera = newCam;
    gf3d_camera_look_at(self->position, NULL);
}

void update(Entity * self) {
    _playerControls(self);
}   

void _playerControls(Entity * self) {

    PlayerData * playerData = getPlayerData(self);
    
    // Get the keybaord state
    const Uint8 * keys;
    keys = SDL_GetKeyboardState(NULL);

    GFC_Vector2D velocity = gfc_vector2d(0, 0);
    velocity.x = PLAYER_SPEED * keys[SDL_SCANCODE_RIGHT] - PLAYER_SPEED * keys[SDL_SCANCODE_LEFT];
    velocity.y = PLAYER_SPEED * keys[SDL_SCANCODE_UP] - PLAYER_SPEED * keys[SDL_SCANCODE_DOWN];
    velocity.x *= -1;
    velocity.y *= -1;
    velocity = gfc_vector2d_rotate(velocity, playerData->playerRotation.z);

    self->position.x += velocity.x;
    self->position.y += velocity.y;
    
    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);

    if (playerData->camera) {
        // Adds mouse motion to the player rotation
        playerData->playerRotation.z -= mouseX * HORIZONTAL_MOUSE_SENSITIVITY;

        // Gets the camera offset, rotates it around the player's Z rotation, then adds it to the player's position
        GFC_Vector3D baseCamPosition = CAMERA_OFFSET;
        GFC_Vector3D * newCamPosition = &baseCamPosition;
        gfc_vector3d_rotate_about_z(newCamPosition, playerData->playerRotation.z);
        *newCamPosition = gfc_vector3d_added(*newCamPosition, self->position);
        gf3d_camera_set_position(*newCamPosition);

        // Takes the base camera rotation, and adds together its Z rotation and the player's Z rotation
        GFC_Vector3D baseCamRotation = gfc_vector3d_added(CAMERA_ROTATION, gfc_vector3d(0, 0, playerData->playerRotation.z));
        
        gf3d_camera_set_rotation(baseCamRotation);

        // Move the player model in the direction facing
        self->rotation.z += (playerData->playerRotation.z - self->rotation.z) * 0.1;
    }
}