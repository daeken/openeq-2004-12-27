#include <math.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glext.h>
#ifdef HAVE_GD
#include <gd.h>
#endif
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "wld.h"
#include "draw.h"
#include "ddslib.h"
#include "move.h"
#include "ui.h"

int RANGE = 10;
#define RANGE_CHECK(p1, p2) (((long) ((p1) - (p2)) >> RANGE) && ((long) ((p2) - (p1)) >> RANGE))

SDL_Surface *screen;

float trans[3];
float rot[3];
short int width, height, mousehold = 1;
extern float start_x, start_y, start_z;
extern int screen_shot;
extern char *zone;

uchar collide_tex[4] = {255, 255, 255, 0};

// From a snippet on steinsoft.net
void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan(fovy * M_PI / 360.0);
   ymin = -ymax;
   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void Setup(int width, int height) {
  float ratio = (float) width / (float) height;
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearDepth(1.0);
  // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glEnable(GL_TEXTURE_2D);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glAlphaFunc(GL_EQUAL, 1);
  glEnable(GL_ALPHA_TEST);
  glShadeModel(GL_SMOOTH);
  // glCullFace(GL_BACK);
  // glEnable(GL_CULL_FACE);

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (float) width / (float) height, 0.01, 10000.0);
            
  glMatrixMode(GL_MODELVIEW);  

}

#ifdef TEXTURE_GROUPING
inline int reorder_polys(draw_object *obj) {
  int i, j;

  ZoneMesh *mesh = obj->mesh;

  // Allocate array of polygon groups and distribution count array
  Poly ***newpolys = (Poly ***) malloc(mesh->tex->count * sizeof(Poly **));
  int *numpolys = (int *) malloc(mesh->tex->count * sizeof(int));

  memset(numpolys, 0, mesh->tex->count * sizeof(int));
  
  // Find distribution of material types on polygons
  for(i = 0; i < mesh->polygonCount; ++i)
    ++numpolys[mesh->poly[i]->tex];

  // Create polygon arrays for each material based on distribution
  for(i = 0; i < mesh->tex->count; ++i) {
    newpolys[i] = (Poly **) malloc(numpolys[i] * sizeof(Poly *));
    numpolys[i] = 0;
  }

  // Put polygons in their appropriate place in the polygon-material arrays
  for(i = 0; i < mesh->polygonCount; ++i)
    newpolys[mesh->poly[i]->tex][numpolys[mesh->poly[i]->tex]++] = mesh->poly[i];

  // Put polygons, based on order of materials, back into the mesh
  for(i = 0; i < mesh->tex->count; ++i)
    for(j = 0; j < numpolys[i]; ++j)
      mesh->poly[i] = newpolys[i][j];

  // Free memory
  for(i = 0; i < mesh->tex->count; ++i)
    free(newpolys[i]);
  free(numpolys);
  free(newpolys);
}
#endif

#ifdef USE_OCTREE

#define SQ(a) ((a) * (a))

#define RIGHT 0
#define LEFT 1
#define BOTTOM 2
#define TOP 3
#define BACK 4
#define FRONT 5

float frustum[6][4];

// Much of this culling code is derived from octree part 2 at gametutorials.com.  Their octree code sucks, but at least their culling is decent.

inline void normalize_plane(int side) {
  float magnitude = (float) sqrt(SQ(frustum[side][0]) + SQ(frustum[side][1]) + SQ(frustum[side][2]));
  frustum[side][0] /= magnitude;
  frustum[side][1] /= magnitude;
  frustum[side][2] /= magnitude;
  frustum[side][3] /= magnitude;
}

inline calculate_frustum() {
  float proj[16], modl[16], clip[16];

  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  glGetFloatv(GL_MODELVIEW_MATRIX, modl);

  clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
  clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
  clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
  clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

  clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
  clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
  clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
  clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

  clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
  clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
  clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
  clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

  clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
  clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
  clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
  clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

  frustum[RIGHT][0] = clip[ 3] - clip[ 0];
  frustum[RIGHT][1] = clip[ 7] - clip[ 4];
  frustum[RIGHT][2] = clip[11] - clip[ 8];
  frustum[RIGHT][3] = clip[15] - clip[12];

  frustum[LEFT][0] = clip[ 3] + clip[ 0];
  frustum[LEFT][1] = clip[ 7] + clip[ 4];
  frustum[LEFT][2] = clip[11] + clip[ 8];
  frustum[LEFT][3] = clip[15] + clip[12];

  frustum[BOTTOM][0] = clip[ 3] + clip[ 1];
  frustum[BOTTOM][1] = clip[ 7] + clip[ 5];
  frustum[BOTTOM][2] = clip[11] + clip[ 9];
  frustum[BOTTOM][3] = clip[15] + clip[13];

  frustum[TOP][0] = clip[ 3] - clip[ 1];
  frustum[TOP][1] = clip[ 7] - clip[ 5];
  frustum[TOP][2] = clip[11] - clip[ 9];
  frustum[TOP][3] = clip[15] - clip[13];

  frustum[BACK][0] = clip[ 3] - clip[ 2];
  frustum[BACK][1] = clip[ 7] - clip[ 6];
  frustum[BACK][2] = clip[11] - clip[10];
  frustum[BACK][3] = clip[15] - clip[14];

  frustum[FRONT][0] = clip[ 3] + clip[ 2];
  frustum[FRONT][1] = clip[ 7] + clip[ 6];
  frustum[FRONT][2] = clip[11] + clip[10];
  frustum[FRONT][3] = clip[15] + clip[14];

  normalize_plane(RIGHT);
  normalize_plane(LEFT);
  normalize_plane(BOTTOM);
  normalize_plane(TOP);
  normalize_plane(BACK);
  normalize_plane(FRONT);
}

inline char point_in_frustum(double x, double y, double z) {
  int i;
  for(i = 0; i < 6; ++i) {
    if(frustum[i][0] * x + frustum[i][1] * y + frustum[i][2] * z + frustum[i][3] <= 0)
      return 0;
  }
  return 1;
}

#define ret_if_point(x, y, z) if(point_in_frustum(x, y, z)) return 1
#define ret_if_frust(i, sa, sb, sc) (frustum[i][0] * (x sa size) + frustum[i][1] * (y sb size) + frustum[i][2] * (z sc size) + frustum[i][3] > 0)

#define check_frust(i) \
( \
  ret_if_frust(i, -, -, -) || \
  ret_if_frust(i, +, -, -) || \
  ret_if_frust(i, -, +, -) || \
  ret_if_frust(i, +, +, -) || \
  ret_if_frust(i, -, -, +) || \
  ret_if_frust(i, +, -, +) || \
  ret_if_frust(i, -, +, +) || \
  ret_if_frust(i, +, +, +) \
)

inline char in_frustum(octree_container *obj) {
  double x, y, z, size;
  char i;

  if(in_box(obj, -trans[0], -trans[1], -trans[2])) return 1;

  x = obj->x;
  y = obj->y;
  z = obj->z;
  size = obj->size;

  if(
     check_frust(0) &&
     check_frust(1) &&
     check_frust(2) &&
     check_frust(3) &&
     check_frust(4) &&
     check_frust(5)
     )
    return 1;

  return 0;
}

inline void render_list(draw_object *obj, octree_end *end) {
  ZoneMesh *mesh = obj->mesh;
  int i;
  /* at */ long last = -1;

  if(!end->polycount)
    return;

#ifdef USE_DISPLAY_LIST

  if(end->li != 0xDEADBEEF) {
    glCallList(end->li);
    return;
  }

  end->li = glGenLists(1);
  glNewList(end->li, GL_COMPILE);

#endif

  for(i = 0; i < end->polycount; ++i) {
    if(last != end->poly[i]->tex) {
      last = end->poly[i]->tex;
      glBindTexture(GL_TEXTURE_2D, obj->textures[end->poly[i]->tex]);
    }
    
    glBegin(GL_TRIANGLES);
    
    glTexCoord2f(mesh->verti[end->poly[i]->v1]->u, mesh->verti[end->poly[i]->v1]->v);
    glVertex3f(mesh->verti[end->poly[i]->v1]->x, mesh->verti[end->poly[i]->v1]->y, mesh->verti[end->poly[i]->v1]->z);
    
    glTexCoord2f(mesh->verti[end->poly[i]->v2]->u, mesh->verti[end->poly[i]->v2]->v);
    glVertex3f(mesh->verti[end->poly[i]->v2]->x, mesh->verti[end->poly[i]->v2]->y, mesh->verti[end->poly[i]->v2]->z);
    
    glTexCoord2f(mesh->verti[end->poly[i]->v3]->u, mesh->verti[end->poly[i]->v3]->v);
    glVertex3f(mesh->verti[end->poly[i]->v3]->x, mesh->verti[end->poly[i]->v3]->y, mesh->verti[end->poly[i]->v3]->z);
    
    glEnd();
  }

#ifdef USE_DISPLAY_LIST
  glEndList();
#endif
}

#define NODE_CHECK(i) \
if(in_frustum(cont)) { \
  if(cont->ntypes & (1 << i)) \
    recursive_render(obj, (octree_container *) cont->nodes[i], level + 1); \
  else \
    render_list(obj, (octree_end *) cont->nodes[i]); \
}

inline void recursive_render(draw_object *obj, octree_container *cont, char level) {
  NODE_CHECK(0);
  NODE_CHECK(1);
  NODE_CHECK(2);
  NODE_CHECK(3);
  NODE_CHECK(4);
  NODE_CHECK(5);
  NODE_CHECK(6);
  NODE_CHECK(7);
}

inline int render(draw_object *obj) {
  calculate_frustum();
  recursive_render(obj, &obj->octree, 0);
}

#else

inline int render(draw_object *obj) {

#ifdef USE_DISPLAY_LIST
  static GLuint dl = 0xDEADBEEF;
#endif

  int i, j = 0;
  /* at */ long last = -1;

  ZoneMesh *mesh = obj->mesh;

#ifdef USE_DISPLAY_LIST
  if(dl == 0xDEADBEEF) {
    dl = glGenLists(1);
    glNewList(dl, GL_COMPILE);
#endif

    for(i = 0; i < mesh->polygonCount; ++i) {
#ifdef BINARY_CULL
      if(RANGE_CHECK(trans[0], -mesh->verti[mesh->poly[i]->v1]->x) || RANGE_CHECK(trans[1], -mesh->verti[mesh->poly[i]->v1]->y))
        continue;
#endif
      
      if(last != mesh->poly[i]->tex) {
        last = mesh->poly[i]->tex;
        glBindTexture(GL_TEXTURE_2D, obj->textures[mesh->poly[i]->tex]);
      }
      
      glBegin(GL_TRIANGLES);
      
      glTexCoord2f(mesh->verti[mesh->poly[i]->v1]->u, mesh->verti[mesh->poly[i]->v1]->v);
      glVertex3f(mesh->verti[mesh->poly[i]->v1]->x, mesh->verti[mesh->poly[i]->v1]->y, mesh->verti[mesh->poly[i]->v1]->z);
      
      glTexCoord2f(mesh->verti[mesh->poly[i]->v2]->u, mesh->verti[mesh->poly[i]->v2]->v);
      glVertex3f(mesh->verti[mesh->poly[i]->v2]->x, mesh->verti[mesh->poly[i]->v2]->y, mesh->verti[mesh->poly[i]->v2]->z);
      
      glTexCoord2f(mesh->verti[mesh->poly[i]->v3]->u, mesh->verti[mesh->poly[i]->v3]->v);
      glVertex3f(mesh->verti[mesh->poly[i]->v3]->x, mesh->verti[mesh->poly[i]->v3]->y, mesh->verti[mesh->poly[i]->v3]->z);
      
      glEnd();
      ++j;
    }

#ifdef USE_DISPLAY_LIST
    glEndList();
  }
  else {
    glCallList(dl);
  }
#endif

  // printf("Rendered %u triangles.\n", j);
  return 0;
}

#endif

int display(draw_object *obj) {
  int i, j;
  Mesh *obj_mesh;
  Placeable_LL *cur;

  pokemove();
  domove(obj);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);

  glPushMatrix();
  glLoadIdentity();

  glRotatef(270, 1.0, 0.0, 0.0);
  glRotatef(rot[1], 1.0, 0.0, 0.0);
  glRotatef(rot[0], 0.0, 0.0, 1.0);
  glTranslatef(trans[0], trans[1], trans[2]);

  render(obj);

  glPopMatrix();

  /*
  while(cur) {
    last = -1;

#ifdef BINARY_CULL
    if(RANGE_CHECK(trans[0], -cur->obj->trans[0]) || RANGE_CHECK(trans[1], -cur->obj->trans[1])) {
      cur = cur->next;
      continue;
    }
#endif

    glPushMatrix();
    glLoadIdentity();
    glRotatef(270, 1.0, 0.0, 0.0);
    glRotatef(rot[1], 1.0, 0.0, 0.0);
    glRotatef(rot[0], 0.0, 0.0, 1.0);
    glTranslatef(trans[0], trans[1], trans[2]);
    glTranslatef(cur->obj->trans[0], cur->obj->trans[1], cur->obj->trans[2]);
    glRotatef(cur->obj->rot[0], 1.0, 0.0, 0.0);
    glRotatef(cur->obj->rot[1], 0.0, 1.0, 0.0);
    glRotatef(cur->obj->rot[2], 0.0, 0.0, 1.0);
    glScalef(cur->obj->scale[0], cur->obj->scale[1], 1);

    obj_mesh = cur->obj->mesh;
    
    for(i = 0; i < obj_mesh->polygonCount; ++i) {
      if(last != obj_mesh->poly[i].tex) {
        last = obj_mesh->poly[i].tex;
        glBindTexture(GL_TEXTURE_2D, obj_mesh->tex->textures[obj_mesh->poly[i].tex]);
      }
      
      glBegin(GL_TRIANGLES);

      glTexCoord2f(obj_mesh->verti[obj_mesh->poly[i].v1].u, obj_mesh->verti[obj_mesh->poly[i].v1].v);
      glVertex3f(obj_mesh->verti[obj_mesh->poly[i].v1].x, obj_mesh->verti[obj_mesh->poly[i].v1].y, obj_mesh->verti[obj_mesh->poly[i].v1].z);
      
      glTexCoord2f(obj_mesh->verti[obj_mesh->poly[i].v2].u, obj_mesh->verti[obj_mesh->poly[i].v2].v);
      glVertex3f(obj_mesh->verti[obj_mesh->poly[i].v2].x, obj_mesh->verti[obj_mesh->poly[i].v2].y, obj_mesh->verti[obj_mesh->poly[i].v2].z);
      
      glTexCoord2f(obj_mesh->verti[obj_mesh->poly[i].v3].u, obj_mesh->verti[obj_mesh->poly[i].v3].v);
      glVertex3f(obj_mesh->verti[obj_mesh->poly[i].v3].x, obj_mesh->verti[obj_mesh->poly[i].v3].y, obj_mesh->verti[obj_mesh->poly[i].v3].z);
      
      glEnd();
    }
    glPopMatrix();
    cur = cur->next;
  }
  */

  // ui_draw(obj->ui);

  glMatrixMode(GL_PROJECTION);
  SDL_GL_SwapBuffers();

  return 0;
}

void mainloop(draw_object *obj) {
  SDL_Event event;
  char run = 1;
  char i;
  for(i = 0; i < 6; i++)
    keydowntime[i] = 0.0f;
  while(run) {
    if (!keystate) {
      if (SDL_WaitEvent(&event)) {
      	if(Event(obj, event)) {
          run = 0;
        }
      }
    }
    while(SDL_PollEvent(&event)) {
      if(Event(obj, event)) {
        run = 0;
        break;
      }
    }
    if(run)
      display(obj);
  }
}

inline int Event(draw_object *obj, SDL_Event event) {
  int hw = width >> 1;
  int hh = height >> 1;
  static char mouselook = 0;
  switch(event.type) {
  case SDL_MOUSEMOTION:
    if(!mouselook && ui_in(obj->ui, event.motion.x, height - event.motion.y)) {
      if(!event.motion.state)
        ui_mouse(obj->ui, event.motion.x, height - event.motion.y, 0, 0, event.motion.state);
      else
        ui_mouse(obj->ui, event.motion.x, height - event.motion.y, event.motion.xrel, event.motion.yrel, event.motion.state);
    }
    else if(mouselook && !(event.motion.x == hw && event.motion.y == hh)) {
      rot[0] += (event.motion.x - hw) * 0.125f;
      rot[1] += (event.motion.y - hh) * 0.125f;
      if(rot[0] < 0.0f)
        rot[0] += 360.0f;
      if(rot[0] > 360.0f)
        rot[0] -= 360.0f;
      if(rot[1] <= -90.0f)
        rot[1] = -90.0f;
      if(rot[1] >= 90.0f)
        rot[1] = 90.0f;
      SDL_WarpMouse(hw, hh);
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
    if(event.button.button == SDL_BUTTON_LEFT) {
      if(ui_in(obj->ui, event.button.x, height - event.button.y))
        mouselook = 0;
      else {
        mouselook = 1;
        SDL_ShowCursor(SDL_DISABLE);
      }
    }
    break;
  case SDL_MOUSEBUTTONUP:
    if(event.button.button == SDL_BUTTON_LEFT) {
      ui_in(obj->ui, event.button.x, height - event.button.y);
      if(ui_in(obj->ui, event.button.x, height - event.button.y)) {
        ui_click(obj->ui, null, event.button.x, height - event.button.y, event.button.state, null);
      }
      mouselook = 0;
      SDL_ShowCursor(SDL_ENABLE);
    }
    break;
  case SDL_KEYDOWN:
    switch(event.key.keysym.sym) {
    case SDLK_UP:
    case SDLK_w:
      keystate |= KS_UP;
      break;
    case SDLK_DOWN:
    case SDLK_s:
      keystate |= KS_DOWN;
      break;
    case SDLK_LEFT:
    case SDLK_a:
      keystate |= KS_LEFT;
      break;
    case SDLK_RIGHT:
    case SDLK_d:
      keystate |= KS_RIGHT;
      break;
    case SDLK_F1:
    case SDLK_LCTRL:
      keystate |= KS_VDN;
      break;
    case SDLK_F2:
    case SDLK_SPACE:
      keystate |= KS_VUP;
      break;
    case SDLK_l:
    	printf("You are at: %f,%f,%f\n",trans[0],trans[1],trans[2]);
      break;
    case SDLK_KP_MINUS:
#ifdef HAVE_GD
    	printf("Taking a screenshot...\n");
      dump_png();
#else
    	printf("Cannot take screenshot without GD support.\n");
#endif
      break;
    case SDLK_q:
    case SDLK_ESCAPE:
      SDL_Quit();
      return 1;
      }
    pokemove();
    break;
  case SDL_KEYUP:
    switch(event.key.keysym.sym) {
    case SDLK_UP:
    case SDLK_w:
      keystate &= ~KS_UP;
      break;
    case SDLK_DOWN:
    case SDLK_s:
      keystate &= ~KS_DOWN;
      break;
    case SDLK_LEFT:
    case SDLK_a:
      keystate &= ~KS_LEFT;
	  break;
    case SDLK_RIGHT:
    case SDLK_d:
      keystate &= ~KS_RIGHT;
      break;
    case SDLK_F1:
    case SDLK_LCTRL:
      // keystate &= ~KS_VDN;
      RANGE -= 1;
      break;
    case SDLK_F2:
    case SDLK_SPACE:
      // keystate &= ~KS_VUP;
      RANGE += 1;
      break;
    }
    break;
  case SDL_QUIT:
    SDL_Quit();
    return 1;
  }
  
  return 0;
}

int Draw(ZoneMesh *mesh, s3d_object *s3d) {
  const SDL_VideoInfo *info = NULL;
  int w, h;
  char bpp;
  draw_object obj;
  int i, j, texcount, curtex;
  uchar *buf;
  uchar *texdata;
  char **texnames, found;
  GLuint *texes;
  int size;
  ddsPF_t pf;

  obj.mesh = mesh;

  obj.ui = (ui_object *) malloc(sizeof(ui_object));

  ui_init(obj.ui);

#ifdef USE_OCTREE
  octree_init(&obj.octree, mesh);
#endif

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    printf("Could not initialize SDL subsystems.\n");
    return 1;
  }
  info = SDL_GetVideoInfo();
  width = 1280;
  height = 1024;
  bpp = info->vfmt->BitsPerPixel;
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
  screen = SDL_SetVideoMode(width, height, bpp, SDL_OPENGL | SDL_GL_DOUBLEBUFFER);

  rot[0] = 315;
  rot[1] = rot[2] = 0;
  trans[0] = start_x;
  trans[1] = start_y;
  trans[2] = start_z;

  obj.textures = (GLuint *) malloc(sizeof(GLuint) * mesh->tex->count);

  glGenTextures(mesh->tex->count, obj.textures);
  
  for(i = 0; i < mesh->tex->count; ++i) {
    if(!strcmp(mesh->tex->filenames[i], "collide.dds")) {
      glBindTexture(GL_TEXTURE_2D, obj.textures[i]);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexImage2D(GL_TEXTURE_2D, 0, 4, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, collide_tex);
      continue;
    }
    size = S3D_GetFile(s3d, mesh->tex->filenames[i], &buf);
    DDSGetInfo((ddsBuffer_t *) buf, &w, &h, &pf);
    texdata = (uchar *) malloc(w * h * 4);
    DDSDecompress((ddsBuffer_t *) buf, texdata, size);
    free(buf);

    glBindTexture(GL_TEXTURE_2D, obj.textures[i]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
    
    free(texdata);
  }

#ifdef TEXTURE_GROUPING
  reorder_polys(&obj);
#endif

  Setup(width, height);

  display(&obj);
  if (!screen_shot)
  	mainloop(&obj);
  else
  	dump_png();
}

int dump_png(void) {
#ifdef HAVE_GD
  int i,j;
  FILE *fp;
  char *file;
  unsigned char *image;
  gdImagePtr im;
  im=gdImageCreateTrueColor(width,height);
  static int count=0;

  /* Allocate our buffer for the image */
  if ((image = (unsigned char *)malloc(3*width*height*sizeof(unsigned char))) == NULL) {
    fprintf(stderr,"Failed to allocate memory for glReadPixels\n");
    return 0;
  }

  glPixelStorei(GL_PACK_ALIGNMENT,1);

  /* Copy the image into our buffer */
  glReadBuffer(GL_BACK_LEFT);
  glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

  /* Write the raw file */
  for (j=height-1;j>=0;j--) {
    for (i=0;i<width;i++) {
      gdImageSetPixel(im,i,height-j,gdImageColorAllocate(im,image[3*j*width+3*i+0], image[3*j*width+3*i+1], image[3*j*width+3*i+2]));
    }
  }

  file = (char *) malloc(strlen(zone) + 9);
  sprintf(file,"%s-%03d.jpg",zone,count++);
  if ((fp=fopen(file,"wb"))!=NULL) {
  	gdImageJpeg(im, fp, -1);
  	fclose(fp);
  }
  free(file);

  gdImageDestroy(im);
  free(image);
#endif
  return 1;
}
