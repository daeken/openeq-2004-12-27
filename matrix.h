#include <GL/gl.h>

inline void matIdentity(GLfloat *targ);
inline void matMultiply(GLfloat *targ, GLfloat *mat);
inline void matRotateX(GLfloat *targ, float theta);
inline void matRotateY(GLfloat *targ, float theta);
inline void matRotateZ(GLfloat *targ, float theta);
inline void matTransform(GLfloat *mat, GLfloat *vert);
void matDump(GLfloat *mat);

