#include "simple_logger.h"
#include "gfc_matrix.h"
#include "TerrainManager.hpp"

const Uint8 TERRAIN_LAYERS = 0b00000010;

Entity * terrainEntityNew() {
	Entity* terrainEntity = entityNew();
	if (terrainEntity == NULL) {
		slog("Terrain Entity could not be created");
		return NULL;
	}
	terrainEntity->type = TERRAIN;
	terrainEntity->collisionLayer = TERRAIN_LAYERS;
	return terrainEntity;
};