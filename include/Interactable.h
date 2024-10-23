#include "Entity.h"

typedef enum {
	NO_INTERACT,
	SPINNING_BOX
} InteractableType;

typedef struct InteractableData_S {
	InteractableType	interactableType;
	Uint8				canInteract;
	void (*interact)	(Entity *entity, struct InteractableData_S *interactData);
} InteractableData;

Entity* interactableNew(InteractableType type);


void _interact(Entity* entity, InteractableData* interactData);

void baseInteract(Entity* entity, InteractableData* interactData);