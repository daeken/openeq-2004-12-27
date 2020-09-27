#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "move.h"
#include "draw.h"
#include "matrix.h"

Uint32 lastpoke = 0;
int keystate = 0, laststate = 0;
Uint32 keydowntime[6];
GLfloat movefactor = 0.1f, fastfactor = 0.75f;

inline void cross_product(Vertex *v1, Vertex *v2, Vertex *v3) {
  v3->x = v1->y * v2->z - v1->z * v2->y;
  v3->y = v1->z * v2->x - v1->x * v2->z;
  v3->z = v1->x * v2->y - v1->y * v2->x;
}

inline int dot_product(Vertex *v1, Vertex *v2) {
  return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

#ifdef USE_OCTREE

#define NODE_CHECK(i) \
if(in_box((octree_container *) obj->nodes[i], x, y, z)) {\
  if(obj->ntypes & (1 << i)) {\
    end = find_node((octree_container *) obj->nodes[i], x, y, z); \
    if(end) \
      return end; \
  } \
  else if(((octree_end *)obj->nodes[i])->polycount > 0) \
    return (octree_end *) obj->nodes[i]; \
}

octree_end *find_node(octree_container *obj, double x, double y, double z) {
  octree_end *end;
  if(!in_box(obj, x, y, z))
    return 0;
  NODE_CHECK(0);
  NODE_CHECK(1);
  NODE_CHECK(2);
  NODE_CHECK(3);
  NODE_CHECK(4);
  NODE_CHECK(5);
  NODE_CHECK(6);
  NODE_CHECK(7);
  return 0;
}

#define do_min_max(i, l, j) \
if(min[j] > p[i]->l) min[j] = p[i]->l; \
if(max[j] < p[i]->l) max[j] = p[i]->l;

inline char drop_to_ground(draw_object *obj) {
  Vertex **verts = obj->mesh->verti;
  Vertex *p[3];
  Poly *poly;
  octree_end *end;
  double x, y, z, newz, dist, newdist;
  double min[2], max[2];
  int i;
  char j;

  x = -trans[0];
  y = -trans[1];
  z = -trans[2];
  newz = 0xDEADBEEF;

  end = find_node(&obj->octree, x, y, z);
  if(!end) return;

  for(i = 0; i < end->polycount; ++i) {
    poly = end->poly[i];
    p[0] = verts[poly->v1];
    p[1] = verts[poly->v2];
    p[2] = verts[poly->v3];

    min[0] = min[1] = max[0] = max[1] = 0;

    do_min_max(0, x, 0);
    do_min_max(1, x, 0);
    do_min_max(2, x, 0);

    do_min_max(0, y, 1);
    do_min_max(1, y, 1);
    do_min_max(2, y, 1);

    if(min[0] > x || max[0] < x) continue;
    if(min[1] > y || max[1] < y) continue;

    newdist = abs(z - p[0]->z);
    if(newz == 0xDEADBEEF || newdist < dist) {
      dist = newdist;
      newz = p[0]->z;
    }
  }
  if(newz != 0xDEADBEEF) {
    trans[2] = -newz;
    return 1;
  }
  return 0;
}
#endif

// Update key timers as time passes. Can be called anytime.
inline void pokemove() {
  int changed = keystate ^ laststate;
  int i;
  Uint32 tick;

  if(SDL_GetTicks() < lastpoke)
    lastpoke = SDL_GetTicks();
  tick = SDL_GetTicks() - lastpoke;

  for(i = 0;i < 6;i++) {
    int pos = 1 << i;
    if(!(keystate & pos))
      continue;
    if(!(changed & pos))
      keydowntime[i] += tick; // If the key was down before, add time
  }

  laststate = keystate;
  lastpoke += tick; // Don't miss any during function call
}

// Update camera position and orientation
inline void domove(draw_object *obj) {
  GLfloat off[3] = {0, 0, 0};
  GLfloat transform[16], speed;
  int i;

  speed =  SDL_GetModState() & KMOD_LSHIFT ? fastfactor : movefactor;
  off[0] += keydowntime[KEY_LEFT] * speed;
  off[0] -= keydowntime[KEY_RIGHT] * speed;
  off[1] += keydowntime[KEY_DOWN] * speed;
  off[1] -= keydowntime[KEY_UP] * speed;

  for(i = 0;i < 6;i++)
    keydowntime[i] = 0.0f;

  if(off[0] == 0.0f && off[1] == 0.0f && off[2] == 0.0f)
    return;

  matIdentity(transform);
  matRotateZ(transform, rot[0]);
  matRotateX(transform, -rot[1]);
  matTransform(transform, off);

  trans[0] += off[0];
  trans[1] += off[1];

  /* #ifdef USE_OCTREE
  if(!drop_to_ground(obj))
  #endif */
    trans[2] += off[2];
}

