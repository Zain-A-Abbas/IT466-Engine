#include "Door.h"
#include "simple_logger.h"
#include "TypesExtra.h"

const float OPEN_TIME = 1.0;


Entity* createDoor(Entity* parent) {
	Entity* newDoor = interactableNew(DOOR, gfc_vector3d(-4, 0, 0));
	newDoor->parent = parent;
	newDoor->update = doorUpdate;

	
	Interactable* doorInteractable = (Interactable*)newDoor->data;
	doorInteractable->interact = doorInteract;

	DoorData* doorData = (DoorData*)malloc(sizeof(DoorData));
	if (!doorData) {
		slog("Could not create door data");
		free(doorInteractable);
		_entityFree(newDoor);
		return NULL;
	}
	doorData->doorState = CLOSED;
	doorInteractable->data = doorData;

	setInteractText(doorInteractable, "Open door");

	return newDoor;
}


void doorUpdate(Entity* self, float delta) {
	Interactable* interact = (Interactable*) self->data;
	DoorData* doorData = (DoorData*) interact->data;
	if (doorData->doorState == OPENING) {
		self->rotation.z = fMoveTowards(self->rotation.z, -1.404, delta * 3);
		
		if ((self->rotation.z < -1.404 + GFC_EPSILON) && (self->rotation.z > -1.404 - GFC_EPSILON)) {
			doorData->doorState = OPEN;
			setInteractText(interact, "Close door");
		}
	}
	else if (doorData->doorState == CLOSING) {
		self->rotation.z = fMoveTowards(self->rotation.z, 0.0, delta * 3);
		if ((self->rotation.z < GFC_EPSILON) && (self->rotation.z > -GFC_EPSILON)) {
			doorData->doorState = CLOSED;
			setInteractText(interact, "Open door");
		}
	}
}

void doorInteract(Entity* player, Entity* entity, Interactable* interact) {
	DoorData* doorData = (DoorData*)interact->data;
	if (doorData->doorState == OPENING || doorData->doorState == CLOSING || doorData->doorState == BROKEN) {
		return;
	}
	if (doorData->doorState == OPEN) {
		doorData->doorState = CLOSING;
		setInteractText(interact, "");
	} else if (doorData->doorState == CLOSED) {
		doorData->doorState = OPENING;
		setInteractText(interact, "");
	}

}