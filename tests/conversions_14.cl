__kernel void test_convert_uchar2_rtp_uchar2( __global uchar2 *src, __global uchar2 *dest )
{
   size_t i = get_global_id(0);
   dest[i] = convert_uchar2_rtp( src[i] );
}
