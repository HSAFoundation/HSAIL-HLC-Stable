__kernel void sample_test(__global float *src, __global int *dst)
{
    size_t tid = get_global_id(0);
    dst[tid] = src[tid];
}
