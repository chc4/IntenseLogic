#ifndef IL_COMMON_MESH_H
#define IL_COMMON_MESH_H

#include <stdlib.h>

#include "common/matrix.h"
#include "asset/asset.h"

typedef struct il_Common_Face {
  size_t edges;
  unsigned *points;
  unsigned *texcoords;
  unsigned normal;
} il_Common_Face;

typedef struct il_Common_FaceMesh {
  size_t faces_len;
  il_Common_Face * faces;
  size_t vertices_len;
  sg_Vector4 * vertices;
  size_t texcoords_len;
  sg_Vector3 * texcoords;
  size_t normals_len;
  sg_Vector3 * normals;
  unsigned refs;
} il_Common_FaceMesh;

il_Common_FaceMesh * il_Common_FaceMesh_fromAsset(il_Asset_Asset * asset);

#endif
