#include "Powerup.h"
#include "simple_logger.h"
#include "Player.h"

const float POWERUP_DURATION = 16;
const float POWERUP_LIFETIME = 20.0;

Entity *powerupEntityNew(Entity *player) {
    Entity *newPowerup = entityNew();
    if (!newPowerup) {
        slog("Not enough memory to create Powerup entity.");
        return NULL;
    }

    PowerupData *powerupData = (PowerupData*)malloc(sizeof(PowerupData));
    if (!powerupData) {
        free(newPowerup);
        slog("Not enough memory to create Powerup data.");
        return NULL;
    }
    memset(powerupData, 0, sizeof(powerupData));
    powerupData->type = gfc_random_int(SPEED_BOOST + 1);
    powerupData->player = player;
    newPowerup->data = powerupData;

    newPowerup->update = powerupUpdate;
    newPowerup->think = powerupThink;
    newPowerup->draw = powerupDraw;
    newPowerup->free = powerupFree;

    return newPowerup;
}

void *powerupThink(struct Entity_S *self, float delta) {
    PowerupData *powerupData = (PowerupData*)self->data;
    powerupData->powerupLifetime += delta;
    if (powerupData->powerupLifetime > POWERUP_LIFETIME) {
        free(powerupData);
        _entityFree(self);
        return;
    }
    if (fabsf(self->position.x - powerupData->player->position.x) <= 4 && fabsf(self->position.y - powerupData->player->position.y) <= 4) {
        PlayerData* playerData = (PlayerData*) powerupData->player->data;
        playerData->powerups[powerupData->type] = 1;
        playerData->powerupTimers[powerupData->type] = POWERUP_DURATION;
        free(powerupData);
        _entityFree(self);

    }
}

void *powerupUpdate(struct Entity_S *self, float delta) {
    self->rotation.z += 1 * delta;
}

void *powerupDraw(Entity * self, LightUBO *lights) {
    if (!self) return;
    
    GFC_Matrix4 matrix;
    gfc_matrix4_from_vectors(
        matrix,
        entityGlobalPosition(self),
        entityGlobalRotation(self),
        entityGlobalScale(self)
    );

    PowerupData *powerupData = (PowerupData*) self->data;
    GFC_Color color = { 0 };
    if (powerupData->type == INFINITE_AMMO) {
        color = GFC_COLOR_RED;
    } else if (powerupData->type == NO_RECOIL)
    {
        color = GFC_COLOR_ORANGE;
    } else if (powerupData->type == QUARTER_RELOAD_TIME)
    {
        color = GFC_COLOR_DARKCYAN;
    } else if (powerupData->type == INVINCIBLE)
    {
        color = GFC_COLOR_GREEN;
    } else if (powerupData->type == SPEED_BOOST)
    {
        color = GFC_COLOR_BLUE;
    }

    gf3d_model_draw(
        self->model,
        matrix,
        color,
        NULL,
        0
    );
}

void* powerupFree(struct Entity_S* self) {
    if (self->data) {
        free(self->data);
    }
    _entityFree(self);
}