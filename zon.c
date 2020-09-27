#include "zon.h"

int ZON_init(zon_object *obj, uchar *zon_buf) {
  int ter_ = htonl(0x5445525f);
  int lit_ = htonl(0x4c49545f);
  int obj_ = htonl(0x4f424a5f);
  uchar *zon_orig = zon_buf;
  zon_header *hdr = (zon_header *) zon_buf;
  zon_placeable *pla;
  int i;

  zon_buf += sizeof(zon_header);
  obj->ter_name = 0;

  while(zon_buf - zon_orig < hdr->list_len) {
    if(*((long *) zon_buf) == ter_ && obj->ter_name == 0) {
      obj->ter_name = (char *) malloc(strlen(zon_buf) + 1);
      memcpy(obj->ter_name, zon_buf, strlen(zon_buf) + 1);
    }
    /*    if(*((long *) zon_buf) == obj_) {
      if(zon_buf[strlen(zon_buf) - 4] != '.')
        printf("%s\n", zon_buf);
        } */
    zon_buf += strlen(zon_buf) + 1;
  }

  // printf("%i\n", hdr->obj_count);
  zon_buf = zon_orig - 1 - (sizeof(zon_placeable) * hdr->obj_count);
  for(i = 0; i < hdr->obj_count; ++i) {
    pla = (zon_placeable *) zon_buf;
    zon_buf += sizeof(zon_placeable);
  }
  return 0;
}
