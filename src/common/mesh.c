#include "mesh.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "common/log.h"

/*int loadild(il_Asset_Asset * asset, il_Common_FaceMesh ** mesh) {
  struct {
    char magic[4];
    uint16_t version;
  } header;
  FILE *handle = il_Asset_getHandle(asset);
  size_t res = fread(&header, sizeof(header), 1, handle);
  if (res < 1) {
    il_Common_log(1, "Invalid ILD file: too short (read %u out of %u-bytes)", res, sizeof(header));
    return -1;
  }
  if (strncmp(&header.magic[0], "ILD\0", 4) != 0) {
    il_Common_log(1, "Invalid magic number for ILD file: %x", *(unsigned*)&header.magic[0]);
    return -1;
  }
  if (header.version != 1) {
    il_Common_log(1, "Can't handle version %u of ILD, please use version 1", header.version);
    return -1;
  }
  
  *mesh = malloc(sizeof(il_Common_FaceMesh));
  
  struct {
    char id[4];
    uint32_t length;
  } chunk;
  
  while (1) {
    res = fread(&chunk, sizeof(chunk), 1, handle);
    if (res < 1) break;
    
    void *buf = malloc(chunk.length);
    
    res = fread(buf, 1, sizeof(chunk), handle);
    
    if (strncmp(&chunk.id[0], "FACE", 4) == 0) {
    
    }
    
  }
  
}*/

int loadobj(il_Asset_Asset * asset, il_Common_FaceMesh ** mesh) {

  il_Common_FaceMesh *m = malloc(sizeof(il_Common_FaceMesh));
  
  FILE *handle = il_Asset_getHandle(asset);
  
  sg_Vector4 v;
  sg_Vector3 vt;
  sg_Vector3 vn;
  struct face {
    int v;
    int vt;
    int vn;
  } f;
  
  struct vnode {
    sg_Vector4 val;
    struct vnode *prev;
  } *vcur;
  
  struct vtnode {
    sg_Vector3 val;
    struct vtnode *prev;
  } *vtcur;
  
  struct vnnode {
    sg_Vector3 val;
    struct vnnode *prev;
  } *vncur;
  
  struct fnode {
    size_t len;
    struct face* val;
    struct fnode *prev;
  } *fcur;

  int line_number;
  while (1) {
    line_number++;
    size_t size = 1024;
    int len = 0;
    char *line = malloc(size);
    int c;
    while ( c != '\n' && c != EOF) {
      if (len >= size) {
        size *= 2;
        char *temp = malloc(size);
        memcpy(temp, line, len);
        line = temp;
      }
      line[len] = c;
      len++;
      c = fgetc(handle);
    }
    line[len] = '\0';
    len++;
    
    if (line[0] == '#')
      goto end;
    
    int offset, offset2, offset3;
    if ((offset=sscanf(line, "v %f", &v.x))) {
      if ((offset2=sscanf(line+offset, " %f", &v.y)))
        if ((offset3=sscanf(line+offset+offset2, " %f", &v.z)))
          sscanf(line+offset+offset2+offset3, " %f", &v.w);
      struct vnode *node = malloc(sizeof(struct vnode));
      node->val = v;
      node->prev = vcur;
      vcur = node;
      v = (sg_Vector4){0,0,0,1};
      goto end;
    }
    if ((offset=sscanf(line, "vt %f", &vt.x))) {
      if ((offset2=sscanf(line+offset, " %f", &vt.y)))
        sscanf(line+offset+offset2, " %f", &vt.z);
      struct vtnode *node = malloc(sizeof(struct vtnode));
      node->val = vt;
      node->prev = vtcur;
      vtcur = node;
      vt = (sg_Vector3){0,0,0};
      goto end;
    }
    if (sscanf(line, "vn %f %f %f", &vt.x, &vt.y, &vt.z)) {
      struct vnnode *node = malloc(sizeof(struct vnnode));
      node->val = vn;
      node->prev = vncur;
      vncur = node;
      vn = (sg_Vector3){0,0,0};
      goto end;
    }
    if ((offset=sscanf(line, "f"))) {
      char* verts[16];
      int i = 0;
      while (sscanf(line+offset, " %as", &verts[i])) i++;
      
      struct fnode *node = malloc(sizeof(struct fnode));
      node->len = i;
      node->val = malloc(sizeof(struct face) * i);
      node->prev = fcur;
      
      int j;
      for (j = 0; j < i; j++) {
        if (!(offset2=sscanf(verts[j], "%i", &f.v))) {
          il_Common_log(2, "Vertex for face value is not a float at %s:%u column %u.", 
                        il_Asset_getName(asset), line_number, offset+i+j);
          goto end;
        }
        
        offset3=sscanf(verts[j]+offset2, "/%i", &f.vt);
        sscanf(verts[j]+offset2+offset3, "/%i", &f.vn);
        
        node->val[j] = f;
      }
      
      fcur = node;
      
      goto end;
    }
    
    il_Common_log(2, "Unrecognised line in OBJ file %s:%u.", il_Asset_getName(asset), line_number);
    
   end: 
    free(line);
  
  }
  
  size_t vcount = 0;
  size_t vtcount = 0;
  size_t vncount = 0;
  size_t fcount = 0;
  int i = 0;
  
  void *old = vcur;
  while (vcur) {
    vcount++;
    vcur = vcur->prev;
  }
  vcur = old;
  m->vertices_len = vcount;
  m->vertices = malloc(vcount * sizeof(sg_Vector4));
  while (vcur) {
    m->vertices[i] = vcur->val;
    i++;
    vcur = vcur->prev;
  }
  
  old = vtcur;
  while (vtcur) {
    vtcount++;
    vtcur = vtcur->prev;
  }
  vtcur = old;
  m->texcoords_len = vtcount;
  m->texcoords = malloc(vtcount * sizeof(sg_Vector3));
  while (vtcur) {
    m->texcoords[i] = vtcur->val;
    i++;
    vtcur = vtcur->prev;
  }
  
  old = vncur;
  while (vncur) {
    vncount++;
    vncur = vncur->prev;
  }
  vncur = old;
  m->normals_len = vncount;
  m->normals = malloc(vcount * sizeof(sg_Vector3));
  while (vncur) {
    m->normals[i] = vncur->val;
    i++;
    vncur = vncur->prev;
  }
  
  old = fcur;
  while (fcur) {
    fcount++;
    fcur = fcur->prev;
  }
  fcur = old;
  m->faces_len = fcount;
  m->faces = malloc(fcount * sizeof(il_Common_Face));
  while (fcur) {
    //m->vertices[i] = fcur.val;
    il_Common_Face face;
    face.edges = fcur->len;
    face.points = malloc(sizeof(unsigned) * fcur->len);
    face.texcoords = malloc(sizeof(unsigned) * fcur->len);
    int j;
    for (j=0; j < fcur->len; j++) {
      face.points[j] = fcur->val[j].v;
      face.texcoords[j] = fcur->val[j].vt;
    }
    face.normal = fcur->val[0].vn;
    i++;
    fcur = fcur->prev;
  }
  
  *mesh = m;
  
  return 0;
  
}

il_Common_FaceMesh * il_Common_FaceMesh_fromAsset(il_Asset_Asset * asset) {
  il_Common_FaceMesh *m;
  loadobj(asset, &m);
  return m;
}
