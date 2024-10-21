#ifndef __ENEMY__
#define __ENEMY__

#include "Entity.hpp"

typedef struct EnemyData_S {
    GFC_Vector3D    enemyVelocity;
    GFC_Vector3D    enemyRotation;
} EnemyData;


/**
* @brief Creates an enemy entity and assigns it the proper collision layer.
*/
Entity * enemyEntityNew();

void enemyThink(Entity* self);
void enemyUpdate(Entity* self);

#endif