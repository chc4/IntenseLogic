#include "common/base.h"

struct ilG_drawable3d;

struct ilG_obj_mesh;

extern il_type ilG_mesh_type;

struct ilG_drawable3d* ilG_mesh_fromObj(struct ilG_obj_mesh * mesh);

struct ilG_drawable3d* ilG_mesh_fromFile(const char *name);

