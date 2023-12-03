#ifndef _MATHUTILS_H
#define _MATHUTILS_H

#include <Arduino.h>
#include <sharedspidisplay.h>

/*

    Math utilities

*/

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

// Rotate a 2D-point [ptxIn ptyIn] by a rotation defined by mtxIn around a center defined by [ptxCenter ptyCenter]
// and store the result in [ptxOut ptyOut]
void rotatePoint(int& ptxOut, int& ptyOut, int& ptxIn, int& ptyIn, int& ptxCenter, int& ptyCenter, float* mtxIn) {
    ptxOut = (int) (mtxIn[0]*(ptxIn - ptxCenter) + mtxIn[1]*(ptyIn - ptyCenter)) + ptxCenter;
    ptyOut = (int) (mtxIn[2]*(ptxIn - ptxCenter) + mtxIn[3]*(ptyIn - ptyCenter)) + ptyCenter;
};

// Rotate a 2D-point [ptxIn ptyIn] by a rotation defined by mtxIn around the screen center
// and store the result in [ptxOut ptyOut]
void rotatePointAroundScreenCenter(int& ptxOut, int& ptyOut, int& ptxIn, int& ptyIn, float* mtxIn) {
    ptxOut = (int) (mtxIn[0]*(ptxIn - DISPLAY_WIDTH_HALF) + mtxIn[1]*(ptyIn - DISPLAY_WIDTH_HALF)) + DISPLAY_WIDTH_HALF;
    ptyOut = (int) (mtxIn[2]*(ptxIn - DISPLAY_WIDTH_HALF) + mtxIn[3]*(ptyIn - DISPLAY_WIDTH_HALF)) + DISPLAY_WIDTH_HALF;
};

// Rotate a 2D-point [ptxIn ptyIn] by a rotation defined by mtxIn around the screen center
// and store the result in [ptxIn ptyIn]
void rotatePointInplaceAroundScreenCenter(int& ptxIn, int& ptyIn, float* mtxIn) {
    int ptxInOld = ptxIn;
    rotatePointAroundScreenCenter(ptxIn, ptyIn, ptxInOld, ptyIn, mtxIn);
};

// Rotate a 2D-point [ptxIn ptyIn] by a rotation defined by mtxIn around a center defined by [ptxCenter ptyCenter]
// and store the result in [ptxIn ptyIn]
void rotatePointInplace(int& ptxIn, int& ptyIn, int& ptxCenter, int& ptyCenter, float* mtxIn) {
    int ptxInOld = ptxIn;
    rotatePoint(ptxIn, ptyIn, ptxInOld, ptyIn, ptxCenter, ptyCenter, mtxIn);
};

#endif