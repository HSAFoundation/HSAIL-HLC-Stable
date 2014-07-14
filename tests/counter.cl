#pragma OPENCL EXTENSION cl_amd_c11_atomics : enable

__kernel void counter(volatile __global int *lock, __global int*   count)
{
	int l=0;
	/* lock using exchange */
	do {
		l = atomic_exchange(&lock[0], 1);
    } while (l == 1);

   (*count)++;
   /* unlock */
   lock[0] = 0;
}
