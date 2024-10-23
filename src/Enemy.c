#include "simple_logger.h"
#include "gfc_matrix.h"
#include "Enemy.h"

const Uint8 ENEMY_LAYERS = 0b00000100;

Entity * enemyEntityNew() {
	Entity* enemyEntity = entityNew();
	if (enemyEntity == NULL) {
		slog("Enemy Entity could not be created");
		return NULL;
	}
	enemyEntity->think = enemyThink;
	enemyEntity->update = enemyUpdate;
	enemyEntity->type = ENEMY;
	enemyEntity->model = gf3d_model_load("models/enemies/enemy1/enemy1.model");

	EnemyData* enemyData = (EnemyData*)malloc(sizeof(EnemyData));
    if (!enemyData) {
        slog("Failed to allocate memory for enemy data.");
		free(enemyEntity);
        return NULL;
    }
	enemyEntity->data = enemyData;
	memset(enemyData, 0, sizeof(enemyData));
	enemyData->enemyVelocity = gfc_vector3d(0, 0, 0);
	enemyData->enemyRotation = gfc_vector3d(M_PI, 0, 0);

	enemyEntity->collisionLayer = ENEMY_LAYERS;
	return enemyEntity;
};

void enemyThink(Entity* self) {
	return;
}


void enemyUpdate(Entity* self) {
	return;
}