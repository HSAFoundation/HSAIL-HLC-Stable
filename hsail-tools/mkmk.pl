use Data::Dumper;
$subdirs = '$(DEFAULT_TARGETS)';
open F,"p4 -z tag opened |";
while(<F>) {
	chomp;
	next unless s{^\.\.\. clientFile .*/hsail-tools/(.*)$}{$1};
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
ASM_DEPTH = $asm_depth

include \$(ASM_DEPTH)/asmdefs

$tt

include \$(ASM_DEPTH)/asmrules
EOT
	close F;
}
