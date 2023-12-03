#ifndef UTILS_MATH_H
#define UTILS_MATH_H

#include <Arduino.h>

/*******************************************************************************
 * Create a 2D-rotation matrix given a certain rotation in radiants.
 * 
 * R =      mtxBuf[0]   mtxBuf[1]
 *          mtxBuf[2]   mtxBuf[3]
 *
 * @param mtxBuf buffer of size sizeof(float)*4 for 2x2 matrix.
 * 
 * @param rad rotation given in radiants
 ******************************************************************************/
void rad2rotMtx(float* mtxBuf, float rad) {
    mtxBuf[0] = cos(rad);
    mtxBuf[1] = -sin(rad);
    mtxBuf[2] = sin(rad);
    mtxBuf[3] = cos(rad);
}


/*******************************************************************************
 * Perform a 2D-rotation around a center point ptxCenter with a given rotation matrix.
 * 
 * Rotates ptxIn around ptxCenter by a rotation mtxIn and stores the result in ptxOut
 *
 * @param ptxOut buffer of size sizeof(int)*2 for rotated 2D-point.
 * 
 * @param mtxIn buffer of size sizeof(float)*4 for 2x2 rotation matrix.
 * 
 * @param ptxIn buffer of size sizeof(int)*2 for original 2D-point.
 * 
 * @param ptxCenter buffer of size sizeof(int)*2 for center 2D-point.
 ******************************************************************************/
void rotatePoint(int* ptxOut, float* mtxIn, int* ptxIn, int* ptxCenter) {
    ptxOut[0] = (int) (mtxIn[0]*(ptxIn[0] - ptxCenter[0]) + mtxIn[1]*(ptxIn[1] - ptxCenter[1])) + ptxCenter[0];
    ptxOut[1] = (int) (mtxIn[2]*(ptxIn[0] - ptxCenter[0]) + mtxIn[3]*(ptxIn[1] - ptxCenter[1])) + ptxCenter[1];
};

/*******************************************************************************
 * Overload of rotatePoint for constant centers.
 ******************************************************************************/
void rotatePoint(int* ptxOut, float* mtxIn, int* ptxIn, const int* ptxCenter) {
    ptxOut[0] = (int) (mtxIn[0]*(ptxIn[0] - ptxCenter[0]) + mtxIn[1]*(ptxIn[1] - ptxCenter[1])) + ptxCenter[0];
    ptxOut[1] = (int) (mtxIn[2]*(ptxIn[0] - ptxCenter[0]) + mtxIn[3]*(ptxIn[1] - ptxCenter[1])) + ptxCenter[1];
};

#endif