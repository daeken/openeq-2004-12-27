#ifndef __EQCLIENT_OCTREE_H_
#define __EQCLIENT_OCTREE_H_

#include <GL/gl.h>
#include "wld.h"
#include "types.h"

struct octree_end {
  double min[3], max[3];
  int polycount;
  Poly **poly;
  GLuint li;
} typedef octree_end;

struct octree_container {
  double min[3], max[3];
  double x, y, z, size;
  uchar ntypes;
  void *nodes[8];
} typedef octree_container;

int octree_init(octree_container *obj, ZoneMesh *mesh);

#define in_box(obj, x, y, z) \
( \
  (x) >= (obj)->min[0] && (x) <= (obj)->max[0] && \
  (y) >= (obj)->min[1] && (y) <= (obj)->max[1] && \
  (z) >= (obj)->min[2] && (z) <= (obj)->max[2] && \
  (x) >= (obj)->min[0] && (x) <= (obj)->max[0] && \
  (y) >= (obj)->min[1] && (y) <= (obj)->max[1] && \
  (z) >= (obj)->min[2] && (z) <= (obj)->max[2] && \
  (x) >= (obj)->min[0] && (x) <= (obj)->max[0] && \
  (y) >= (obj)->min[1] && (y) <= (obj)->max[1] && \
  (z) >= (obj)->min[2] && (z) <= (obj)->max[2]    \
)

#endif
