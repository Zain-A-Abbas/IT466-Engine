#include "simple_logger.h"
#include "Interactable.h"

const Uint8 INTERACTABLE_LAYERS = 0b00001010;



Entity* interactableNew(InteractableType type) {

	Entity* newInteractable = entityNew();
	if (!newInteractable) {
		slog("Could not create interactable.");
		return NULL;
	}

	newInteractable->type = INTERACTABLE;
	newInteractable->model = gf3d_model_load("models/interactables/interactable.model");

	InteractableData* interactableData = (InteractableData*)malloc(sizeof(InteractableData));
	if (!interactableData) {
		slog("Could not create InteractableData");
		free(newInteractable);
		return NULL;
	}

	memset(interactableData, 0, sizeof(interactableData));
	newInteractable->data = interactableData;

	interactableData->canInteract = true;
	interactableData->interact = baseInteract;
	return newInteractable;
}

void _interact(Entity* entity, InteractableData* interactData) {
	if (!interactData) return;
	if (interactData->interact && interactData->canInteract) {
		interactData->interact(entity, interactData);
		return;
	} else {
		slog("Interactable does not have interact function pointer set");
	}
}

void baseInteract(Entity* entity, InteractableData* interactData) {
	entity->rotation.z += M_PI / 16;
}