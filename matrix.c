#include <math.h>
#include <stdio.h>
#include "matrix.h"

const GLfloat deg2rad = 3.14159f / 180.0f;

inline void matIdentity(GLfloat *mat) {
  int i;
  for(i = 0;i < 16;i++)
    mat[i] = i % 5 ? 0.0f : 1.0f;
}

inline void matMultiply(GLfloat *targ, GLfloat *mat) {
  GLfloat out[16];
  int i, j;

  for(i = 0;i < 16;i++) {
    int row = i % 4, col = i - row;
    out[i] = 0.0f;
    for(j = 0;j < 4;j++)
      out[i] += targ[j * 4 + row] * mat[col + j];
  }
  for(i = 0;i < 16;i++)
    targ[i] = out[i];
}

inline void matRotateX(GLfloat *targ, GLfloat theta) {
  GLfloat mat[16];
  GLfloat rad = theta * deg2rad;
  GLfloat c = cos(rad), s = sin(rad);

  matIdentity(mat);
  mat[5] = mat[10] = c;
  mat[6] = s;
  mat[9] = -s;
  matMultiply(targ, mat);
}

inline void matRotateY(GLfloat *targ, GLfloat theta) {
  GLfloat mat[16];
  GLfloat rad = theta * deg2rad;
  GLfloat c = cos(rad), s = sin(rad);

  matIdentity(mat);
  mat[0] = mat[10] = c;
  mat[3] = -s;
  mat[8] = s;
  matMultiply(targ, mat);
}

inline void matRotateZ(GLfloat *targ, GLfloat theta) {
  GLfloat mat[16];
  GLfloat rad = theta * deg2rad;
  GLfloat c = cos(rad), s = sin(rad);

  matIdentity(mat);
  mat[0] = mat[5] = c;
  mat[1] = -s;
  mat[4] = s;
  matMultiply(targ, mat);
}

inline void matTransform(GLfloat *mat, GLfloat *vert) {
  GLfloat vtc[16];
  GLfloat temp[16];

  memcpy(temp, mat, sizeof(GLfloat) * 16);
  matIdentity(vtc);
  vtc[0] = vert[0];
  vtc[1] = vert[1];
  vtc[2] = vert[2];
  matMultiply(temp, vtc);
  vert[0] = temp[0];
  vert[1] = temp[1];
  vert[2] = temp[2];
}

/*inline void quat2mat(GLfloat *quat, GLfloat *mat) {
  mat[0] = 1 - 2 * (quat[2]*quat[2] + quat[3]*quat[3]); // 1 - 2y^2 - 2z^2
  mat[1] = 2 * (quat[1] * quat[2] + quat[0] * quat[3]); // 2xy + 2wz
  mat[2] = 2 * (quat[1] * quat[3] - quat[0] * quat[2]); // 2xz - 2wy
  mat[3] = 0.0f;
  mat[4] = 2 * (quat[1] * quat[2] - quat[0] * quat[3]); // 2xy - 2wz
  mat[5] = 1 - 2 * (quat[1]*quat[1] + quat[3]*quat[3]); // 1 - 2x^2 - 2z^2
  mat[6] = 2 * (quat[2] * quat[3] + quat[0] * quat[1]); // 2yz + 2wx
  mat[7] = 0.0f;
  mat[8] = 2 * (quat[1] * quat[3] + quat[0] * quat[2]); // 2xz + 2wy
  mat[9] = 2 * (quat[2] * quat[3] - quat[0] * quat[1]); // 2yz - 2wx
  mat[10] = 1 - 2 * (quat[1]*quat[1] + quat[2]*quat[2]); // 1 - 2x^2 - 2y^2
  mat[11] = 0.0f;
  mat[12] = 0.0f;
  mat[13] = 0.0f;
  mat[14] = 0.0f;
  mat[15] = 1.0f;
}

inline void quatRotate(GLfloat *quat, GLfloat theta, GLfloat x, GLfloat y, GLfloat z) {
  quat[0] = 
*/

void matDump(GLfloat *mat) {
  printf("\n");
  printf("%+03.02f %+03.02f %+03.02f %+03.02f\n", mat[0], mat[4], mat[8], mat[12]);
  printf("%+03.02f %+03.02f %+03.02f %+03.02f\n", mat[1], mat[5], mat[9], mat[13]);
  printf("%+03.02f %+03.02f %+03.02f %+03.02f\n", mat[2], mat[6], mat[10], mat[14]);
  printf("%+03.02f %+03.02f %+03.02f %+03.02f\n", mat[3], mat[7], mat[11], mat[15]);
}


