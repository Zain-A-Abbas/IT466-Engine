#ifndef __RETICLE__
#define __RETICLE__

#include "simple_logger.h"
#include "gf2d_actor.h"
#include "gf3d_vgraphics.h"
#include <stdbool.h>    

typedef struct {
    Actor   *actor;
    float   frame;
    bool    hidden;
} Reticle;

/**
 * @brief Loads the reticle sprite
 * @param reticleFile The file used to load the reticle
 */
void reticleLoad(const char *reticleFile);

/**
 * @brief Make the reticle visible
 */
void reticleShow();

/**
 * @brief Make the reticle invisible
 */
void reticleHide();

/**
 * @brief Check if the reticle is visible or not
 */
int reticleIsHidden();

/**
 * @brief Called once per frame to draw the reticle
 */
void reticleDraw();

void reticleAnimate();

#endif