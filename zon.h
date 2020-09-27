#ifndef __EQCLIENT_ZON_H_
#define __EQCLIENT_ZON_H_

#include <stdio.h>
#include "types.h"

struct zon_header {
  char magic[4]; // Constant at EQGZ
  long version; // Constant at 2
  long list_len; // Length of the list to follow.
  long unk[3]; // Unknown.
  long obj_count; // Placeable object count. Thanks jbb!
} typedef zon_header;

struct zon_placeable {
  long loc;
  float y, x, z;
  float rx, ry, rz;
  float scale;
} typedef zon_placeable;

struct zon_object {
  char *ter_name;
} typedef zon_object;

int ZON_init(zon_object *obj, uchar *zon_buf);

#endif
