#ifndef IL_GRAPHICS_TRIMESH_H
#define IL_GRAPHICS_TRIMESH_H

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#endif

#include "common/mesh.h"
#include "graphics/drawable3d.h"

typedef struct il_Graphics_Trimesh {
  il_Graphics_Drawable3d drawable;
  GLuint vbo;
  GLenum mode;
  GLuint num;
} il_Graphics_Trimesh;

il_Graphics_Trimesh * il_Graphics_Trimesh_new ( const il_Common_Positionable * positionable, 
                                                const il_Common_FaceMesh * mesh, 
                                                GLuint vert, GLuint frag, GLuint geom );

#endif
