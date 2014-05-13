#/usr/bin
#
use Getopt::Std;
use Time::HiRes qw(usleep gettimeofday tv_interval);
$inputFile = "";
$inputNoExt = "";
$clang = "clc --emit=llvmbc -D__IMAGE_SUPPORT__=1 -DFP_FAST_FMAF=1 -DFP_FAST_FMA=1 -D__AMD__=1 -D__Lions__=1 -D__GPU__=1 -Dcl_khr_fp64=1 -Dcl_amd_fp64=1 -Dcl_khr_global_int32_base_atomics=1-Dcl_khr_global_int32_extended_atomics=1 -Dcl_khr_local_int32_base_atomics=1 -Dcl_khr_local_int32_extended_atomics=1 -Dcl_khr_3d_image_writes=1 -Dcl_khr_byte_addressable_store=1 -Dcl_khr_gl_sharing=1 -Dcl_ext_atomic_counters_32=1 -Dcl_amd_device_attribute_query=1 -Dcl_amd_vec3=1 -Dcl_amd_printf=1 -Dcl_amd_media_ops=1 -Dcl_amd_popcnt=1 -Dcl_khr_d3d10_sharing=1";
$llvmlink = "llvm-link";
$llvmasm = "llvm-as";
$llvmopt = "opt -O3 -time-passes ";
$llc = "llc -time-passes -stats -march=hsail ";
sub processArgs
{
    @NoExt = split(/\./,$ARGV[0]);
    $inputFile = $ARGV[0];
    $inputNoExt = @NoExt[0];
}
sub compileProg
{
    print "Timing information:\n";
    print STDERR "Function $inputFile\n";
    $start = [gettimeofday];
    @ret = system("$clang $inputFile");
    if (@ret[0] != 0) {
        print "Compiling to llvm-ir for $inputFile failed.\n";
        system("cat $inputFile");
        return;
    }

    $fetime = [gettimeofday];
    $s0 = tv_interval($start, $fetime);
    print "CLC FE: $s0 \n";

    @ret = system("$llvmopt < $inputNoExt.bc > $inputNoExt-opt.bc");
    if (@ret[0] != 0) {
        print "optimizing bitcode failed.\n";
        system("cat $inputNoExt.cl");
        return;
    }
    $opttime = [gettimeofday];
    $s2 = tv_interval($fetime, $opttime);
    print "OPTMZR: $s2 \n";

    @ret = system("$llc < $inputNoExt-opt.bc -o $inputNoExt-opt.s");
    if (@ret[0] != 0) {
        print "Compiling IL code failed.\n";
        return;
    }
    $llctime = [gettimeofday];

    @ret = system("$llc < $inputNoExt.bc -o $inputNoExt-unopt.s");
    if (@ret[0] != 0) {
        print "Compiling IL code failed.\n";
        return;
    }
    $llc2time = [gettimeofday];
    $s3 = tv_interval($opttime, $llctime);
    $s4 = tv_interval($llctime, $llc2time);
    print "LLCOMP: Opt: $s3 UnOpt: $s4\n";

    $s6 = $s0 + $s2 + $s3;
    $s7 = $s0 + $s4;
    print "COMBIN: $s6 $s7\n";

}

processArgs();
compileProg();

