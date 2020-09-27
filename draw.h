#ifndef __EQCLIENT_DRAW_H_
#define __EQCLIENT_DRAW_H_

#include <GL/gl.h>
#include "wld.h"
#include "ui.h"
#include "octree.h"
#include "types.h"

extern float trans[3];
extern float rot[3];

struct draw_object {
  ZoneMesh *mesh;
  GLuint *textures;
  Placeable_LL *placeable;
  ui_object *ui;
  octree_container octree;
} typedef draw_object;

struct texcache {
  unsigned short width, height;
  char rgba;
} typedef texcache;

int Draw(ZoneMesh *mesh, s3d_object *s3d);

#endif
