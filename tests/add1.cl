global int a;

__kernel void test_int_add(__global int *dst)
{
    int tid;
    tid = get_global_id(0);

    dst[tid] = a;
}

