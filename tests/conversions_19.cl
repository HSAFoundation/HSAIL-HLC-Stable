__kernel void test_convert_uchar_rtn_uchar( __global uchar *src, __global uchar *dest )
{
   size_t i = get_global_id(0);
   dest[i] = convert_uchar_rtn( src[i] );
}
