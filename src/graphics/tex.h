#ifndef ILG_TEX_H
#define ILG_TEX_H

#include <GL/glew.h>

struct ilG_context;
struct ilA_file;
struct ilA_img;

typedef struct ilG_tex {
    unsigned unit;
    _Bool complete;
    GLenum target;
    GLenum object;
    void (*build)(struct ilG_tex *self, struct ilG_context *context);
    void *data;
} ilG_tex;

void ilG_tex_loadfile(ilG_tex *self, const char *file);
void ilG_tex_loadasset(ilG_tex *self, const struct ilA_file *iface, void *file);
void ilG_tex_loadcube(ilG_tex *self, struct ilA_img *faces[6]);
void ilG_tex_loadimage(ilG_tex *self, struct ilA_img *img);
void ilG_tex_loaddata(ilG_tex *self, GLenum target, GLenum internalformat, unsigned width, unsigned height, unsigned depth, GLenum format, GLenum type, void *data);

void ilG_tex_bind(ilG_tex *self);
void ilG_tex_build(ilG_tex *self, struct ilG_context *context);

#endif

