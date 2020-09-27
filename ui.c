#include <SDL.h>
#include "draw.h"
#include "ui.h"

#define MAKE_CHILDREN(var) (var)->children_last = (var)->children = (widget_LL *) malloc(sizeof(widget_LL)); (var)->children->next = null; (var)->children->type = WIDGET_NONE;
#define MAKE_WIDGET(var, type_) (var)->children_last = (var)->children_last->next = (widget_LL *) malloc(sizeof(widget_LL)); (var)->children_last->next = null; (var)->children_last->type = (type_); MAKE_CHILDREN((var)->children_last);
#define MAKE_WINDOW(var, temp, x_, y_, w_, h_) (temp) = (window_object *) (var)->children_last->widget = (void *) malloc(sizeof(window_object)); (temp)->x = x_; (temp)->y = y_; (temp)->w = w_; (temp)->h = h_;
#define MAKE_BUTTON(var, temp, x_, y_, w_, h_, callback_, text_) (temp) = (button_object *) (var)->children_last->widget = (void *) malloc(sizeof(button_object)); (temp)->x = x_; (temp)->y = y_; (temp)->w = w_; (temp)->h = h_; (temp)->callback = callback_; (temp)->text = text_;

#define SET_FG(obj, r_, g_, b_) \
(obj)->attr.r = r_; \
(obj)->attr.g = g_; \
(obj)->attr.b = b_;

#define SET_BG(obj, r_, g_, b_, a_) \
(obj)->attr.bgr = r_; \
(obj)->attr.bgg = g_; \
(obj)->attr.bgb = b_; \
(obj)->attr.bga = a_;

#define SET_BORDER(obj, r_, g_, b_) \
(obj)->attr.br = r_; \
(obj)->attr.bg = g_; \
(obj)->attr.bb = b_;

static float z;

void basic_button(ui_object *obj, button_object *but) {
  but->attr.bgr = 255 - but->attr.bgr;
}

void ui_init(ui_object *obj) {
  window_object *win;
  button_object *but;
  MAKE_CHILDREN(obj);
  MAKE_WIDGET(obj, WIDGET_WINDOW);
  MAKE_WINDOW(obj, win, 50, 50, 600, 400);
  SET_BG(win, 1.0f, 0.0f, 0.0f, 0.0f);
  SET_BORDER(win, 1.0f, 0.0f, 0.0f);

  MAKE_WIDGET(obj->children_last, WIDGET_BUTTON);
  MAKE_BUTTON(obj->children_last, but, 100, 150, 200, 50, basic_button, "Test!");
  SET_FG(but, 0.0f, 1.0f, 1.0f);
  SET_BG(but, 0.0f, 0.0f, 0.0f, 1.0f);
  SET_BORDER(but, 1.0f, 1.0f, 1.0f);
}

uint32 ui_window_timer(uint32 interval, void *param) {
  SDL_Event event;
  window_object *win;

  win = param;
  if(win->attr.in) {
    if(win->attr.bga >= 1.0f) {
      event.type = SDL_KEYDOWN;
      event.key.keysym.sym = SDLK_r;
      return 0;
    }
    win->attr.bga += 0.1f;
  }
  else {
    if(win->attr.bga <= win->attr.orig_a)
      return 0;
    win->attr.bga -= 0.1f;
  }
  SDL_PushEvent(&event);
  return interval;
}

int ui_in(ui_object *obj, short x, short y) {
  widget_LL *ll;
  window_object *win;
  char res = 0;

  ll = obj->children;
  while(ll) {
    switch(ll->type) {
    case WIDGET_WINDOW:
      win = ll->widget;
      if(x < win->x || x > win->x + win->w || y < win->y || y > win->y + win->h) {
        if(win->attr.in) {
          SDL_AddTimer(50, (SDL_NewTimerCallback) ui_window_timer, win);
          win->attr.in = 0;
        }
        break;
      }
      res = 1;
    }
    ll = ll->next;
  }
  return res;
}

void ui_click(ui_object *obj, widget_LL *ll, short x, short y, uchar state, void *parent) {
  button_object *but, *but_temp;
  window_object *win, *win_temp;

  if(!ll)
    ll = obj->children;

  while(ll) {
    switch(ll->type) {
    case WIDGET_WINDOW:
      win = ll->widget;

      if(x < win->x || x > win->x + win->w || y < win->y || y > win->y + win->h)
        break;
      ui_click(obj, ll->children, x, y, state, ll->widget);
      break;
    case WIDGET_BUTTON:
      but = ll->widget;
      if(parent) {
        but_temp = but;
        but = (button_object *) malloc(sizeof(button_object));
        memcpy(but, but_temp, sizeof(button_object));
        win_temp = (window_object *) parent;
        but->x += win_temp->x;
        but->y += win_temp->y;
      }
      if(x < but->x || x > but->x + but->w || y < but->y || y > but->y + but->h) {
        if(parent) free(but);
        break;
      }
      but->callback(obj, but);
      if(parent) {
        but->x -= win_temp->x;
        but->y -= win_temp->y;
        memcpy(ll->widget, but, sizeof(button_object));
        free(but);
      }
      break;
    }
    ll = ll->next;
  }
}

void ui_mouse(ui_object *obj, short x, short y, char relx, char rely, uchar state) {
  widget_LL *ll;
  window_object *win;

  ll = obj->children;
  while(ll) {
    switch(ll->type) {
    case WIDGET_WINDOW:
      win = ll->widget;
      switch(state) {
      case 0:
        if(x < win->x || x > win->x + win->w || y < win->y || y > win->y + win->h) {
          if(win->attr.in) {
            win->attr.in = 0;
            SDL_AddTimer(50, (SDL_NewTimerCallback) ui_window_timer, win);
          }
        }
        else {
          if(!win->attr.in) {
            win->attr.in = 1;
            SDL_AddTimer(50, (SDL_NewTimerCallback) ui_window_timer, win);
          }
        }
        break;
      case SDL_BUTTON_LEFT:
        if(x < win->x || x > win->x + win->w || y < win->y || y > win->y + win->h)
          break;
        win->x += relx;
        win->y -= rely;
        return;
      }
      break;
    }
    ll = ll->next;
  }
}

inline void ui_draw_widgets(widget_LL *ll, void *parent) {
  window_object *win_temp, *win;
  button_object *but_temp, *but;

  while(ll) {
    switch(ll->type) {
    case WIDGET_WINDOW:
      win = ll->widget;
      glColor4f(win->attr.bgr, win->attr.bgg, win->attr.bgb, win->attr.bga);
      glEnable(GL_BLEND);
      if(parent) {
        win_temp = win;
        win = (window_object *) malloc(sizeof(window_object));
        memcpy(win, win_temp, sizeof(window_object));
        win_temp = (window_object *) parent;
        win->x += win_temp->x;
        win->y += win_temp->y;
      }
      glBegin(GL_QUADS);
      glVertex3f(win->x + 1, win->y + 1, z);
      glVertex3f(win->x + 1, win->y + win->h - 1, z);
      glVertex3f(win->x + win->w - 1, win->y + win->h - 1, z);
      glVertex3f(win->x + win->w - 1, win->y + 1, z);
      glEnd();
      glDisable(GL_BLEND);

      glColor3f(win->attr.br, win->attr.bg, win->attr.bb);
      glBegin(GL_QUADS);
      glVertex3f(win->x, win->y, z);
      glVertex3f(win->x, win->y + win->h, z);
      glVertex3f(win->x + win->w, win->y + win->h, z);
      glVertex3f(win->x + win->w, win->y, z);
      glEnd();
      z += 0.000001f;
      ui_draw_widgets(ll->children, (void *) win);
      if(parent)
        free(win);
      break;
    case WIDGET_BUTTON:
      but = ll->widget;
      glColor4f(but->attr.bgr, but->attr.bgg, but->attr.bgb, but->attr.bga);
      glEnable(GL_BLEND);
      if(parent) {
        but_temp = but;
        but = (button_object *) malloc(sizeof(button_object));
        memcpy(but, but_temp, sizeof(button_object));
        win_temp = (window_object *) parent;
        but->x += win_temp->x;
        but->y += win_temp->y;
      }
      glBegin(GL_QUADS);
      glVertex3f(but->x + 1, but->y + 1, z);
      glVertex3f(but->x + 1, but->y + but->h - 1, z);
      glVertex3f(but->x + but->w - 1, but->y + but->h - 1, z);
      glVertex3f(but->x + but->w - 1, but->y + 1, z);
      glEnd();
      glDisable(GL_BLEND);

      glColor3f(but->attr.br, but->attr.bg, but->attr.bb);
      glBegin(GL_QUADS);
      glVertex3f(but->x, but->y, z);
      glVertex3f(but->x, but->y + but->h, z);
      glVertex3f(but->x + but->w, but->y + but->h, z);
      glVertex3f(but->x + but->w, but->y, z);
      glEnd();
      z += 0.000001f;
      ui_draw_widgets(ll->children, parent);
      if(parent)
        free(but);
      break;
    }
    ll = ll->next;
  }
}

void ui_draw(ui_object *obj) {
  int vPort[4];

  glGetIntegerv(GL_VIEWPORT, vPort);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_ALPHA_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  z = 0;

  ui_draw_widgets(obj->children, null);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}
