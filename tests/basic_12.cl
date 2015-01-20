__kernel void test_int_mul(__global int *srcA, __global int *srcB, __global int *dst)
{
    int  tid = get_global_id(0);

    dst[tid] = srcA[tid] * srcB[tid];
}
