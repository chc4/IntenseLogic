#include "trimesh.h"

#include <stdlib.h>
#include <stddef.h>

#include "common/log.h"
#include "graphics/camera.h"

void trimesh_draw(il_Graphics_Drawable3d * drawable, il_Graphics_Camera* camera) {
  il_Graphics_Trimesh *this = (il_Graphics_Trimesh*)drawable;
  
  glUseProgramObjectARB(this->drawable.program);
  
  glUniform3fv ( glGetUniformLocation(this->drawable.program, "camera.position"),
                 1,
                 (const GLfloat*)&camera->positionable->position );
  glUniformMatrix4fv ( glGetUniformLocation(this->drawable.program, "camera.rotation"), 
                       1, 
                       GL_TRUE, 
                       (const GLfloat*)&camera->positionable->rotation.data );
  
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, this->vao);
  glDrawElements(this->mode, this->num, GL_UNSIGNED_INT, NULL);
}

il_Graphics_Trimesh * il_Graphics_Trimesh_new ( const il_Common_Positionable * positionable, 
                                                const il_Common_FaceMesh * mesh, 
                                                GLuint vert, GLuint frag, GLuint geom ) {
  //
  if (!GLEW_ARB_vertex_buffer_object) {
    il_Common_log(0, "Vertex buffer objects not supported. Update your GL to have GL_ARB_vertex_buffer_object.");
    exit(1);
  }
  
  if (!GL_ARB_shader_objects || !GL_ARB_vertex_shader || !GL_ARB_fragment_shader) {
    il_Common_log(0, "Shaders not supported. Update your GL to have GL_ARB_shader_objects,"
                     " GL_ARB_vertex_shader, and GL_ARB_fragment_shader");
    exit(1);
  }
  
  #define max(x,y) (x>y?x:y)
  
  size_t data_size = max(max(mesh->vertices_len, mesh->texcoords_len), mesh->normals_len);
  struct point {
    sg_Vector4 vertex;
    sg_Vector2 texcoord;
    sg_Vector3 normal;
  } *data = calloc(sizeof(struct point), data_size);
  
  if (sizeof(struct point) != sizeof(float) * 9) {
    il_Common_log(0, "struct point isn't the size of its values");
  }
  
  int i;
  for (i = 0; i < mesh->vertices_len; i++) {
    data[i].vertex = mesh->vertices[i];
  }
  
  for (i = 0; i < mesh->texcoords_len; i++) {
    data[i].texcoord = (sg_Vector2){mesh->texcoords[i].x, mesh->texcoords[i].y};
  }
  
  for (i = 0; i < mesh->normals_len; i++) {
    data[i].normal = mesh->normals[i];
  }
  
  size_t numedges;
  numedges = mesh->faces[0].edges;
  
  size_t indices_size = mesh->faces_len * numedges;
  struct index {
    unsigned vert;
    unsigned tex;
    unsigned norm;
  }  *indices = calloc(sizeof(struct index), indices_size);
  
  il_Common_Face *cur = NULL;
  for (i = 0; i < mesh->faces_len; i++) {
    cur = &mesh->faces[i];
    if (cur->edges != numedges) {
      il_Common_log(1, "Unexpected number of faces in triangle mesh: %zu\n", cur->edges);
    }
    
    #define check_bounds(val, mi, ma, err, ...) \
      if (val < mi || val >= ma) \
        il_Common_log(1, err, ##__VA_ARGS__);
    
    int j;
    for (j = 0; j < numedges; j++) {
      check_bounds(cur->points[j], 0, mesh->vertices_len, "Vertex index out of bounds.\n");
    }
    for (j = 0; j < numedges; j++) {
      check_bounds(cur->texcoords[j], 0, mesh->texcoords_len, "Texcoord index out of bounds.\n");
    }
    check_bounds(cur->normal, 0, mesh->normals_len, "Normal index out of bounds.\n");
    
    for (j = 0; j < numedges; j++) {
      indices[i * numedges + j] = (struct index) {
        cur->points[j],
        cur->texcoords[j],
        cur->normal
      };
    }
    
    /*data[i * 3] = (struct point) {
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
    };*/

  }
  
  il_Graphics_Trimesh *trimesh = calloc(sizeof(il_Graphics_Trimesh), 1);
  trimesh->mode = GL_TRIANGLES;
  trimesh->num = mesh->faces_len * 3;
  trimesh->drawable.positionable = calloc(sizeof(il_Common_Positionable), 1);
  trimesh->drawable.draw = &trimesh_draw;
  
  glGenBuffersARB(1, &trimesh->vbo);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, trimesh->vbo);
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, data_size, data, GL_STATIC_DRAW_ARB);
  free(data);
  
  /* void glVertexAttribPointer(GLuint index​, GLint size​, GLenum type​, 
     GLboolean normalized​, GLsizei stride​, const GLvoid * pointer​); */
  glVertexAttribPointerARB(0, 4, GL_FLOAT, GL_FALSE, sizeof(struct point), (void*)offsetof(struct point, vertex));
  glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct point), (void*)offsetof(struct point, texcoord));
  glVertexAttribPointerARB(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct point), (void*)offsetof(struct point, normal));
  
  glEnableVertexAttribArrayARB(0);
  glEnableVertexAttribArrayARB(1);
  glEnableVertexAttribArrayARB(2);
  
  glGenBuffersARB(1, &trimesh->vao);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, trimesh->vao);
  glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indices_size, indices, GL_STATIC_DRAW_ARB);
  free(indices);
  
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
  glBindAttribLocationARB(trimesh->drawable.program, 1, "in_Texcoord");
  glBindAttribLocationARB(trimesh->drawable.program, 2, "in_Normal");
  
  glLinkProgramARB(trimesh->drawable.program);
  
  GLint success;
  glGetProgramiv(trimesh->drawable.program, GL_LINK_STATUS, &success);
  if (!success) {
    GLint length;
    glGetProgramiv(trimesh->drawable.program, GL_INFO_LOG_LENGTH, &length);
    GLchar *buf = malloc(length * sizeof(GLchar));
    glGetProgramInfoLog(trimesh->drawable.program, length, NULL, buf);
    il_Common_log(1, "Shader program link failed: %s\n", (char*)buf);
  }
  
  return trimesh;
}
