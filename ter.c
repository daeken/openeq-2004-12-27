#include "s3d.h"
#include "wld.h"
#include "ter.h"
#include <stdio.h>

#define sq(i) ((i) * (i))

inline void set_face_normal(Poly *poly, Vertex **verti) {
  double v0[3], v1[3];
  double mag0, mag1;
  Vertex *p[3];

  p[0] = verti[poly->v1];
  p[1] = verti[poly->v2];
  p[2] = verti[poly->v3];

  v0[0] = p[0]->x - p[1]->x;
  v0[1] = p[0]->y - p[1]->y;
  v0[2] = p[0]->z - p[1]->z;

  v1[0] = p[1]->x - p[2]->x;
  v1[1] = p[1]->y - p[2]->y;
  v1[2] = p[1]->z - p[2]->z;

  mag0 = sqrt(sq(v0[0]) + sq(v0[1]) + sq(v0[2]));
  mag1 = sqrt(sq(v1[0]) + sq(v1[1]) + sq(v1[2]));

  v0[0] /= mag0;
  v1[0] /= mag1;
  v0[1] /= mag0;
  v1[1] /= mag1;
  v0[2] /= mag0;
  v1[2] /= mag1;

  poly->normal[0] = (v0[1] * v1[2]) - (v0[2] * v1[1]);
  poly->normal[1] = (v0[2] * v1[0]) - (v0[0] * v1[2]);
  poly->normal[2] = (v0[0] * v1[1]) - (v0[1] * v1[0]);
}

int TER_Init(ter_object *obj, uchar *ter, int ter_size, s3d_object *s3d) {
  ter_header *thdr = (ter_header *) ter;
  ter_vertex *tver;
  ter_triangle *ttri;
  material *mlist = (material *) malloc(sizeof(material) * thdr->mat_count);
  char var_len, val_len, mi = -1;
  char *var, *val;
  uchar *ter_tmp, *ter_orig;
  int i, j;

  ter_orig = ter;
  ter += sizeof(ter_header);
  ter_tmp = ter + thdr->list_len;

  if(sizeof(ter_header) + thdr->list_len + (thdr->vert_count * sizeof(ter_vertex)) + (thdr->tri_count * sizeof(ter_triangle)) > ter_size)
    return 0;
  j = 0;
  while(ter < ter_tmp) {
    var_len = strlen(ter);
    var = ter;
    ter += var_len + 1;
    if(var[0] == 'e' && var[1] == '_') {
      val_len = strlen(ter);
      val = ter;
      ter += val_len + 1;
      if(!mlist[mi].basetex) {
        mlist[mi].basetex = (char *) malloc(val_len + 1);
        memcpy(mlist[mi].basetex, val, val_len + 1);
      }

      ++mlist[mi].var_count;
      mlist[mi].var_names = (char **) realloc(mlist[mi].var_names, mlist[mi].var_count * sizeof(char *));
      mlist[mi].var_vals = (char **) realloc(mlist[mi].var_vals, mlist[mi].var_count * sizeof(char *));
      mlist[mi].var_names[mlist[mi].var_count - 1] = (char *) malloc(var_len + 1);
      memcpy(mlist[mi].var_names[mlist[mi].var_count - 1], var, var_len + 1);
      mlist[mi].var_vals[mlist[mi].var_count - 1] = (char *) malloc(val_len + 1);
      memcpy(mlist[mi].var_vals[mlist[mi].var_count - 1], val, val_len + 1);
    }
    else {
      val_len = strlen(ter);
      val = ter;

      ++mi;
      mlist[mi].var_count = 0;
      mlist[mi].name = (char *) malloc(var_len + 1);
      memcpy(mlist[mi].name, var, var_len + 1);
      mlist[mi].var_names = mlist[mi].var_vals = 0;

      if(val[0] == 'e' && val[1] == '_')
        mlist[mi].basetex = 0;
      else {
        mlist[mi].basetex = (char *) malloc(val_len + 1);
        memcpy(mlist[mi].basetex, val, val_len + 1);
        ter += val_len + 1;
      }
    }

    ++j;
  }

  obj->mesh.polygonCount = thdr->tri_count;
  obj->mesh.vertexCount = thdr->vert_count;
  obj->mesh.verti = (Vertex **) malloc(obj->mesh.vertexCount * sizeof(Vertex *));
  obj->mesh.poly = (Poly **) malloc(obj->mesh.polygonCount * sizeof(Poly *));

  ter = ter_orig + (ter_size - 4 - (thdr->tri_count * 20) - (thdr->vert_count * 32));

  for(i = 0; i < thdr->vert_count; ++i) {
    tver = (ter_vertex *) ter;
    obj->mesh.verti[i] = (Vertex *) malloc(sizeof(Vertex));
    obj->mesh.verti[i]->x = tver->x;
    obj->mesh.verti[i]->y = tver->y;
    obj->mesh.verti[i]->z = tver->z;
    obj->mesh.verti[i]->u = tver->u;
    obj->mesh.verti[i]->v = tver->v;
    ter += sizeof(ter_vertex);
  }

  j = 0;
  for(i = 0; i < thdr->tri_count; ++i) {
    ttri = (ter_triangle *) ter;
    if(ttri->group == -1) {
      ter += sizeof(ter_triangle);
      continue;
    }
    obj->mesh.poly[j] = (Poly *) malloc(sizeof(Poly));
    obj->mesh.poly[j]->v1 = ttri->v1;
    obj->mesh.poly[j]->v2 = ttri->v2;
    obj->mesh.poly[j]->v3 = ttri->v3;
    if(ttri->group == -1)
      obj->mesh.poly[j]->tex = 0;
    else
      obj->mesh.poly[j]->tex = ttri->group;

    set_face_normal(obj->mesh.poly[j], obj->mesh.verti);

    ++j;
    ter += sizeof(ter_triangle);
  }
  obj->mesh.polygonCount = j;

  obj->mesh.tex = (Texture *) malloc(sizeof(Texture));
  obj->mesh.tex->count = thdr->mat_count;
  obj->mesh.tex->filenames = (char **) malloc(sizeof(char *) * (thdr->mat_count));

  for(i = 0; i < thdr->mat_count; ++i)
    obj->mesh.tex->filenames[i] = mlist[i].basetex;
  // obj->mesh.tex->filenames[thdr->mat_count] = "negative.tex";

  return 0;
}
