#ifndef __EQCLIENT_UI_H_
#define __EQCLIENT_UI_H_
#include "draw.h"
#include "types.h"

enum widget_type {
  WIDGET_NONE,
  WIDGET_WINDOW,
  WIDGET_BUTTON
};

#define ATTR_TRANS 1
#define ATTR_COLOR 2

struct attributes {
  float r, g, b; // FG
  float bgr, bgg, bgb, bga; // BG
  float br, bg, bb; // Border
  float orig_a;
  uchar flags;
  uchar in;
} typedef attributes;

struct widget_LL {
  enum widget_type type;
  void *widget;
  struct widget_LL *children, *children_last;
  struct widget_LL *next, *last;
} typedef widget_LL;

struct ui_object {
  widget_LL *children, *children_last;
} typedef ui_object;

struct window_object {
  short int x, y, w, h;
  attributes attr;
} typedef window_object;

struct button_object {
  short int x, y, w, h;
  void (*callback)(ui_object *, struct button_object *);
  char *text;
  attributes attr;
} typedef button_object;

void ui_init(ui_object *obj);
void ui_draw(ui_object *obj);
void ui_mouse(ui_object *obj, short x, short y, char relx, char rely, uchar state);
void ui_click(ui_object *obj, widget_LL *ll, short x, short y, uchar state, void *parent);

#endif
