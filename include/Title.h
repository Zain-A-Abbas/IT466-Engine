#ifndef __TITLE__
#define __TITLE__

#include "gf2d_actor.h"

typedef struct {
	Actor*			bgActor;
	Actor*			logoActor;
    int             selectedOption;
} TitleData ;

void processTitle();
void createTitle();

void drawTitle();

#endif