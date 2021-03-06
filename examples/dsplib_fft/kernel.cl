void DSPF_sp_fftSPxSP(int N,
    global float *x, global float *w, global float *y,
    unsigned char *brev, int n_min, int offset, int n_max);

kernel void ocl_DSPF_sp_fftSPxSP(int N,
    global float *x, global float *w, global float *y,
    int n_min, int offset, int n_max)
{
  DSPF_sp_fftSPxSP (N, x, w, y, 0, n_min, offset, n_max);
}
