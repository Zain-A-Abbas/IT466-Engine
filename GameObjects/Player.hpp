#ifndef __PLAYER__
#define __PLAYER__

#include "simple_logger.h"
#include "Entity.hpp"
#include "gf3d_camera.h"

const float PLAYER_SPEED = 0.125;
const float HORIZONTAL_MOUSE_SENSITIVITY = 0.05;
#define CAMERA_OFFSET gfc_vector3d(-4, 20, 4)
#define CAMERA_ROTATION gfc_vector3d(M_PI, 0, M_PI)

typedef struct PlayerData_S {
    Camera          *camera;    // Pointer to camera
    GFC_Vector3D  playerRotation;
} PlayerData;


/**
 * @brief Creates an entity and assigns the player functions to it.
 */
Entity * createPlayer();

/**
 * @brief Assigns a camera to the player.
 */
void assignCamera(Entity * self, Camera * newCam);

void update(Entity * self);

void _playerControls(Entity * self); 

#endif