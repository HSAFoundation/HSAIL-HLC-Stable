__kernel void counter(volatile __global atomic_int *count)
{
   atomic_fetch_add(count, 1);
}
