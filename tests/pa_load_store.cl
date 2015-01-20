__kernel void ldstore(volatile __global atomic_int *buffer, volatile __global atomic_int*  atomicBuffer)
{
 int i;
 
   while (atomic_load_explicit (&atomicBuffer[0], memory_order_relaxed) != 99);

  i = get_global_id(0);
  
  atomic_fetch_add(buffer+i, i);

  atomic_store_explicit (&atomicBuffer[i], (100+i), memory_order_release);
   
}
