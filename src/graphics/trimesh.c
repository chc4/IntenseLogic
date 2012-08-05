#include "trimesh.h"

#include "common/log.h"

#include <stdlib.h>

void trimesh_draw(il_Graphics_Drawable3d * drawable) {
  il_Graphics_Trimesh *this = (il_Graphics_Trimesh*)drawable;
  
  glUseProgramObjectARB(this->drawable.program);
  glDrawArrays(this->mode, 0, this->num);
}

il_Graphics_Trimesh * il_Graphics_Trimesh_new(const il_Common_FaceMesh * mesh, GLuint vert, GLuint frag, GLuint geom) {
  
  if (!GLEW_ARB_vertex_buffer_object) {
    il_Common_log(0, "Vertex buffer objects not supported. Update your GL to have GL_ARB_vertex_buffer_object.");
    exit(1);
  }
  
  if (!GL_ARB_shader_objects || !GL_ARB_vertex_shader || !GL_ARB_fragment_shader) {
    il_Common_log(0, "Shaders not supported. Update your GL to have GL_ARB_shader_objects,"
                     " GL_ARB_vertex_shader, and GL_ARB_fragment_shader");
    exit(1);
  }
  
  struct point {
    sg_Vector4 vertices;
    sg_Vector2 texcoords;
    sg_Vector3 normal;
  } *data = malloc(sizeof(struct point) * mesh->faces_len * 3);
  
  int i;
  il_Common_Face *cur;
  for (i = 0; i < mesh->faces_len; i++) {
    cur = &mesh->faces[i];
    if (cur->edges != 3) {
      il_Common_log(1, "Unexpected number of faces in triangle mesh: %u", cur->edges);
    }
    
    data[i * 3] = (struct point) {
      (sg_Vector4) mesh->vertices[cur->points[0]],
      (sg_Vector2) {mesh->texcoords[cur->texcoords[0]].x,mesh->texcoords[cur->texcoords[0]].y},
      (sg_Vector3) mesh->normals[cur->normal]
    };
    data[i * 3 + 1] = (struct point) {
      (sg_Vector4) mesh->vertices[cur->points[1]],
      (sg_Vector2) {mesh->texcoords[cur->texcoords[1]].x,mesh->texcoords[cur->texcoords[1]].y},
      (sg_Vector3) mesh->normals[cur->normal]
    };
    data[i * 3 + 2] = (struct point) {
      (sg_Vector4) mesh->vertices[cur->points[2]],
      (sg_Vector2) {mesh->texcoords[cur->texcoords[2]].x,mesh->texcoords[cur->texcoords[2]].y},
      (sg_Vector3) mesh->normals[cur->normal]
    };

  }
  
  il_Graphics_Trimesh *trimesh = malloc(sizeof(il_Graphics_Trimesh));
  trimesh->mode = GL_TRIANGLES;
  trimesh->num = mesh->faces_len;
  trimesh->drawable.draw = &trimesh_draw;
  
  glGenBuffersARB(1, &trimesh->vbo);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, trimesh->vbo);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(struct point) * mesh->faces_len, data, GL_STATIC_DRAW_ARB);
  free(data);
  
  /* void glVertexAttribPointer(GLuint index​, GLint size​, GLenum type​, 
     GLboolean normalized​, GLsizei stride​, const GLvoid * pointer​); */
  glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct point), (void*) (sizeof(float) * 0)); // vertices
  glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct point), (void*) (sizeof(float) * 3)); // texcoords
  glVertexAttribPointerARB(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct point), (void*) (sizeof(float) * 5)); // normals
  
  glEnableVertexAttribArrayARB(0);
  glEnableVertexAttribArrayARB(1);
  glEnableVertexAttribArrayARB(2);
  
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, trimesh->vbo);
  
  //////////////////////////////
  
  trimesh->drawable.program = glCreateProgramObjectARB();
  
  if (vert)
    glAttachObjectARB(trimesh->drawable.program, vert);
  if (geom)
    glAttachObjectARB(trimesh->drawable.program, geom);
  if (frag)
    glAttachObjectARB(trimesh->drawable.program, frag);
  
  glBindAttribLocationARB(trimesh->drawable.program, 0, "in_Position");
  glBindAttribLocationARB(trimesh->drawable.program, 0, "in_Texcoord");
  glBindAttribLocationARB(trimesh->drawable.program, 0, "in_Normal");
  
  glLinkProgramARB(trimesh->drawable.program);
  
  return trimesh;
}
