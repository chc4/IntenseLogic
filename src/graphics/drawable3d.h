#ifndef IL_GRAPHICS_DRAWABLE3D_H
#define IL_GRAPHICS_DRAWABLE3D_H

#include "common/base.h"

struct ilG_context;

typedef struct ilG_drawable3d ilG_drawable3d;

struct ilG_drawable3d {
    il_base base;
    unsigned long long attrs;
    struct ilG_context *context;
    const char *name;
};

extern il_type ilG_drawable3d_type;

#endif

