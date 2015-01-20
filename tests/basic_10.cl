__kernel void test_int_add(__global int *srcA, __global int *srcB, __global int *dst)
{
    int tid = 0;
    tid = get_global_id(0);

    dst[tid] = srcA[tid] + srcB[tid];
}

