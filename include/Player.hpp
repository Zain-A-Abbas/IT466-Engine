#ifndef __PLAYER__
#define __PLAYER__

#include "simple_logger.h"
#include "Entity.hpp"
#include "gfc_input.h"
#include "gf3d_camera.h"
#include "Weapon.hpp"
#include "gf3d_draw.h"

#define CAMERA_OFFSET gfc_vector3d(-4, 20, 4)
#define FAR_CAMERA_OFFSET gfc_vector3d(-8, 32, 4)
#define CAMERA_ROTATION gfc_vector3d(M_PI, 0, M_PI)

typedef struct PlayerData_S {
    Camera          *camera;    // Pointer to camera
    GFC_Vector3D    playerRotation;
    GFC_Vector3D    playerVelocity;
    Weapon          *playerWeapons;
    GFC_Edge3D      raycastTest;
    GFC_Color       raycastColor;
} PlayerData;


/**
 * @brief Creates an entity and assigns the player functions to it.
 */
Entity * createPlayer();

/**
 * @brief Assigns a camera to the player.
 */
void assignCamera(Entity * self, Camera * newCam);

void think(Entity * self, float delta);
void update(Entity * self, float delta);
void playerFree(Entity * self);

void _playerControls(Entity * self, float delta);
void _playerUpdate(Entity * self, float delta);

/**
* @brief Gets the player data.
*/
PlayerData* getPlayerData(Entity* self);

/**
 * @brief Returns where the camera position should be
 */
GFC_Vector3D getCameraPosition(Entity *self);

/**
* @brief Checks if player is on ground or not.
* @param The player entity
* @param The angle of the ground, if the player is on the floor
*/
int isOnFloor(Entity* self, float* floorAngle);


#endif