#include "Zombie.h"
#include "simple_logger.h"
#include "TypesExtra.h"

const int HP = 200;
const float AGGRO_RANGE = 32;

const float WANDER_SPEED = 2;
const float WANDER_AI_INTERVAL = 4.0;

const float CHASE_SPEED = 5;
const float CHASE_TURN_SPEED = 8;
const float CHASE_AI_INTERVAL = 0.12;

Entity* createZombie(Entity *player) {
	Entity* newZombie = enemyEntityNew();

	// Make and assign states
	StateMachine* stateMachine = (StateMachine*)malloc(sizeof(StateMachine));
	if (!stateMachine) {
		slog("Could not allocate enemy state machine");
		free(newZombie);
		return NULL;

	}
	memset(stateMachine, 0, sizeof(StateMachine));

	State* wanderState = createState("Wander", stateMachine, wanderEnter, NULL, wanderThink, wanderUpdate, wanderOnHit, calloc(1, sizeof(WanderData)));
	State* chaseState = createState("Chase", stateMachine, NULL, NULL, chaseThink, chaseUpdate, NULL, calloc(1, sizeof(ChaseData)));
	ChaseData* chaseData = (ChaseData*)chaseState->stateData;
	chaseData->player = player;
	WanderData* wanderData = (WanderData*)wanderState->stateData;
	wanderData->player = player;

	changeState(newZombie, stateMachine, "Wander");


	EnemyData* enemyData = (EnemyData*)newZombie->data;
	enemyData->hp = HP;
	enemyData->enemyStateMachine = stateMachine;

	// Animation/Models

	animationSetup(
        newZombie,
        "models/enemies/zombie/", 
        (char *[]){
            "ZombieA"
        },
        1
    );
    animationPlay(newZombie, "models/enemies/zombie/ZombieA.model");

	// Material

	//gf3d_material_load_mtl_file("models/enemies/zombie/Zombie.mtl");
	//newZombie->model->material = gf3d_material_get_by_name("Zombie0001");


	// Collision
    EntityCollision* collision = (EntityCollision*) malloc(sizeof(EntityCollision));
    memset(collision, 0, sizeof(EntityCollision));

    // Create capsule
    GFC_ExtendedPrimitive* collisionPrimitive = (GFC_ExtendedPrimitive*)malloc(sizeof(GFC_ExtendedPrimitive));
    memset(collisionPrimitive, 0, sizeof(GFC_ExtendedPrimitive));
    collisionPrimitive->type = E_Capsule;
    GFC_Capsule playerCapsule = gfc_capsule(8, 2);
    collisionPrimitive->s.c = playerCapsule;
    collision->collisionPrimitive = collisionPrimitive;
    
    GFC_Box boundingBox = {0};
    boundingBox.w = 8;
    boundingBox.d = 24;
    boundingBox.h = 8;

    collision->AABB = boundingBox;

    newZombie->entityCollision = collision;


	return newZombie;
}


// WANDER

void wanderEnter(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	enemyData->aiTime = WANDER_AI_INTERVAL;
}

void wanderUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
	printf("\nPosition: %f, %f, %f", self->position.x, self->position.y, self->position.z);
}

void wanderThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	WanderData* wanderData = (WanderData*)state->stateData;
	enemyData->aiTime += delta;

	if (enemyData->aiTime >= WANDER_AI_INTERVAL) {
		if (gfc_vector3d_distance_between_less_than(entityGlobalPosition(wanderData->player), entityGlobalPosition(self), AGGRO_RANGE)) {
			changeState(self, stateMachine, "Chase");
			return;
		}
		enemyData->aiTime = 0;
		enemyData->character3dData->rotation.z = gfc_random() * 4 - 8;
		enemyData->character3dData->velocity = gfc_vector3d(0, -WANDER_SPEED * delta, 0);
		gfc_vector3d_rotate_about_z(&enemyData->character3dData->velocity, enemyData->character3dData->rotation.z);
	}
}


void wanderOnHit(struct Entity_S* self, struct State_S* state, StateMachine* stateMachine) {
	changeState(self, stateMachine, "Chase");
}

// CHASE

void chaseUpdate(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;

	float modelRotation = fMoveTowardsAngle(self->rotation.z, enemyData->character3dData->rotation.z, delta);
	self->rotation.z = modelRotation;
	moveAndSlide(self, enemyData->character3dData, delta);
	//enemyHorizontalWallSlide(self, enemyData->character3dData, delta);
}

void chaseThink(struct Entity_S* self, float delta, struct State_S* state, StateMachine* stateMachine) {
	EnemyData* enemyData = (EnemyData*)self->data;
	ChaseData* chaseData = (ChaseData*)state->stateData;
	enemyData->aiTime += delta;
	if (enemyData->aiTime >= CHASE_AI_INTERVAL) {

		enemyData->aiTime = 0;

		GFC_Vector2D positionDifference = gfc_vector2d(chaseData->player->position.x - self->position.x, chaseData->player->position.y - self->position.y);
		gfc_vector2d_normalize(&positionDifference);
		float angleTarget = gfc_vector2d_angle(gfc_vector2d(positionDifference.x, positionDifference.y)) - GFC_HALF_PI;
		float angleDifference = angleTarget - enemyData->character3dData->rotation.z + GFC_HALF_PI;
		while (angleDifference < -GFC_PI_HALFPI) {
			angleDifference += GFC_2PI;
		}
		while (angleDifference > GFC_HALF_PI) {
			angleDifference -= GFC_2PI;
		}

		float rotationAmount = CHASE_TURN_SPEED * delta;

		if (fabsf(angleDifference) <= rotationAmount) {
			enemyData->character3dData->rotation.z = angleTarget + GFC_HALF_PI;
		} else if (angleDifference > 0) {
			enemyData->character3dData->rotation.z += rotationAmount;

		}
		else if (angleDifference < 0) {
			enemyData->character3dData->rotation.z -= rotationAmount;
		}

		if (fabsf(enemyData->character3dData->rotation.z) > GFC_2PI) {
			enemyData->character3dData->rotation.z = fMoveTowards(enemyData->character3dData->rotation.z, 0.0, GFC_2PI);
		}
		

		if (fabsf(angleDifference) < GFC_PI/32.0) {
			enemyData->character3dData->velocity = gfc_vector3d(0, -CHASE_SPEED * delta, 0);
			gfc_vector3d_rotate_about_z(&enemyData->character3dData->velocity, enemyData->character3dData->rotation.z);
		}
		else {
			enemyData->character3dData->velocity = gfc_vector3d(0, 0, 0);
		}
	}
}