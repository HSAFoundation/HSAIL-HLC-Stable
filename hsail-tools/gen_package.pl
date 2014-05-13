use DirHandle;
use FileHandle;
use strict;

my $plate = <<'EOT';
/*******************************************************************************
*
*
*  Confidential and Proprietary Information
*  Copyright 2012(c), Advanced Micro Devices, Inc. (unpublished)
*
*  All rights reserved. This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality. The year included in the foregoing notice is the
*  year of creation of the work.
*
*
********************************************************************************/
EOT

my $packaging = 0;

sub test {
	return 1 if $packaging;
	return 0 if m{^/test/.*/build/$}i;
	return 0 if m{^/test/MatrixMultiplication/.*\.bin$}i;
	return 0 if m{^/test/MatrixMultiplication/scdata}i;
	return 0 if m{^/test/MatrixMultiplication/build/$}i;
	return 0 if m{^/test/MatrixMultiplication/ReadMe.txt$}i;
	return 1 if m{^/test/}i;
	return 1 if m{^/libHSAIL/$}i;
	return 1 if m{^/llvm/$}i;
	return 1 if m{^/HSAILAsm/$}i;
	return 1 if m{^/finalizer/$}i;
	return 1 if m{^/finalizer/R1000.*/$}i;
	return 0 if m{^/finalizer/.*/$}i;

	return 1 if m{^/finalizer/\Qarena.cpp\E}i;
	return 1 if m{^/finalizer/\Qarena.hpp\E}i;
	return 1 if m{^/finalizer/\Qatiid.h\E}i;
	return 1 if m{^/finalizer/\Qbitfield.cpp\E}i;
	return 1 if m{^/finalizer/\Qbitfield.hpp\E}i;
	return 1 if m{^/finalizer/\Qbitset.hpp\E}i;
	return 1 if m{^/finalizer/\Qbrigbase.hpp\E}i;
	return 1 if m{^/finalizer/\Qbrigir.cpp\E}i;
	return 1 if m{^/finalizer/\Qbrigir.hpp\E}i;
	return 1 if m{^/finalizer/\Qcfg.hpp\E}i;
	return 1 if m{^/finalizer/\Qchannel.hpp\E}i;
	return 1 if m{^/finalizer/\Qci_id.h\E}i;
	return 1 if m{^/finalizer/\Qcompiler.hpp\E}i;
	return 1 if m{^/finalizer/\Qcompilerbase.cpp\E}i;
	return 1 if m{^/finalizer/\Qcompilerbase.hpp\E}i;
	return 1 if m{^/finalizer/\Qcompilerexternal.cpp\E}i;
	return 1 if m{^/finalizer/\Qcompilerexternal.hpp\E}i;
	return 1 if m{^/finalizer/\Qconstant.hpp\E}i;
	return 1 if m{^/finalizer/\Qdefs.h\E}i;
	return 1 if m{^/finalizer/\Qdiagnose.cpp\E}i;
	return 1 if m{^/finalizer/\Qdiagnose.hpp\E}i;
	return 1 if m{^/finalizer/\Qdlist.cpp\E}i;
	return 1 if m{^/finalizer/\Qdlist.hpp\E}i;
	return 1 if m{^/finalizer/\Qdominatorbase.hpp\E}i;
	return 1 if m{^/finalizer/\Qexpansion.hpp\E}i;
	return 1 if m{^/finalizer/\Qfsailtoir.cpp\E}i;
	return 1 if m{^/finalizer/\Qfsasymbol.cpp\E}i;
	return 1 if m{^/finalizer/\Qfsasymbol.hpp\E}i;
	return 1 if m{^/finalizer/\Qglobalregs.hpp\E}i;
	return 1 if m{^/finalizer/\Qglobals.h\E}i;
	return 1 if m{^/finalizer/\Qhwutils.cpp\E}i;
	return 1 if m{^/finalizer/\Qhwutils.hpp\E}i;
	return 1 if m{^/finalizer/\Qidvbase.hpp\E}i;
	return 1 if m{^/finalizer/\Qil2irtable.hpp\E}i;
	return 1 if m{^/finalizer/\Qil_ops_table.h\E}i;
	return 1 if m{^/finalizer/\Qildisassembler.cpp\E}i;
	return 1 if m{^/finalizer/\Qildisassembler.h\E}i;
	return 1 if m{^/finalizer/\Qilformat.h\E}i;
	return 1 if m{^/finalizer/\Qilformatdecode.hpp\E}i;
	return 1 if m{^/finalizer/\Qilmask.h\E}i;
	return 1 if m{^/finalizer/\Qilparser.cpp\E}i;
	return 1 if m{^/finalizer/\Qilparser.h\E}i;
	return 1 if m{^/finalizer/\Qilprogram.hpp\E}i;
	return 1 if m{^/finalizer/\Qilshift.h\E}i;
	return 1 if m{^/finalizer/\Qiltables.cpp\E}i;
	return 1 if m{^/finalizer/\Qiltables.h\E}i;
	return 1 if m{^/finalizer/\Qiltablesinternal.hpp\E}i;
	return 1 if m{^/finalizer/\Qirdefs.hpp\E}i;
	return 1 if m{^/finalizer/\Qirinst.hpp\E}i;
	return 1 if m{^/finalizer/\Qirlimits.hpp\E}i;
	return 1 if m{^/finalizer/\Qkv_id.h\E}i;
	return 1 if m{^/finalizer/\Qlibs.h\E}i;
	return 1 if m{^/finalizer/\Qlinkageinfo.hpp\E}i;
	return 1 if m{^/finalizer/\Qmachineassembler.hpp\E}i;
	return 1 if m{^/finalizer/\Qmain.cpp\E}i;
	return 1 if m{^/finalizer/\Qmem.hpp\E}i;
	return 1 if m{^/finalizer/\Qnumberrep.hpp\E}i;
	return 1 if m{^/finalizer/\Qoptables.hpp\E}i;
	return 1 if m{^/finalizer/\Qpatterns.cpp\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-asic.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-asic.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-chip-enum.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-chip-offset.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-chip-registers.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-chip-typedef.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-dis.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-dis.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-gen.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-inst-info.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-inst-info.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-regs.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-regs.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-sq-uc-reg.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-tables.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci-tables.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/ci/sp3-ci.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/r1000disassembler.cpp\E}i;
	return 1 if m{^/finalizer/\Qr1000/r1000disassembler.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/r1000limits.hpp\E}i;
	return 1 if m{^/finalizer/\Qr1000/r1100limits.hpp\E}i;
	return 1 if m{^/finalizer/\Qr1000/r678_scdev_enums.hpp\E}i;
	return 1 if m{^/finalizer/\Qr1000/r678_scdev_enums_compatible.hpp\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-asic.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-asic.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-chip-enum.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-chip-offset.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-chip-registers.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-chip-typedef.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-dis.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-dis.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-gen.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-inst-info.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-inst-info.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-regs.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-regs.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-sq-uc-reg.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-tables.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si-tables.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/si/sp3-si.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-asic.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-asic.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-dispatch.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-eval.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-eval.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-gc.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-gc.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-int.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-int.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-lex.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-lib.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-native.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-parse.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-parse.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-type.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-vm.c\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3-vm.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/sp3.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/features_bonaire.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/features_pitcairn.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/features_tahiti.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/features_tiran.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/features_verde.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/features_oland.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_enum.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_mask.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_registers.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_shift.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_sq_reg.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_sq_uc_reg.h\E}i;
	return 1 if m{^/finalizer/\Qr1000/udx/si_ci_merged_typedef.h\E}i;
	return 1 if m{^/finalizer/\Qscassembler.cpp\E}i;
	return 1 if m{^/finalizer/\Qscassembler.hpp\E}i;
	return 1 if m{^/finalizer/\Qscc.hpp\E}i;
	return 1 if m{^/finalizer/\Qsccfg.cpp\E}i;
	return 1 if m{^/finalizer/\Qsccfg.hpp\E}i;
	return 1 if m{^/finalizer/\Qsccommon.h\E}i;
	return 1 if m{^/finalizer/\Qsccopyvs.cpp\E}i;
	return 1 if m{^/finalizer/\Qsccopyvs.hpp\E}i;
	return 1 if m{^/finalizer/\Qsccvnbase.hpp\E}i;
	return 1 if m{^/finalizer/\Qscdominator.cpp\E}i;
	return 1 if m{^/finalizer/\Qscdominator.hpp\E}i;
	return 1 if m{^/finalizer/\Qscenums.hpp\E}i;
	return 1 if m{^/finalizer/\Qscgcm.cpp\E}i;
	return 1 if m{^/finalizer/\Qscgcm.hpp\E}i;
	return 1 if m{^/finalizer/\Qscidv.cpp\E}i;
	return 1 if m{^/finalizer/\Qscidv.hpp\E}i;
	return 1 if m{^/finalizer/\Qscinst.cpp\E}i;
	return 1 if m{^/finalizer/\Qscinst.hpp\E}i;
	return 1 if m{^/finalizer/\Qscinstscheduler.cpp\E}i;
	return 1 if m{^/finalizer/\Qscinstscheduler.hpp\E}i;
	return 1 if m{^/finalizer/\Qscinterface.cpp\E}i;
	return 1 if m{^/finalizer/\Qscinterface.h\E}i;
	return 1 if m{^/finalizer/\Qscinterference.cpp\E}i;
	return 1 if m{^/finalizer/\Qscinterference.hpp\E}i;
	return 1 if m{^/finalizer/\Qscirenums.hpp\E}i;
	return 1 if m{^/finalizer/\Qsclib_ver.h\E}i;
	return 1 if m{^/finalizer/\Qscmatheng.cpp\E}i;
	return 1 if m{^/finalizer/\Qscmatheng.hpp\E}i;
	return 1 if m{^/finalizer/\Qscmathengcmntypes.hpp\E}i;
	return 1 if m{^/finalizer/\Qscmathengcommon.hpp\E}i;
	return 1 if m{^/finalizer/\Qscoperand.hpp\E}i;
	return 1 if m{^/finalizer/\Qscopinfo.cpp\E}i;
	return 1 if m{^/finalizer/\Qscopinfo.hpp\E}i;
	return 1 if m{^/finalizer/\Qscosr.cpp\E}i;
	return 1 if m{^/finalizer/\Qscosr.hpp\E}i;
	return 1 if m{^/finalizer/\Qscpatterns.cpp\E}i;
	return 1 if m{^/finalizer/\Qscpatterns.hpp\E}i;
	return 1 if m{^/finalizer/\Qscpeephole.cpp\E}i;
	return 1 if m{^/finalizer/\Qscpeephole.hpp\E}i;
	return 1 if m{^/finalizer/\Qscrefinememory.cpp\E}i;
	return 1 if m{^/finalizer/\Qscrefinememory.hpp\E}i;
	return 1 if m{^/finalizer/\Qscregalloc.cpp\E}i;
	return 1 if m{^/finalizer/\Qscregalloc.hpp\E}i;
	return 1 if m{^/finalizer/\Qscshaderinfo.cpp\E}i;
	return 1 if m{^/finalizer/\Qscshaderinfo.hpp\E}i;
	return 1 if m{^/finalizer/\Qscshadersr678xxcommon.h\E}i;
	return 1 if m{^/finalizer/\Qscshaderssi.h\E}i;
	return 1 if m{^/finalizer/\Qscssa.cpp\E}i;
	return 1 if m{^/finalizer/\Qscssa.hpp\E}i;
	return 1 if m{^/finalizer/\Qscstructureanalyzer.cpp\E}i;
	return 1 if m{^/finalizer/\Qscstructureanalyzer.hpp\E}i;
	return 1 if m{^/finalizer/\Qscsymbol.cpp\E}i;
	return 1 if m{^/finalizer/\Qscsymbol.hpp\E}i;
	return 1 if m{^/finalizer/\Qscsymboltable.cpp\E}i;
	return 1 if m{^/finalizer/\Qscsymboltable.hpp\E}i;
	return 1 if m{^/finalizer/\Qsctargetinfo.cpp\E}i;
	return 1 if m{^/finalizer/\Qsctargetinfo.hpp\E}i;
	return 1 if m{^/finalizer/\Qsctypes.h\E}i;
	return 1 if m{^/finalizer/\Qscunroll.cpp\E}i;
	return 1 if m{^/finalizer/\Qscunroll.hpp\E}i;
	return 1 if m{^/finalizer/\Qscvaluenumber.cpp\E}i;
	return 1 if m{^/finalizer/\Qscvaluenumber.hpp\E}i;
	return 1 if m{^/finalizer/\Qscwavecf.cpp\E}i;
	return 1 if m{^/finalizer/\Qscwavecf.hpp\E}i;
	return 1 if m{^/finalizer/\Qset.hpp\E}i;
	return 1 if m{^/finalizer/\Qshdisassembler.cpp\E}i;
	return 1 if m{^/finalizer/\Qshdisassembler.h\E}i;
	return 1 if m{^/finalizer/\Qshutils.cpp\E}i;
	return 1 if m{^/finalizer/\Qshutils.h\E}i;
	return 1 if m{^/finalizer/\Qsi_id.h\E}i;
	return 1 if m{^/finalizer/\Qsihwshaders.cpp\E}i;
	return 1 if m{^/finalizer/\Qstack.hpp\E}i;
	return 1 if m{^/finalizer/\Qstats.hpp\E}i;
	return 1 if m{^/finalizer/\Qstring.hpp\E}i;
	return 1 if m{^/finalizer/\Qtypedefs.h\E}i;
	return 1 if m{^/finalizer/\Qunionfind.hpp\E}i;
	return 1 if m{^/finalizer/\Qvaluedefs.hpp\E}i;
	return 1 if m{^/finalizer/\Qvector.cpp\E}i;
	return 1 if m{^/finalizer/\Qvector.hpp\E}i;
	return 1 if m{^/finalizer/\Qvregprops.hpp\E}i;

	return 0 if m{^/[^/]*/$}i;
	return 0 if m{^/finalizer/}i;
	return 0 if m{/build/$}i;
	return 1 if m{/$}i;
	return 1 if m{/[^/]*\.(lib|a)$}i;
	if (m{/[^/]*\.(c|h|hpp|cpp|inc|asm|td)$}) {
		return 1 if m{^/llvm/}i;
		return 1 if m{HSAILBrigObjectFile}i;
		return 2;
	}
	return 1 if m{/[^/]*\.vcxproj$};
	return 0 if m{/CMakeLists.txt$}i;
	return 1 if m{\.txt$}i;
	return 0;
};

system("make -C finalizer/patgen PATGEN_OUTDIR=..") && die "Error making patgen";

my $root = ".";
my $dstdir = "package_output";
mkdir($dstdir);

sub process_file {
	my ($src,$dst,$test) = @_;

	local $/ = undef;

	my $fh = new FileHandle;
	open $fh, "<:raw", $root.$src;
	my $text = <$fh>;
#	if ($test==2 && $text =~ /[^\r]\n|\r[^\n]/) {
#		print "WARNING: CR/LF mismatch in $root$src\n";
#	}
	if ($test==2 && $text =~ /[\r]/) {
		print "WARNING: CR/LF mismatch in $root$src\n";
	}
	close $fh;

	my $fh = new FileHandle;
	open $fh, ">:raw", $dstdir.$dst;
	print "Copying ".$dstdir.$dst."\n";
	if ($test == 2) {
		print $fh $plate;
	}
	print $fh $text;
	close $fh;
}

sub traverse {
	my $dir = shift;
	my $dh = new DirHandle;
	opendir $dh, $root.$dir;
	while(my $fn = readdir $dh) {
		next if ($fn =~ /^\.\.?$/);
		my $isdir = -d $root.$dir.$fn;
		local $_ = $dir.$fn.($isdir?"/":"");
		my $test = test();
		if (!$test) { print "excluding $_\n"; next; }
		if ($isdir) { 
			mkdir $dstdir.$dir.$fn;
			traverse($_); 
		} else {
			process_file($_,$_,$test);
		}
	}
	closedir $dh;
}

traverse("/");

system("make -C re2c -f Makefile.standalone") && die "Error making re2c";
my $re2c = File::Spec->catfile(qw(re2c build re2c));
system("$re2c -i -s libHSAIL/HSAILScannerRules.re2c >${dstdir}/libHSAIL/HSAILScannerRules.inc") && die "Error running re2c";

$root="packaging";
$packaging = 1;
traverse("/");

