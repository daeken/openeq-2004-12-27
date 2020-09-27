#include "octree.h"

#define MIN_TRIS 500

inline char tri_box_intersect(double min[3], double max[3], Vertex *v[3]) {
  if(v[0]->x >= min[0] && v[0]->x <= max[0] &&
     v[0]->y >= min[1] && v[0]->y <= max[1] &&
     v[0]->z >= min[2] && v[0]->z <= max[2])
    return 1;

  if(v[1]->x >= min[0] && v[1]->x <= max[0] &&
     v[1]->y >= min[1] && v[1]->y <= max[1] &&
     v[1]->z >= min[2] && v[1]->z <= max[2])
    return 1;

  if(v[2]->x >= min[0] && v[2]->x <= max[0] &&
     v[2]->y >= min[1] && v[2]->y <= max[1] &&
     v[2]->z >= min[2] && v[2]->z <= max[2])
    return 1;

  return 0;
}

int get_polygons_in_box(double min[3], double max[3], int polycount, Poly **poly, Vertex **verts, Poly ***out) {
  int i, count = 0;
  Vertex *v[3];
  Poly **ret = (Poly **) malloc(polycount * sizeof(Poly *));

  for(i = 0; i < polycount; ++i) {
    v[0] = verts[poly[i]->v1];
    v[1] = verts[poly[i]->v2];
    v[2] = verts[poly[i]->v3];

    if(tri_box_intersect(min, max, v))
      ret[count++] = poly[i];
  }
  *out = (Poly **) realloc(ret, count * sizeof(Poly *));
  return count;
}

inline void octree_add_end(octree_container *obj, double min[3], double max[3], char c, int polycount, Poly **poly) {
  octree_end *end = (octree_end *) malloc(sizeof(octree_end));

  end->poly = (Poly **) malloc(polycount * sizeof(Poly *));

  memcpy(end->min, min, sizeof(double) * 3);
  memcpy(end->max, max, sizeof(double) * 3);
  memcpy(end->poly, poly, polycount * sizeof(Poly *));

  end->polycount = polycount;
  end->li = 0xDEADBEEF;

  obj->nodes[c] = end;
}

inline octree_container *octree_add_node(octree_container *obj, double min[3], double max[3], char i) {
  float x, y, z;
  octree_container *node = (octree_container *) malloc(sizeof(octree_container));

  obj->ntypes |= 1 << i;

  memcpy(node->min, min, sizeof(double) * 3);
  memcpy(node->max, max, sizeof(double) * 3);

  node->x = min[0] + (max[0] - min[0]) / 2;
  node->y = min[1] + (max[1] - min[1]) / 2;
  node->z = min[2] + (max[2] - min[2]) / 2;

  x = max[0] - min[0];
  y = max[1] - min[1];
  z = max[2] - min[2];

  if(x > y && x > z) node->size = x;
  else if(y > x && y > z) node->size = y;
  else node->size = z;

  node->ntypes = 0;

  obj->nodes[i] = node;

  return node;
}

#define set(a, i, j, k) \
a[0] = i; \
a[1] = j; \
a[2] = k;

inline void octree_find_min_max(octree_container *obj, int polycount, Poly **poly, Vertex **verts) {
  int i;
  Vertex *v[3];

  memset(obj->min, 0, sizeof(double) * 3);
  memset(obj->max, 0, sizeof(double) * 3);
    
  for(i = 0; i < polycount; ++i) {
    v[0] = verts[poly[i]->v1];
    v[1] = verts[poly[i]->v2];
    v[2] = verts[poly[i]->v3];
    
    if(v[0]->x < obj->min[0]) obj->min[0] = v[0]->x;
    if(v[1]->y < obj->min[1]) obj->min[1] = v[1]->y;
    if(v[2]->z < obj->min[2]) obj->min[2] = v[2]->z;
    
    if(v[0]->x > obj->max[0]) obj->max[0] = v[0]->x;
    if(v[1]->y > obj->max[1]) obj->max[1] = v[1]->y;
    if(v[2]->z > obj->max[2]) obj->max[2] = v[2]->z;
  }
}

void octree_split(octree_container *obj, int polycount, Poly **poly, Vertex **verts, char level) {
  double min[3], max[3], cen[3], a[3], b[3];
  double cmin[8][3], cmax[8][3];
  int i, j;
  Vertex *v[3];
  int poly_count[8];
  Poly **children[8];
  char cont;

  memset(children, 0, sizeof(Poly **) * 8);

  memcpy(min, obj->min, sizeof(double) * 3);
  memcpy(max, obj->max, sizeof(double) * 3);

  cen[0] = min[0] + (max[0] - min[0]) / 2.0;
  cen[1] = min[1] + (max[1] - min[1]) / 2.0;
  cen[2] = min[2] + (max[2] - min[2]) / 2.0;
  
  set(cmin[0], min[0], min[1], min[2]);
  set(cmax[0], cen[0], cen[1], cen[2]);
  poly_count[0] = get_polygons_in_box(cmin[0], cmax[0], polycount, poly, verts, &children[0]);

  set(cmin[1], min[0], min[1], cen[2]);
  set(cmax[1], cen[0], cen[1], max[2]);
  poly_count[1] = get_polygons_in_box(cmin[1], cmax[1], polycount, poly, verts, &children[1]);

  set(cmin[2], min[0], cen[1], min[2]);
  set(cmax[2], cen[0], max[1], cen[2]);
  poly_count[2] = get_polygons_in_box(cmin[2], cmax[2], polycount, poly, verts, &children[2]);

  set(cmin[3], min[0], cen[1], cen[2]);
  set(cmax[3], cen[0], max[1], max[2]);
  poly_count[3] = get_polygons_in_box(cmin[3], cmax[3], polycount, poly, verts, &children[3]);

  set(cmin[4], cen[0], min[1], min[2]);
  set(cmax[4], max[0], cen[1], cen[2]);
  poly_count[4] = get_polygons_in_box(cmin[4], cmax[4], polycount, poly, verts, &children[4]);

  set(cmin[5], cen[0], min[1], cen[2]);
  set(cmax[5], max[0], cen[1], max[2]);
  poly_count[5] = get_polygons_in_box(cmin[5], cmax[5], polycount, poly, verts, &children[5]);

  set(cmin[6], cen[0], cen[1], min[2]);
  set(cmax[6], max[0], max[1], cen[2]);
  poly_count[6] = get_polygons_in_box(cmin[6], cmax[6], polycount, poly, verts, &children[6]);

  set(cmin[7], cen[0], cen[1], cen[2]);
  set(cmax[7], max[0], max[1], max[2]);
  poly_count[7] = get_polygons_in_box(cmin[7], cmax[7], polycount, poly, verts, &children[7]);

  cont = 0;
  for(i = 0; i < 8; ++i)
    if(poly_count[i] > 0)
      ++cont;

  if(cont > 1)
    cont = 1;
  else
    cont = 0;

  for(i = 0; i < 8; ++i) {
    if(!cont || poly_count[i] < MIN_TRIS)
      octree_add_end(obj, cmin[i], cmax[i], i, poly_count[i], children[i]);
    else
      octree_split(octree_add_node(obj, cmin[i], cmax[i], i), poly_count[i], children[i], verts, level + 1);

    // j += poly_count[i];

    free(children[i]);
  }

  //if(j < polycount)
  //  printf("Children have less polygons than the root. Oh dear.\n");
}

void octree_print(octree_container *obj, int level) {
  char i, j;
  /* for(j = 0; j < level - 1; ++j)
    printf("\t");
    printf("Nodes are at %p\n", obj->nodes); */
  for(i = 0; i < 8; ++i) {
    for(j = 0; j < level - 1; ++j)
      printf("\t");
    if(obj->ntypes & (1 << i)) {
      printf("Level %i: #%i, Container.\n", level, i + 1);
      octree_print((octree_container *) obj->nodes[i], level + 1);
    }
    else
      printf("Level %i: #%i, End has %i polys.\n", level, i + 1, ((octree_end *) obj->nodes[i])->polycount);
  }
}

int octree_init(octree_container *obj, ZoneMesh *mesh) {
  obj->ntypes = 0;
  octree_find_min_max(obj, mesh->polygonCount, mesh->poly, mesh->verti);
  octree_split(obj, mesh->polygonCount, mesh->poly, mesh->verti, 0);
  // octree_print(obj, 1);
}
