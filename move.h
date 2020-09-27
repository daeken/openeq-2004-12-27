#ifndef __EQCLIENT_MOVE_H_
#define __EQCLIENT_MOVE_H_
#include "draw.h"

extern int keystate;
extern Uint32 keydowntime[6];

#define KEY_UP		0
#define KS_UP		0x01
#define KEY_DOWN	1
#define KS_DOWN		0x02
#define KEY_LEFT	2
#define KS_LEFT		0x04
#define KEY_RIGHT	3
#define KS_RIGHT	0x08
#define KEY_VUP		4
#define KS_VUP		0x10
#define KEY_VDN		5
#define KS_VDN		0x20

inline void pokemove();
Uint32 SDLCALL domove_timer(Uint32, void*);
inline void domove(draw_object *obj);

#endif
