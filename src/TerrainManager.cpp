#include "simple_logger.h"
#include "gfc_matrix.h"
#include "TerrainManager.hpp"

const int TERRAIN_LAYER = 1;

Entity * terrainEntityNew() {
	Entity* terrainEntity = entityNew();
	if (terrainEntity == NULL) {
		slog("Terrain Entity could not be created");
		return NULL;
	}
	terrainEntity->collisionLayer = TERRAIN_LAYER;
	return terrainEntity;
};