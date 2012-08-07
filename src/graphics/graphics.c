#include <stdlib.h>
#include <time.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#endif

#include "SDL/SDL.h"

#include "graphics.h"
#include "heightmap.h"
#include "common/heightmap.h"
#include "camera.h"
#include "common/event.h"
#include "common/input.h"
#include "common/matrix.h"
#include "common/mesh.h"
#include "graphics/trimesh.h"
#include "asset/asset.h"
#include "common/log.h"
#include "common/mesh.h"

SDL_Surface* canvas;
int width = 800;
int height = 600;
float heights[4] = {50, 0,0,0}; //temp
il_Graphics_Heightmap* h;
il_Graphics_Trimesh* trimesh;
il_Graphics_Camera* camera;
float theta;
sg_Vector3 speed = (sg_Vector3){0, 0, 0};
sg_Vector3 rot = (sg_Vector3){0,0,0};

void il_Graphics_init() {

	srand((unsigned)time(NULL)); //temp
        SDL_Init(SDL_INIT_EVERYTHING);
        canvas = SDL_SetVideoMode(width, height, 32, SDL_OPENGL| SDL_HWSURFACE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); 
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    il_Common_log(0, "glewInit() failed: %s\n", glewGetErrorString(err));
    exit(1);
  }
	
	glFrontFace(GL_CW);

	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustum(-2, 2, -1, 1, 1.0f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();

	GLfloat diffuse[] = { 1.0, 1.0, 1.0};
	GLfloat lightPosition[] = {0, 0.5, 0.5, 0.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	h = il_Graphics_Heightmap_new(il_Common_Heightmap_new(heights));
  //h->drawable.positionable->position = (sg_Vector3){-5, 0, -5};

	int i;
	for (i = 0; i < 6; i++) {
		il_Common_Heightmap_Quad_divide(h->heightmap->root, 0, NULL);
	}
  
  il_Common_Positionable * positionable = malloc(sizeof(il_Common_Positionable));
  positionable->position = (sg_Vector3){-5, -5, -5};
  positionable->size = (sg_Vector3){10, 10, 10};
  
  il_Asset_Asset *vertfile = il_Asset_open(il_Common_fromC("cube.vert"));
  il_Asset_Asset *fragfile = il_Asset_open(il_Common_fromC("cube.frag"));
  
  GLuint vertshader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragshader = glCreateShader(GL_FRAGMENT_SHADER);
  
  il_Common_String vertsource = il_Asset_read(vertfile);
  il_Common_String fragsource = il_Asset_read(fragfile);
  
  il_Common_log ( 5, 
                  "----- *** BEGIN VERTEX SHADER SOURCE *** -----\n"
                  "%s\n"
                  "----- *** END VERTEX SHADER SOURCE *** -----\n", il_Common_toC(vertsource) );
  il_Common_log ( 5, 
                  "----- *** BEGIN FRAGMENT SHADER SOURCE *** -----\n"
                  "%s\n"
                  "----- *** END FRAGMENT SHADER SOURCE *** -----\n", il_Common_toC(fragsource) );
  
  glShaderSource(vertshader, 1, (const GLchar**)&vertsource.data, (GLint*)&vertsource.length);
  glShaderSource(fragshader, 1, (const GLchar**)&fragsource.data, (GLint*)&fragsource.length);
  
  glCompileShader(vertshader);
  glCompileShader(fragshader);
  
  GLint success;
  glGetShaderiv(vertshader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint len;
    glGetShaderiv(vertshader, GL_INFO_LOG_LENGTH, &len);
    GLchar *buf = malloc(len * sizeof(GLchar));
    glGetShaderInfoLog(vertshader, len, NULL, buf);
    il_Common_log(0, "Vertex shader compilation: %s", (char*)buf);
    //exit(1);
  }
  glGetShaderiv(fragshader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint len;
    glGetShaderiv(fragshader, GL_INFO_LOG_LENGTH, &len);
    GLchar *buf = malloc(len * sizeof(GLchar));
    glGetShaderInfoLog(fragshader, len, NULL, buf);
    il_Common_log(0, "Fragment shader compilation: %s", (char*)buf);
    //exit(1);
  }
  
  il_Common_FaceMesh *mesh = malloc(sizeof(il_Common_FaceMesh));
  
  mesh->vertices_len = 8;
  mesh->vertices = malloc(sizeof(sg_Vector4) * 8);
  sg_Vector4 verts[] =  {
    (sg_Vector4) { 1.0, -1.0, -1.0, 1.0},
    (sg_Vector4) { 1.0, -1.0,  1.0, 1.0},
    (sg_Vector4) {-1.0, -1.0, -1.0, 1.0},
    (sg_Vector4) {-1.0, -1.0, -1.0, 1.0},
    (sg_Vector4) { 1.0,  1.0, -1.0, 1.0},
    (sg_Vector4) { 1.0,  1.0,  1.0, 1.0},
    (sg_Vector4) {-1.0,  1.0,  1.0, 1.0},
    (sg_Vector4) {-1.0,  1.0, -1.0, 1.0}
  };
  memcpy(mesh->vertices, &verts, sizeof(sg_Vector4) * 8);
  mesh->texcoords_len = 1;
  mesh->texcoords = malloc(sizeof(sg_Vector2));
  mesh->normals_len = 1;
  mesh->normals = malloc(sizeof(sg_Vector3));
  mesh->faces_len = 6;
  mesh->faces = malloc(sizeof(il_Common_Face) * 6);
  
  unsigned points[6][4] = {
    {0, 1, 2, 3},
    {4, 7, 6, 5},
    {0, 4, 5, 1},
    {1, 5, 6, 2},
    {2, 6, 7, 3},
    {4, 0, 3, 7}
  };
  
  #define face(x) \
  mesh->faces[x].edges = 3;\
  mesh->faces[x].points = malloc(4 * sizeof(unsigned));\
  memcpy(mesh->faces[x].points, &points[x], sizeof(unsigned) * 4);\
  mesh->faces[x].texcoords = calloc(sizeof(unsigned), 4);\
  mesh->faces[x].normal = 0;
  
  face(0);
  face(1);
  face(2);
  face(3);
  face(4);
  face(5);
  
  trimesh = il_Graphics_Trimesh_new(
    positionable,
    mesh, //il_Common_FaceMesh_fromAsset(il_Asset_open(il_Common_fromC("cube.obj"))),
    vertshader, fragshader, 0
  );
  
	
	il_Event_register(IL_INPUT_KEYDOWN, (il_Event_Callback)&handleKeyDown);
	il_Event_register(IL_INPUT_KEYUP, (il_Event_Callback)&handleKeyUp);
	
	camera = il_Graphics_Camera_new();

}

void il_Graphics_draw() {

	//GLfloat lightPosition[] = {0, 0.5, 0.5, 0.0};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity();
	//glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	il_Graphics_Camera_translate(camera, speed.x, speed.y, speed.z);
  /*camera->positionable->rotation = sg_Matrix_rotate_v( 
    camera->positionable->rotation,
    3.14 / 120,
    rot
  );*/
	//il_Graphics_Camera_render(camera);

	//glRotatef(theta, 0, 1, 0);
	theta += 0.1;

	//glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	//h->drawable.draw(&h->drawable);
  
  trimesh->drawable.draw(&trimesh->drawable, camera);

	SDL_GL_SwapBuffers();
}

void handleKeyDown(il_Event_Event* ev) {
	int keyCode = *(int*)&ev->data;
	if (keyCode == SDLK_LEFT || keyCode == SDLK_a) {
		speed.x = -0.1f;
	} else if (keyCode == SDLK_RIGHT || keyCode == SDLK_d) {
		speed.x = 0.1f;
	}
	if (keyCode == SDLK_r) {
		speed.y = 0.1f;
	} else if (keyCode == SDLK_f) {
		speed.y = -0.1f;
	}
	if (keyCode == SDLK_DOWN || keyCode == SDLK_s) {
		speed.z = 0.1f;
	} else if (keyCode == SDLK_UP || keyCode == SDLK_w) {
		speed.z = -0.1f;
	}
  if (keyCode == SDLK_q) {
    rot.y = -1.0f;
  } else if (keyCode == SDLK_e) {
    rot.y = 1.0f;
  }
}

void handleKeyUp(il_Event_Event* ev) {
	int keyCode = *(int*)&ev->data;
	if (keyCode == SDLK_LEFT || keyCode == SDLK_RIGHT || keyCode == SDLK_a || keyCode == SDLK_d) {
		speed.x = 0;
	} else if (keyCode == SDLK_r || keyCode == SDLK_f) {
		speed.y = 0;
	} else if (keyCode == SDLK_DOWN || keyCode == SDLK_UP || keyCode == SDLK_w || keyCode == SDLK_s) {
		speed.z = 0;
	}
  if (keyCode == SDLK_q || keyCode == SDLK_e) {
    rot.y = 0.0f;
  }
}

void il_Graphics_quit() {
	SDL_Quit();
}
