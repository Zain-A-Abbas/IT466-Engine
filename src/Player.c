#include "Player.h"
#include "gf2d_mouse.h"
#include "Interactable.h"

const float PLAYER_SPEED = 10;
const float HORIZONTAL_MOUSE_SENSITIVITY = 1.28;
const float VERTICAL_MOUSE_SENSITIVITY = 0.96;
const int MAX_RELATIVE_MOUSE_X = 10;
const int HIGHEST_X_DEGREES = 48;
const int LOWEST_X_DEGREES = -20;

const float MAX_SLOPE_DEGREES = M_PI / 4;
const float PLAYER_GRAVITY_RAYCAST_HEIGHT = 6.5;
const float GRAVITY = -1;
const float HORIZONTAL_COLLISION_RADIUS = 4;

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
    playerData->playerVelocity = gfc_vector3d(0, 0, 0);
    playerData->playerRotation = gfc_vector3d(M_PI, 0, 0);
    
    playerData->playerWeapons = (Weapon*) malloc(10 * sizeof(Weapon));
    memset(playerData->playerWeapons, 0, 10 * sizeof(Weapon));
    playerData->playerWeapons[0] = loadWeapon("GameData/WeaponData/Pistol.json");

    actualCameraOffset = BASE_CAMERA_OFFSET;
    zoomCameraOffset = gfc_vector3d_multiply(BASE_CAMERA_OFFSET, gfc_vector3d(0.95, 0.75, 0.75));

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
    
    if (gfc_input_command_pressed("interact")) {
        interact(self);
    }

    // Horizontal movement
    GFC_Vector2D inputVector = gfc_vector2d(
        gfc_input_command_down("walkright") - gfc_input_command_down("walkleft"),
        gfc_input_command_down("walkforward") - gfc_input_command_down("walkback")
    );
    inputVector = gfc_vector2d_rotate(inputVector, playerData->playerRotation.z);

    GFC_Vector3D movementVelocity = gfc_vector3d(inputVector.x, inputVector.y, 0);

    movementVelocity.x *= PLAYER_SPEED * delta;
    movementVelocity.y *= PLAYER_SPEED * delta;
    movementVelocity.x *= -1;
    movementVelocity.y *= -1;

    playerData->playerVelocity.x = movementVelocity.x;
    playerData->playerVelocity.y = movementVelocity.y;

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

    playerData->playerRotation.z -= mouseX * HORIZONTAL_MOUSE_SENSITIVITY * delta;
    playerData->playerRotation.x += mouseY * VERTICAL_MOUSE_SENSITIVITY * delta;
    if (playerData->playerRotation.x > HIGHEST_X_DEGREES * GFC_DEGTORAD) playerData->playerRotation.x = HIGHEST_X_DEGREES * GFC_DEGTORAD;
    if (playerData->playerRotation.x < LOWEST_X_DEGREES * GFC_DEGTORAD) playerData->playerRotation.x = LOWEST_X_DEGREES * GFC_DEGTORAD;
    
    // Gravity/slope movement
    GFC_Vector3D floorNormal;
    GFC_Vector3D contact = { 0 };
    if (isOnFloor(self, &floorNormal, &contact)) {
        float floorAngle = M_PI / 2 - asinf(floorNormal.z);
        // Make it snap to a certain height when traversing on ground with a lower angle than on the previous frame to account for slightly dipping into the floor
        if (previousFloorAngle > floorAngle) {
            snapZ = contact.z + PLAYER_GRAVITY_RAYCAST_HEIGHT;
            snapToSnapZ = true;
        }

        previousFloorAngle = floorAngle;

        GFC_Vector3D horizontalDirection = gfc_vector3d(playerData->playerVelocity.x, playerData->playerVelocity.y, 0);
        gfc_vector3d_normalize(&horizontalDirection);
        // Used to determine if moving up or down
        float dotProduct = gfc_vector3d_dot_product(floorNormal, horizontalDirection);
        float horizontalMagnitude = sqrtf(pow(playerData->playerVelocity.x, 2) + pow(playerData->playerVelocity.y, 2));

        // Get the dot product as if parallel to the slope, to slow down vertical movement in case the player is not moving parallel to it
        GFC_Vector3D opposite = gfc_vector3d(-floorNormal.x, -floorNormal.y, 0);
        gfc_vector3d_normalize(&opposite);
        float parallelDotProduct = gfc_vector3d_dot_product(floorNormal, opposite);

        // Uses tanf rather than sinf as the horizontal speed should not adjust with the angle of the slope
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
        playerData->playerVelocity.z += GRAVITY * delta;
    }

    // Zoom
    aimZoom = gf2d_mouse_button_held(2);

    // Attacking
    if (gf2d_mouse_button_held(0)) {
        playerData->playerWeapons[0].shoot(&playerData->playerWeapons[0], self->position, playerData->playerRotation, getCameraPosition(self));
        /*
        GFC_Vector3D raycastPosition = self->position;
        GFC_Vector3D raycastAdd = gfc_vector3d(0, -32, 0);
        gfc_vector3d_rotate_about_z(&raycastAdd, playerData->playerRotation.z);
        raycastPosition = gfc_vector3d_added(raycastPosition, raycastAdd);

        GFC_Edge3D raycast = gfc_edge3d_from_vectors(self->position, raycastPosition);
        //slog("Current Position: %f, %f, %f", self->position.x, self->position.y, self->position.z);
        //slog("Raycast End: %f, %f, %f", raycastPosition.x, raycastPosition.y, raycastPosition.z);


        playerData->raycastTest = raycast;
        playerData->raycastColor = gfc_color(1, 0, 0, 1);

        int collided = shotCollided(self, raycast);
        if (collided) {
            slog("Hit");
            playerData->raycastColor = gfc_color(0, 0, 1, 1);
        }

    }*/
    }
}

void _playerUpdate(Entity * self, float delta) {

    // Movement
    PlayerData * playerData = getPlayerData(self);

    GFC_Vector3D velocity = playerData->playerVelocity;

    for (int i = 0; i < entityManager.entityMax; i++) {
        // Filter out inactive entities, non-interactables, and interactables out of range
        Entity* currEntity = &entityManager.entityList[i];
        if (!currEntity->_in_use) {
            continue;
        }

        if (!isOnLayer(currEntity, 3)) {
            continue;
        }


        if (!gfc_vector3d_distance_between_less_than(self->position, currEntity->position, HORIZONTAL_COLLISION_RADIUS * 2)) {
            continue;
        }

        GFC_Vector3D playerDir = velocity;
        gfc_vector3d_normalize(&playerDir);

        GFC_Vector3D collisionSpherePosition;
        GFC_Vector3D nextPosition = gfc_vector3d_multiply(velocity, gfc_vector3d(8, 8, 8));
        nextPosition = gfc_vector3d_added(self->position, velocity);

        GFC_Edge3D movementRaycast = gfc_edge3d_from_vectors(self->position, nextPosition);
        GFC_Vector3D contact;
        GFC_Triangle3D t;
        if (entityRaycastTest(currEntity, movementRaycast, &contact, &t, NULL)) {
            GFC_Vector3D normal = gfc_trigfc_angle_get_normal(t);
            
            slog("Normal of the triangle: %f, %f, %f", normal.x, normal.y, normal.z);
            slog("Player Direction: %f, %f, %f", playerDir.x, playerDir.y, playerDir.z);
        }
            

    }

    self->position.x += playerData->playerVelocity.x;
    self->position.y += playerData->playerVelocity.y;
    self->position.z += playerData->playerVelocity.z;
    if (snapToSnapZ) {
        //slog("Pos z: %f", self->position.z);
        //slog("Snap z: %f", snapZ);
        self->position.z = snapZ;
        snapToSnapZ = false;
    }

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
        GFC_Vector3D baseCamRotation = gfc_vector3d_added(CAMERA_ROTATION, gfc_vector3d(0, 0, playerData->playerRotation.z));
        baseCamRotation = gfc_vector3d_added(baseCamRotation, gfc_vector3d(playerData->playerRotation.x, 0, 0));
        gf3d_camera_set_rotation(baseCamRotation);

        // Move the player model in the direction facing
        float targetRotation = self->rotation.z + (playerData->playerRotation.z - self->rotation.z) * 0.1;
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

int isOnFloor(Entity* self, GFC_Vector3D * floorNormal, GFC_Vector3D * contact) {
    GFC_Triangle3D t = {0};
    GFC_Vector3D gravityRaycastDir = gfc_vector3d(0, 0, -PLAYER_GRAVITY_RAYCAST_HEIGHT);
    GFC_Edge3D gravityRaycast = gfc_edge3d_from_vectors(self->position, gfc_vector3d_added(self->position, gravityRaycastDir));
    for (int i = 0; i < entityManager.entityMax; i++) {
        // Get ground entitiesentities
        if (!entityManager.entityList[i]._in_use) {
            continue;
        }
        if (!isOnLayer(&entityManager.entityList[i], 1)) {
            continue;
        }

        // Found terrain
        if (entityRaycastTest(&entityManager.entityList[i], gravityRaycast, contact, &t, NULL)) {
            /*slog("%f, %f, %f", t.a.x, t.a.y, t.a.z);
            slog("%f, %f, %f", t.b.x, t.b.y, t.b.z);
            slog("%f, %f, %f", t.c.x, t.c.y, t.c.z);*/
            *floorNormal = gfc_trigfc_angle_get_normal(t);
            return true;
            }
    }
    return false;
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
    GFC_Vector3D newCamPosition = actualCameraOffset;
    gfc_vector3d_rotate_about_x(&newCamPosition, playerData->playerRotation.x); 
    gfc_vector3d_rotate_about_z(&newCamPosition, playerData->playerRotation.z);
    newCamPosition = gfc_vector3d_added(newCamPosition, self->position);
    return newCamPosition;
}