#ifndef __ENTITY_SYSTEM__
#define __ENTITY_SYSTEM__

#include "gfc_types.h"
#include "gfc_text.h"
#include "gfc_vector.h"
#include "gf3d_model.h"


typedef struct Entity_S {
    Uint8           _in_use; // Active memory
    GFC_TextLine    name; // Entity Name
    GFC_Vector3D    position; 
    GFC_Vector3D    rotation;
    GFC_Vector3D    scale;
    Model           *model; // POINTER to model
    // Behavior
    void (*think)   (struct Entity_S *self); // Called every frame on the entity
    void (*update)   (struct Entity_S *self); // Called every frame for entity state update
    void (*draw)    (struct Entity_S *self); // Custom draw code
    void (*free)    (struct Entity_S *self); // Cleans up custom data
    void            *data; // Custom entity data

} Entity;

typedef struct {
    Entity* entityList;
    Uint32 entityMax;
} EntityManager;

/**
 * @brief Allocates blank entity for use
 * @return NULL on failure (no memory) | POINTER to entity
 */
Entity * entityNew();

void entitySystemClose();

/**
 * @brief Initialize entity manager
 * @param maxEnts Man number of entities
 */
void entitySystemInit(Uint32 maxEnts);

/**
 * @brief Draw all the entities to screen
 */
void entityDrawAll();


/**
 * @brief Update all entities
 */
void entityUpdateAll();

/**
 * @brief Have all entities think
 */
void entityThinkAll();

/**
 * @brief Return memory of previously allocated entity back to pool
 * @param self POINTER to entity
 */
void _entityFree(Entity *self);

#endif