#ifndef __EQCLIENT_S3D_H_
#define __EQCLIENT_S3D_H_
#include <stdio.h>
#include "types.h"

struct struct_header {
  uint32 offset;
  char magicCookie[4];
  uint32 unknown;
} typedef struct_header;

struct struct_directory_header {
  uint32 count;
} typedef struct_directory_header;

struct struct_directory {
  uint32 crc, offset, size;
} typedef struct_directory;

struct struct_data_block {
  uint32 deflen, inflen;
} typedef struct_data_block;

struct struct_fn_header {
  uint32 fncount;
} typedef struct_fn_header;

struct struct_fn_entry {
  uint32 fnlen;
} typedef struct_fn_entry;

struct s3d_object {
  FILE *fp;
  long count;
  char **filenames;
  uint32 *files;
} typedef s3d_object;

int S3D_Init(s3d_object *obj, FILE *fp);
size_t S3D_GetFile(s3d_object *obj, char *filename, uchar **out);

#endif
