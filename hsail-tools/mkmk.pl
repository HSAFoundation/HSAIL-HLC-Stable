use Data::Dumper;
$subdirs = "2k xp64a lnx lnx64a lh lh64a w7 w764a w8 w864a";
open F,"p4 -z tag opened |";
while(<F>) {
	chomp;
	next unless s{^\.\.\. clientFile .*/assembler1.0/(.*)$}{$1};
        if(m{^(.*?/)(Makefile)(\..*)$}) { $text{$1.$2} = "BUILD_MAKEFILE = $2$3\nBUILD_SUBDIRS = $subdirs"; }
        elsif (m{/Makefile$}) { $makefiles{$_} = 1; }
}
$text{"libBRIGdwarf/Makefile"} = <<'EOT';
SUBDIRS = $(OPENCL_DEPTH)/compiler/lib/loaders/elf/utils/libelf \
          libdwarf \
          build
EOT
print Dumper(\%text,\%makefiles);
for $fn (keys %makefiles) {
	$depth = "../../../$fn";
	$depth =~ s/[^\/]+/../g;
	($opencl_depth = $depth) =~ s|^\Q../../\E||;
	($asm_depth = $opencl_depth) =~ s|^\Q../../\E||;
	open F,">$fn" or die "Unable to open $fn";
	$tt = $text{$fn} // "SUBDIRS = build";
	print F <<"EOT";
OPENCL_DEPTH = $opencl_depth
ASM_DEPTH = $asm_depth

include \$(ASM_DEPTH)/asmdefs

$tt

include \$(ASM_DEPTH)/asmrules
EOT
	close F;
}
