#ifndef __EQCLIENT_MOB_H_
#define __EQCLIENT_MOB_H_

int Mobs_Init(mobs_object *obj);
int Mobs_Add(mobs_object *obj, char *name, char *model, float loc[3], float angle);
int Mobs_Move(mobs_object *obj, char *name, float loc[3], float angle);
int Mobs_Del(mobs_object *obj, char *name);

int Mob_Init(mob_object *obj, char *model);
int Mob_Del(mob_object *obj);

#endif
