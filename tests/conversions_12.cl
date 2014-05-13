__kernel void test_convert_uchar16_rte_uchar16( __global uchar16 *src, __global uchar16 *dest )
{
   size_t i = get_global_id(0);
   dest[i] = convert_uchar16_rte( src[i] );
}
