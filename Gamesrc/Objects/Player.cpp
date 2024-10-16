#include "Player.hpp"

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
    playerData->playerVelocity = gfc_vector3d(0, 0, 0);
    playerData->playerRotation = gfc_vector3d(M_PI, 0, 0);
    
    playerData->raycastTest = gfc_edge3d_from_vectors(gfc_vector3d(0, 0, 0), gfc_vector3d(0, 0, 0));
    playerData->raycastColor = gfc_color(1.0, 0.0, 0.0, 1.0);

    playerData->playerWeapons = (Weapon*) malloc(10 * sizeof(Weapon));
    memset(playerData->playerWeapons, 0, 10 * sizeof(Weapon));
    playerData->playerWeapons[0] = loadWeapon("GameData/WeaponData/Pistol.json");

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

void playerFree(Entity * self) {
    PlayerData * playerData = getPlayerData(self);
    if (playerData){
        free(playerData);
    }
}

void think(Entity * self) {
    _playerControls(self);
}


void update(Entity * self) {
    _playerUpdate(self);
}   

void _playerControls(Entity * self) {
    PlayerData * playerData = getPlayerData(self);

    GFC_Vector2D inputVector = gfc_vector2d(
        gfc_input_command_down("walkright") - gfc_input_command_down("walkleft"),
        gfc_input_command_down("walkforward") - gfc_input_command_down("walkback")
        );
    inputVector = gfc_vector2d_rotate(inputVector, playerData->playerRotation.z);

    GFC_Vector3D movementVelocity = gfc_vector3d(inputVector.x, inputVector.y, 0);

    movementVelocity.x *= PLAYER_SPEED;
    movementVelocity.y *= PLAYER_SPEED;
    movementVelocity.x *= -1;
    movementVelocity.y *= -1;
    
    playerData->playerVelocity = movementVelocity;

    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    playerData->playerRotation.z -= mouseX * HORIZONTAL_MOUSE_SENSITIVITY;
    playerData->playerRotation.x += mouseY * HORIZONTAL_MOUSE_SENSITIVITY;
    if (playerData->playerRotation.x > HIGHEST_X_DEGREES * GFC_DEGTORAD) playerData->playerRotation.x = HIGHEST_X_DEGREES * GFC_DEGTORAD;
    if (playerData->playerRotation.x < LOWEST_X_DEGREES * GFC_DEGTORAD) playerData->playerRotation.x = LOWEST_X_DEGREES * GFC_DEGTORAD;
    

    if (gfc_input_command_pressed("continue"))  {
        GFC_Vector3D raycastPosition = self->position;
        GFC_Vector3D raycastAdd = gfc_vector3d(0, -32, 0);
        gfc_vector3d_rotate_about_z(&raycastAdd, playerData->playerRotation.z); 
        raycastPosition = gfc_vector3d_added(raycastPosition, raycastAdd);
        
        GFC_Edge3D raycast = gfc_edge3d_from_vectors(self->position, raycastPosition);
        slog("Current Position: %f, %f, %f", self->position.x, self->position.y, self->position.z);
        slog("Raycast End: %f, %f, %f", raycastPosition.x, raycastPosition.y, raycastPosition.z);

        playerData->raycastTest = raycast;
            playerData->raycastColor = gfc_color(1, 0, 0, 1);

        int collided = shot_collided(self, raycast);
        if (collided) {
            slog("%d", collided);
            playerData->raycastColor = gfc_color(0, 0, 1, 1);
        }

    }
}

void _playerUpdate(Entity *self) {

    PlayerData * playerData = getPlayerData(self);
    self->position.x += playerData->playerVelocity.x;
    self->position.y += playerData->playerVelocity.y;

    if (playerData->camera) {

        // Gets the camera offset, rotates it around the player's Z and X rotations, then adds it to the player's position
        gf3d_camera_set_position(getCameraPosition(self));

        // Takes the base camera rotation, and adds together its Z rotation and the player's Z rotation
        GFC_Vector3D baseCamRotation = gfc_vector3d_added(CAMERA_ROTATION, gfc_vector3d(0, 0, playerData->playerRotation.z));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, gfc_vector3d(playerData->playerRotation.x, 0, 0));
        gf3d_camera_set_rotation(baseCamRotation);

        // Move the player model in the direction facing
        float targetRotation = self->rotation.z + (playerData->playerRotation.z - self->rotation.z) * 0.1;
        self->rotation.z = targetRotation;
    }
}

int shot_collided(Entity *self, GFC_Edge3D raycast) {
    for (int i = 0; i < entityManager.entityMax; i++) {
    // Get non-player entities
        if (entityManager.entityList[i]._in_use) {
            if (&entityManager.entityList[i] == self) continue;
            slog("Entity position: %f, %f, %f", entityManager.entityList[i].position.x, entityManager.entityList[i].position.y, entityManager.entityList[i].position.z);
            Model *entityModel = entityManager.entityList[i].model;

            // Get meshes
            for (int i = 0; i < gfc_list_get_count(entityModel->mesh_list); i++) {
                Mesh *mesh = (Mesh*) gfc_list_get_nth(entityModel->mesh_list, i);
                if (mesh) {
                    // Get primitives
                    for (int i = 0; i < gfc_list_get_count(mesh->primitives); i++) {
                        MeshPrimitive *primitive = (MeshPrimitive*) gfc_list_get_nth(mesh->primitives, i);
                        if (primitive) {
                            if (primitive->objData) {
                                if (gf3d_obj_line_test(primitive->objData, raycast, NULL)) {
                                    slog("Collided");
                                    return true;
                                }
                            }
                        }
                    }

                }
            }
        }
    }  
    return false;
}

GFC_Vector3D getCameraPosition(Entity *self) {
    PlayerData * playerData = getPlayerData(self);
    GFC_Vector3D newCamPosition = CAMERA_OFFSET;
    gfc_vector3d_rotate_about_x(&newCamPosition, playerData->playerRotation.x); 
    gfc_vector3d_rotate_about_z(&newCamPosition, playerData->playerRotation.z);
    newCamPosition = gfc_vector3d_added(newCamPosition, self->position);
    return newCamPosition;
}