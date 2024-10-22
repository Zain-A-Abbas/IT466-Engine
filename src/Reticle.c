#include "Reticle.h"

static Reticle reticle = {0};

void reticleLoad(const char *reticleFile) {
    if (reticle.actor) gf2d_actor_free(reticle.actor);
    reticle.actor = gf2d_actor_load(reticleFile);

    reticle.hidden = false;
    if (!reticle.actor) slog("Failed to load reticle actor");   
}

void reticleDraw() {
    if (reticle.hidden) return;
    GFC_Vector2D resolution = gf3d_vgraphics_get_resolution();
    GFC_Vector2D reticlePosition = gfc_vector2d(resolution.x / 2.0, resolution.y / 2.0);
    //slog("actorCenter: %f,%f",reticle.actor->center.x, reticle.actor->center.y);

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