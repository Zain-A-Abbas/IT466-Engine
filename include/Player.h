#ifndef __PLAYER__
#define __PLAYER__

#include "simple_logger.h"
#include "Entity.h"
#include "gfc_input.h"
#include "gf3d_camera.h"
#include "Weapon.h"
#include "gf3d_draw.h"
#include "Character3D.h"

#define FAR_CAMERA_OFFSET gfc_vector3d(-8, 32, 4)
#define CAMERA_ROTATION gfc_vector3d(M_PI, 0, M_PI)

typedef struct PlayerData_S {
    Camera              *camera;    // Pointer to camera
    Weapon              *playerWeapons;
    GFC_Edge3D          raycastTest;
    GFC_Color           raycastColor;
    Character3DData     *character3dData;
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
* @brief Interacts with all objects in a radius in front of the player
*/
void interact(Entity* self);

#endif