#include "Player.hpp"

const float PLAYER_SPEED = 0.25;
const float HORIZONTAL_MOUSE_SENSITIVITY = 0.025;
const int MAX_RELATIVE_MOUSE_X = 10;
const int HIGHEST_X_DEGREES = 15;
const int LOWEST_X_DEGREES = -20;

const float MAX_SLOPE_DEGREES = M_PI / 4;
const float PLAYER_GRAVITY_RAYCAST_HEIGHT = 6;
const float GRAVITY = -0.098;

float previousFloorAngle = 0.0;
float snapZ = 0.0;
bool snapToSnapZ = false;

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

    playerData->playerVelocity.x = movementVelocity.x;
    playerData->playerVelocity.y = movementVelocity.y;

    int mouseX, mouseY;
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    if (mouseX > 10) {
        mouseX = 10;
    } else if (mouseX < -10) {
        mouseX = -10;
    }
    playerData->playerRotation.z -= mouseX * HORIZONTAL_MOUSE_SENSITIVITY;
    playerData->playerRotation.x += mouseY * HORIZONTAL_MOUSE_SENSITIVITY;
    if (playerData->playerRotation.x > HIGHEST_X_DEGREES * GFC_DEGTORAD) playerData->playerRotation.x = HIGHEST_X_DEGREES * GFC_DEGTORAD;
    if (playerData->playerRotation.x < LOWEST_X_DEGREES * GFC_DEGTORAD) playerData->playerRotation.x = LOWEST_X_DEGREES * GFC_DEGTORAD;
    

    GFC_Vector3D floorNormal;
    GFC_Vector3D contact = { 0 };
    if (isOnFloor(self, &floorNormal, &contact)) {
        float floorAngle = M_PI / 2 - asinf(floorNormal.z);
        if (previousFloorAngle > floorAngle) {
            snapZ = contact.z + PLAYER_GRAVITY_RAYCAST_HEIGHT;
            snapToSnapZ = true;
        }
        previousFloorAngle = floorAngle;

        GFC_Vector3D horizontalDirection = gfc_vector3d(playerData->playerVelocity.x, playerData->playerVelocity.y, 0);
        gfc_vector3d_normalize(&horizontalDirection);
        float dotProduct = gfc_vector3d_dot_product(floorNormal, horizontalDirection);
        float horizontalMagnitude = sqrtf(pow(playerData->playerVelocity.x, 2) + pow(playerData->playerVelocity.y, 2));

        // Get the dot product as if parallel to the slope
        GFC_Vector3D opposite = gfc_vector3d(-floorNormal.x, -floorNormal.y, 0);
        gfc_vector3d_normalize(&opposite);
        float parallelDotProduct = gfc_vector3d_dot_product(floorNormal, opposite);

        float floorRatio = tanf(floorAngle) * fabs(dotProduct / parallelDotProduct);

        if (dotProduct < 0) {
            playerData->playerVelocity.z = horizontalMagnitude * floorRatio;
        } else if (dotProduct > 0) {
            playerData->playerVelocity.z = -horizontalMagnitude * floorRatio;
        }
        else {
            playerData->playerVelocity.z = 0;
        }

    } else {
        snapToSnapZ = false;
        playerData->playerVelocity.z += GRAVITY;
    }



    if (gfc_input_command_pressed("continue"))  {
        GFC_Vector3D raycastPosition = self->position;
        GFC_Vector3D raycastAdd = gfc_vector3d(0, -32, 0);
        gfc_vector3d_rotate_about_z(&raycastAdd, playerData->playerRotation.z); 
        raycastPosition = gfc_vector3d_added(raycastPosition, raycastAdd);
        
        GFC_Edge3D raycast = gfc_edge3d_from_vectors(self->position, raycastPosition);
        //slog("Current Position: %f, %f, %f", self->position.x, self->position.y, self->position.z);
        //slog("Raycast End: %f, %f, %f", raycastPosition.x, raycastPosition.y, raycastPosition.z);

        float groundAngle = 0.0;
        playerData->raycastTest = raycast;
        playerData->raycastColor = gfc_color(1, 0, 0, 1);

        int collided = shotCollided(self, raycast);
        if (collided) {
            playerData->raycastColor = gfc_color(0, 0, 1, 1);
        }

    }
}

void _playerUpdate(Entity *self) {

    PlayerData * playerData = getPlayerData(self);
    self->position.x += playerData->playerVelocity.x;
    self->position.y += playerData->playerVelocity.y;
    self->position.z += playerData->playerVelocity.z;
    if (snapToSnapZ) {
        slog("Pos z: %f", self->position.z);
        slog("Snap z: %f", snapZ);
        self->position.z = snapZ;
        snapToSnapZ = false;
    }

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

int isOnFloor(Entity* self, GFC_Vector3D * floorNormal, GFC_Vector3D * contact) {
    GFC_Triangle3D t = {0};
    GFC_Vector3D gravityRaycastDir = gfc_vector3d(0, 0, -PLAYER_GRAVITY_RAYCAST_HEIGHT);
    GFC_Edge3D gravityRaycast = gfc_edge3d_from_vectors(self->position, gfc_vector3d_added(self->position, gravityRaycastDir));
    for (int i = 0; i < entityManager.entityMax; i++) {
        // Get ground entitiesentities
        if (entityManager.entityList[i]._in_use) {
            if (entityManager.entityList[i].collisionLayer != 1) {
                continue;
            }

            // Found terrain
            if (entityRaycastTest(&entityManager.entityList[i], gravityRaycast, contact, &t)) {
                *floorNormal = gfc_trigfc_angle_get_normal(t);
                return true;
            }
        }
    }
    return false;
    
}

int shotCollided(Entity *self, GFC_Edge3D raycast) {
    GFC_Triangle3D t = {0};
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

GFC_Vector3D getCameraPosition(Entity *self) {
    PlayerData * playerData = getPlayerData(self);
    GFC_Vector3D newCamPosition = CAMERA_OFFSET;
    gfc_vector3d_rotate_about_x(&newCamPosition, playerData->playerRotation.x); 
    gfc_vector3d_rotate_about_z(&newCamPosition, playerData->playerRotation.z);
    newCamPosition = gfc_vector3d_added(newCamPosition, self->position);
    return newCamPosition;
}