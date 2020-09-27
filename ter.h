#ifndef __EQCLIENT_TER_H_
#define __EQCLIENT_TER_H_

#include "wld.h"

struct ter_header {
  char magic[4];
  unsigned long version, list_len, mat_count, vert_count, tri_count;
} typedef ter_header;

struct ter_vertex {
  float x, y, z;
  float i, j, k;
  float u, v;
} typedef ter_vertex;

struct ter_triangle {
  long v1, v2, v3;
  long group;
  long unk;
} typedef ter_triangle;

struct material {
  char *name;
  char *basetex;
  char var_count;
  char **var_names;
  char **var_vals;
} typedef material;

struct ter_object {
  ZoneMesh mesh;
} typedef ter_object;

int TER_Init(ter_object *obj, uchar *ter, int ter_size, s3d_object *s3d);

#endif
