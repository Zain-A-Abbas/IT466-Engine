#ifndef __EXTRA_PRIMITIVES__
#define __EXTRA_PRIMITIVES__

#include "gfc_primitives.h"

typedef struct GFC_Capsule_S {
    float radius, height; // Height starts from base
    float rotation_x, rotation_z;
    GFC_Vector3D tip, finalBase, finalTip; // finalBase is entity position, finalTip is tip transformed by entity's scale -> rotation -> position
} GFC_Capsule;

typedef enum
{
    E_Primitive,
    E_Capsule
}GFC_ExtendedPrimitiveTypes;


typedef struct
{
    GFC_ExtendedPrimitiveTypes type;
    union
    {
        GFC_Primitive p;
        GFC_Capsule c;
    }s;
}GFC_ExtendedPrimitive;


GFC_Capsule gfc_capsule(float height, float radius);

#endif