#include "Reticle.hpp"

static Reticle reticle = {0};

void reticleLoad(const char *reticleFile) {
    if (reticle.actor) gf2d_actor_free(reticle.actor);
    reticle.actor = gf2d_actor_load(reticleFile);
    reticle.hidden = false;
    if (!reticle.actor) slog("Failed to load reticle actor");   
}

void reticleDraw() {
    if (reticle.hidden) return;
    GFC_Vector2D reticlePosition = gfc_vector2d(gf3d_vgraphics_get_resolution().x / 2.0, gf3d_vgraphics_get_resolution().y / 2.0);
    reticlePosition.x -= reticle.actor->frameWidth;
    reticlePosition.y -= reticle.actor->frameHeight;

    gf2d_actor_draw(
        reticle.actor,
        reticle.frame,
        reticlePosition,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

}