#include "s3d.h"
#include "wld.h"
// #include "mob.h"

// #define FRAG_DEBUG

#define WORD_A(c) sizeof(short) * (c)
#define LONG_A(c) sizeof(long) * (c)

#define WORD_P(offset) *((short *) (buf + pos + (offset)))
#define LONG_P(offset) *((long *) (buf + pos + (offset)))

uchar encarr[] = {0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A};

inline void decode(uchar *str, int len) { int i; for(i = 0; i < len; ++i) str[i] ^= encarr[i % 8]; }

FRAGMENT_FUNC(Data03) {
  int i, pos, nameLen;
  Texture *tex = (Texture *) malloc(sizeof(Texture));
  tex->count = *((long *) buf) + 1;
  tex->flags = (int *) malloc(sizeof(int) * tex->count);
  tex->filenames = (char **) malloc(sizeof(char *) * tex->count);
  *obj = (void *) tex;

#ifdef FRAG_DEBUG
  printf("Data03: %p %p\n", obj, *obj);
#endif
  
  pos = sizeof(long);
  for(i = 0; i < tex->count; ++i) {
    nameLen = *((short *) (buf + pos));
    tex->filenames[i] = (char *) malloc(nameLen + 1);
    memcpy(tex->filenames[i], (char *) (buf + pos + sizeof(short)), nameLen + 1);
    decode(tex->filenames[i], nameLen);
    Lower(tex->filenames[i]);
  }
  return 0;
}
FRAGMENT_FUNC(Data04) {
  int flags, i, pos;
  Texture *tex = (Texture *) malloc(sizeof(Texture));
  tex->count = *((long *) (buf + sizeof(long)));
  tex->flags = (int *) malloc(sizeof(int) * tex->count);
  tex->filenames = (char **) malloc(sizeof(char *) * tex->count);
  *obj = (void *) tex;

#ifdef FRAG_DEBUG
  printf("Data04: %p %p\n", obj, *obj);
#endif
  
  flags = *((long *) buf);
  pos = sizeof(long) * 2;
  if(flags & (1 << 2))
    pos += sizeof(long);
  if(flags & (1 << 3))
    pos += sizeof(long);

  for(i = 0; i < 1 /* tex->count */; ++i) {
    tex->flags[i] = flags;
    tex->filenames[i] = ((Texture *) wld->frags[*((long *) (buf + pos)) - 1]->frag)->filenames[i];
    pos += sizeof(long);
  }

  return 0;
}

FRAGMENT_FUNC(Data05) {
  *obj = wld->frags[*((long *) buf) - 1]->frag;

#ifdef FRAG_DEBUG
  printf("Data05: %p %p\n", obj, *obj);
#endif

  return 0;
}

FRAGMENT_FUNC(Data15) {
  char *name, *temp;
  Placeable *place = (Placeable *) malloc(sizeof(Placeable));
  struct_Data15 *hdr = (struct_Data15 *) buf;

  if(((signed long) -hdr->ref) < (signed long) 0)
    return -1;

  name = (char *) malloc(strlen(&wld->sHash[-hdr->ref]) - 9 + 1);
  memcpy(name, &wld->sHash[-hdr->ref], strlen(&wld->sHash[-hdr->ref]) - 9);
  name[strlen(&wld->sHash[-hdr->ref]) - 9] = 0;
  temp = (char *) malloc(strlen(name) + 13);
  sprintf(temp, "%s_DMSPRITEDEF", name);

  WLD_GetObjectMesh(wld, temp, &place->mesh);
  if(!place->mesh) {
    printf("Object not found: %s. Original Name: %s. Cut Name: %s\n", temp, &wld->sHash[-hdr->ref], name);
    return 0;
  }

  free(name);
  
  place->trans[0] = hdr->trans[0];
  place->trans[1] = hdr->trans[1];
  place->trans[2] = hdr->trans[2];

  place->rot[0] = hdr->rot[2] / 512.f * 360.f;
  place->rot[1] = hdr->rot[1] / 512.f * 360.f;
  place->rot[2] = hdr->rot[0] / 512.f * 360.f;

  place->scale[0] = hdr->scale[2];
  place->scale[1] = hdr->scale[1];
  place->scale[2] = hdr->scale[0];

  if(wld->placeable == null)
    wld->placeable_cur = wld->placeable = (Placeable_LL *) malloc(sizeof(Placeable_LL));
  else
    wld->placeable_cur = wld->placeable_cur->next = (Placeable_LL *) malloc(sizeof(Placeable_LL));
  wld->placeable_cur->obj = place;
  wld->placeable_cur->next = null;

  return 0;
}

FRAGMENT_FUNC(Data21) {
  struct_Data21 *data;
  long count = *((long *) buf);
  long i;
  BSP_Node *tree = (BSP_Node *) malloc(count * sizeof(BSP_Node));
  
  for(i = 0; i < count; ++i) {
    data = (struct_Data21 *) (buf + i * sizeof(struct_Data21));
    tree[i].normal[0] = data->normal[0];
    tree[i].normal[1] = data->normal[1];
    tree[i].normal[2] = data->normal[2];
    tree[i].splitdistance = data->splitdistance;
    // tree[i].region = (BSP_Region *) wld->frags[data->region]->frag;
    // tree[i].left = &tree[data->node[0]];
    // tree[i].right = &tree[data->node[1]];
  }
  return 0;
}

FRAGMENT_FUNC(Data22) {
  int pos, i, j, temp, weird;
  struct_Data22 *data = (struct_Data22 *) buf;
  uchar *rle;

  if(!wld->loadBSP)
    return -1;

  pos = sizeof(struct_Data22) + (12 * data->size1) + (8 * data->size2);

  return 0;
}

FRAGMENT_FUNC(Data30) {
  Texture *temp;
  int params;
  params = *((long *) (buf + 4));
  if(!params || !*((long *) (buf + 20))) {
    *obj = (void *) malloc(sizeof(Texture));
    temp = (Texture *) *obj;
    temp->count = 1;
    temp->filenames = (char **) malloc(sizeof(char *));
    temp->filenames[0] = "collide.dds";
    temp->flags = (int *) malloc(sizeof(int));
    temp->flags[0] = 0;
  }
  else {
    if(!*((long *) buf))
      *obj = wld->frags[*((long *) (buf + 28)) - 1]->frag;
    else
      *obj = wld->frags[*((long *) (buf + 20)) - 1]->frag;
    ((Texture *) *obj)->params = params;
  }

#ifdef FRAG_DEBUG
  printf("Data30: %p %p\n", obj, *obj);
#endif

  return 0;
}

FRAGMENT_FUNC(Data31) {
  int i, pos;
  Texture *tex = (Texture *) malloc(sizeof(Texture));
  tex->count = *((long *) (buf + sizeof(long)));
  tex->flags = (int *) malloc(sizeof(int) * tex->count);
  tex->filenames = (char **) malloc(sizeof(char *) * tex->count);
  *obj = (void *) tex;

#ifdef FRAG_DEBUG
  printf("Data31: %p %p\n", obj, *obj);
#endif
  
  pos = sizeof(long) * 2;

  for(i = 0; i < tex->count; ++i) {
    tex->flags[i] = ((Texture *) wld->frags[*((long *) (buf + pos)) - 1]->frag)->flags[0];
    tex->filenames[i] = ((Texture *) wld->frags[*((long *) (buf + pos)) - 1]->frag)->filenames[0];
    tex->params = ((Texture *) wld->frags[*((long *) (buf + pos)) - 1]->frag)->params;
    pos += sizeof(long);
  }

  return 0;
}

FRAGMENT_FUNC(Data36) {
  struct_Data36 *header = (struct_Data36 *) buf;
  Vertex *verti = (Vertex *) malloc(header->vertexCount * sizeof(Vertex));
  Poly *poly = (Poly *) malloc(header->polygonsCount * sizeof(Poly));
  Polygon *p;
  TexCoordsNew *tex_new;
  TexCoordsOld *tex_old;
  Vert *v;
  Mesh *mesh = (Mesh *) malloc(sizeof(Mesh));
  int i, j, polyCount, vertCount, texIndex, skinIndex, pc, pos = sizeof(struct_Data36);
  float scale = 1.0f / (float) (1 << header->scale);
  mesh->name = -frag_name;
  mesh->tex = ((Texture *) wld->frags[header->fragment1 - 1]->frag);
  mesh->polygonCount = header->polygonsCount;
  mesh->vertexCount = header->vertexCount;

  for(i = 0; i < header->vertexCount; ++i) {
    v = (Vert *) (buf + pos);
    verti[i].x = header->centerX + v->x * scale;
    verti[i].y = header->centerY + v->y * scale;
    verti[i].z = header->centerZ + v->z * scale;
    pos += sizeof(Vert);
  }

  if(wld->new) {
    for(i = 0; i < header->texCoordsCount; ++i) {
      tex_new = (TexCoordsNew *) (buf + pos);
      verti[i].u = tex_new->tx / 256.0f;
      verti[i].v = tex_new->tz / 256.0f;
      pos += sizeof(TexCoordsNew);
    }
  }
  else {
    for(i = 0; i < header->texCoordsCount; ++i) {
      tex_old = (TexCoordsOld *) (buf + pos);
      verti[i].u = tex_old->tx / 256.0f;
      verti[i].v = tex_old->tz / 256.0f;
      pos += sizeof(TexCoordsOld);
    }
  }

  pos += 3 * header->normalsCount;
  pos += 4 * header->colorCount;

  for(i = 0; i < header->polygonsCount; ++i) {
    p = (Polygon *) (buf + pos);
    poly[i].flags = p->flags;
    poly[i].v1 = p->v1;
    poly[i].v2 = p->v2;
    poly[i].v3 = p->v3;
    pos += sizeof(Polygon);
  }

  pc = 0;
  for(i = 0; i < header->size6; ++i) {
    vertCount = *((short *) (buf + pos));
    skinIndex = *((short *) (buf + pos + 2));
    for(j = 0; j < vertCount; ++j, ++pc) {
      verti[pc].skin = skinIndex;
    }
    pos += 4;
  }

  pc = 0;
  for(i = 0; i < header->polygonTexCount; ++i) {
    polyCount = *((short *) (buf + pos));
    texIndex = *((short *) (buf + pos + 2));
    for(j = 0; j < polyCount; ++j, ++pc)
      poly[pc].tex = texIndex;
    pos += 4;
  }

  mesh->verti = verti;
  mesh->poly = poly;

  *obj = (void *) mesh;

#ifdef FRAG_DEBUG
  printf("Data36: %p %p\n", obj, *obj);
#endif

  return 0;
}

int WLD_Init(wld_object *obj, uchar *wld, s3d_object *s3d, char loadBSP) {
  int pos, i, status;
  void *frag_obj;
  struct_wld_header *header;
  struct_wld_basic_frag *frag;

  obj->wld = wld;
  obj->s3d = s3d;
  header = (struct_wld_header *) wld;
  pos = sizeof(struct_wld_header);
  if(header->magic != 0x54503d02)
    return -1;
  if(header->version == 0x00015500)
    obj->new = 0;
  else
    obj->new = 1;
  obj->sHash = wld + pos;
  decode(obj->sHash, header->stringHashSize);

  obj->frags = (struct_frag **) malloc(sizeof(struct_frag *) * header->fragmentCount);
  obj->fragCount = header->fragmentCount;
  obj->loadBSP = loadBSP;
  obj->placeable = null;

  pos += header->stringHashSize;
  for(i = 0; i < header->fragmentCount; ++i) {
    frag = (struct_wld_basic_frag *) (wld + pos);
    obj->frags[i] = (struct_frag *) malloc(sizeof(struct_frag));
    switch(frag->id) {
    case 0x35: status = -1; break;
    case 0x03: FRAGMENT(Data03); break;
    case 0x04: FRAGMENT(Data04); break;
    case 0x05: FRAGMENT(Data05); break;
    case 0x15: FRAGMENT(Data15); break;
    case 0x21: FRAGMENT(Data21); break;
    case 0x22: FRAGMENT(Data22); break;
    case 0x30: FRAGMENT(Data30); break;
    case 0x31: FRAGMENT(Data31); break;
    case 0x36: FRAGMENT(Data36); break;
    }

#ifdef FRAG_DEBUG
    if(frag->id != 0x35)
      printf("Init: %i - %p\n", i, frag_obj);
#endif

    obj->frags[i]->type = frag->id;
    obj->frags[i]->name = frag->nameRef;
    obj->frags[i]->frag = frag_obj;
    pos += sizeof(struct_wld_basic_frag) + frag->size - 4;
  }
  return 0;
}

int WLD_GetObjectMesh(wld_object *obj, char *name, Mesh **mesh) {
  int i;
  for(i = 0; i < obj->objs->fragCount; ++i) {
    if(obj->objs->frags[i]->type != 0x36)
      continue;
    if(!strcmp(&obj->objs->sHash[-obj->objs->frags[i]->name], name)) {
      *mesh = (Mesh *) obj->objs->frags[i]->frag;
      return 0;
    }
  }
  return -1;
}

int WLD_GetZoneMesh(wld_object *obj, ZoneMesh *mesh) {
  Mesh *temp;
  int i, j;
  int polygonCount = 0, vertexCount = 0;

  for(i = 0; i < obj->fragCount; ++i) {
    if(obj->frags[i]->type != 0x36)
      continue;
    temp = (Mesh *) obj->frags[i]->frag;
    polygonCount += temp->polygonCount;
    vertexCount += temp->vertexCount;
  }

  mesh->polygonCount = polygonCount;
  mesh->vertexCount = vertexCount;
  mesh->poly = (Poly **) malloc(sizeof(Poly *) * polygonCount);
  mesh->verti = (Vertex **) malloc(sizeof(Vertex *) * vertexCount);
  polygonCount = 0;
  vertexCount = 0;

  mesh->tex = temp->tex;
  
  for(i = 0; i < obj->fragCount; ++i) {
    if(obj->frags[i]->type != 0x36)
      continue;
    temp = (Mesh *) obj->frags[i]->frag;
    for(j = 0; j < temp->polygonCount; ++j) {
      mesh->poly[polygonCount + j] = &temp->poly[j];
      mesh->poly[polygonCount + j]->v1 += vertexCount;
      mesh->poly[polygonCount + j]->v2 += vertexCount;
      mesh->poly[polygonCount + j]->v3 += vertexCount;
    }
    for(j = 0; j < temp->vertexCount; ++j) {
      mesh->verti[vertexCount + j] = &temp->verti[j];
    }
    polygonCount += temp->polygonCount;
    vertexCount += temp->vertexCount;
  }
  return 0;
}
