#ifndef __EQCLIENT_WLD_H_
#define __EQCLIENT_WLD_H_

#include <GL/gl.h>
#include "s3d.h"
#include "types.h"

#define FRAGMENT(name) status = name(&frag_obj, obj, (wld + pos + sizeof(struct_wld_basic_frag)), frag->size, frag->nameRef)
#define FRAGMENT_FUNC(name) inline int name(void **obj, wld_object *wld, uchar *buf, int len, int frag_name)

struct struct_wld_header {
  long magic;
  long version;
  long fragmentCount;
  long header3;
  long header4;
  long stringHashSize;
  long header6;
} typedef struct_wld_header;

struct struct_wld_basic_frag {
  long size;
  long id;
  long nameRef;
} typedef struct_wld_basic_frag;


struct struct_Data36 {
  long flags;
  long fragment1;
  long fragment2;
  long fragment3;
  long fragment4;
  float centerX;
  float centerY;
  float centerZ;
  long params2[3]; // 48
  float maxDist;
  float minX;
  float minY;
  float minZ;
  float maxX;
  float maxY;
  float maxZ; // 24
  short int vertexCount;
  short int texCoordsCount;
  short int normalsCount;
  short int colorCount;
  short int polygonsCount;
  short int size6;
  short int polygonTexCount;
  short int vertexTexCount;
  short int size9;
  short int scale; // 20
} typedef struct_Data36;

struct Data10 {
  long flags, size1, fragment;
};

struct Vertex {
  double x, y, z;
  float u, v;
  short skin;
} typedef Vertex;

struct Vert {
  signed short int x, y, z;
} typedef Vert;

struct TexCoordsNew {
  signed long tx, tz;
} typedef TexCoordsNew;

struct TexCoordsOld {
  signed short int tx, tz;
} typedef TexCoordsOld;

struct VertexNormal {
  signed char nx, ny, nz;
} typedef VertexNormal;

struct VertexColor {
  char color[4];
} typedef VertexColor;

struct Polygon {
  short int flags, v1, v2, v3;
} typedef Polygon;

struct Poly {
  long int flags, v1, v2, v3, tex;
  double normal[3];
} typedef Poly;

struct Texture {
  int count;
  int *flags;
  int params;
  char **filenames;
  GLuint *textures;
} typedef Texture;

struct Mesh {
  long name;
  long polygonCount;
  long vertexCount;
  Vertex *verti;
  Poly *poly;
  Texture *tex;
} typedef Mesh;

struct ZoneMesh {
  long name;
  long polygonCount;
  long vertexCount;
  Vertex **verti;
  Poly **poly;
  Texture *tex;
} typedef ZoneMesh;

struct struct_frag {
  long type;
  long name;
  void *frag;
} typedef struct_frag;

struct struct_Data21 {
  float normal[3], splitdistance;
  long region, node[2];
} typedef struct_Data21;

struct struct_Data22 {
  long flags, fragment1, size1, size2, params1, size3, size4, params2, size5, size6;
} typedef struct_Data22;

struct BSP_Region {
  
} typedef BSP_Region;

struct BSP_Node {
  float normal[3], splitdistance;
  BSP_Region *region;
  struct BSP_Node *left, *right;
} typedef BSP_Node;

struct struct_Data15 {
  uint32 ref, flags, fragment1;
  float trans[3], rot[3];
  float scale[3];
  uint32 fragment2, flags2;
} typedef struct_Data15;

struct Placeable {
  float trans[3], rot[3], scale[3];
  Mesh *mesh;
} typedef Placeable;

struct Placeable_LL {
  Placeable *obj;
  struct Placeable_LL *next;
} typedef Placeable_LL;

struct wld_object {
  int fragCount;
  char loadBSP;
  uchar *wld;
  uchar *sHash;
  uint8 new;
  s3d_object *s3d;
  Placeable_LL *placeable, *placeable_cur;
  struct wld_object *objs;
  struct_frag **frags;
} typedef wld_object;

int WLD_Init(wld_object *obj, uchar *wld, s3d_object *s3d, char loadBSP);
int WLD_GetZoneMesh(wld_object *obj, ZoneMesh *mesh);

#endif
