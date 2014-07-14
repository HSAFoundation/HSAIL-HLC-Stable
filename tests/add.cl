__kernel void test_int_add(__global int *dst, __global int *src)
{
    int tid;
    tid = get_global_id(0);

    dst[tid] = src[tid] + 10;
}

