#ifndef __TK_IMGPROC_H__
#define __TK_IMGPROC_H__

/** Resize an image using bilinear interpolation
 *  @param A the image to resize
 *  @param m the number of rows of the new size
 *  @param n the number of cols of the new size
 *  @return the interpolated image
 */
gcube gpu_imresize2(const gcube &A, int m, int n);

#endif
