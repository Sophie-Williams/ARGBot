#include "highgui.h"
#include "imgproc.h"
#include <cmath>
#include "gpu_util.h"
#include <cassert>
#include <cstdio>

#define M_PI_8   0.39269908169872414
#define M_3_PI_8 1.1780972450961724
#define M_5_PI_8 1.9634954084936207
#define M_7_PI_8 2.748893571891069

__global__ void GPU_bilinear_filter2(float *G, float *F, int G_rows, int G_cols, int F_rows, int F_cols, int n_slices, float kr, float kc) {
  // gather
  int j = blockIdx.x * blockDim.x + threadIdx.x;
  int i = blockIdx.y * blockDim.y + threadIdx.y;
  if (i >= G_rows || j >= G_cols) {
    return;
  }
  float y = (float)i * kr - 0.5f;
  float x = (float)j * kc - 0.5f;
  int Fi = (int)floorf(y);
  int Fj = (int)floorf(x);
  float dy = 1.0f - (y - floorf(y));
  float dx = 1.0f - (x - floorf(x));
  float wsum = 0.0f;
  float total = 0.0f;
  for (int k = 0; k < n_slices; k++) {
    if (Fj >= 0 && Fj < F_cols) {
      if (Fi >= 0 && Fi < F_rows) {
        wsum += dx * dy * F[IJK2C(Fi, Fj, k, F_rows, F_cols)];
        total += dx * dy;
      }
      if (Fi+1 >= 0 && Fi+1 < F_rows) {
        wsum += dx * (1-dy) * F[IJK2C(Fi+1, Fj, k, F_rows, F_cols)];
        total += dx * (1-dy);
      }
    }
    if (Fj+1 >= 0 && Fj+1 < F_cols) {
      if (Fi >= 0 && Fi < F_rows) {
        wsum += (1-dx) * dy * F[IJK2C(Fi, Fj+1, k, F_rows, F_cols)];
        total += (1-dx) * dy;
      }
      if (Fi+1 >= 0 && Fi < F_cols) {
        wsum += (1-dx) * (1-dy) * F[IJK2C(Fi+1, Fj+1, k, F_rows, F_cols)];
        total += (1-dx) * (1-dy);
      }
    }
    if (total != 0.0f) {
      G[IJK2C(i, j, k, G_rows, G_cols)] = wsum / total; // normalize
    }
  }
}

gcube gpu_imresize2(const gcube &A, int m, int n) {
  gcube G(m, n, A.n_slices);
  double kr = (double)A.n_rows / (double)m;
  double kc = (double)A.n_cols / (double)n;
  dim3 blockSize(16, 16, 1);
  dim3 gridSize((G.n_cols-1)/16+1, (G.n_rows-1)/16+1, 1);
  GPU_bilinear_filter2<<<gridSize, blockSize>>>(G.d_pixels, A.d_pixels,
      G.n_rows, G.n_cols, A.n_rows, A.n_cols, A.n_slices, (float)kr, (float)kc);
  checkCudaErrors(cudaGetLastError());
  return G;
}
