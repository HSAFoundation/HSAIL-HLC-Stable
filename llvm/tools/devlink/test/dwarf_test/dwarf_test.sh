#!/bin/bash

readonly pfx="DDT:"
echo "$pfx Devlink DWARF-linking test tool (c) AMD 2012"

if [ $# -eq 0 ]
then 
  echo "$pfx usage: $0 target"
  echo "$pfx examples:"
  echo "$pfx     $0 w864a   - tests Win8 Debug binaries"
  echo "$pfx     $0 rel.lnx - tests Linux Release binaries"
  echo "$pfx     $0 clean   - only removes intermediate files"
  exit 1
fi


case "$1"
in
  "w864a" | "w8" | "lnx64a" | "lnx")
    readonly xdir="$1/B_dbg";;
  "rel.w864a")
    readonly xdir="w864a/B_rel";;
  "rel.w8")
    readonly xdir="w8/B_rel";;
  "rel.lnx64a")
    readonly xdir="lnx64a/B_rel";;
  "rel.lnx")
    readonly xdir="lnx/B_rel";;
  "clean")
    readonly xdir="dummy";;
  *)
    echo "Wrong parameter $1"
    exit 1;;
esac

[ "Linux" == "`uname -s`" ] && readonly exe_ext="" || readonly exe_ext=".exe"
readonly hsailasm="../../../../../assembler/HSAILAsm/build/$xdir/HSAILasm"$exe_ext
readonly devlink="../../build/$xdir/devlink"$exe_ext
readonly bdx="./brigdwarfextractor.sh"

readonly hsail_ext='.hsail'
readonly brig_ext='.brig'
readonly debug_ext='.debug'

readonly libm='amp_libm2_1'
readonly test1='test_libm1'
readonly test2='test_libm2'

readonly test1_ref=$test1"_linked_ref"$debug_ext
readonly test2_ref=$test2"_linked_ref"$debug_ext

echo "$pfx cleaning..."
echo "$pfx removing $libm$brig_ext $libm$debug_ext"
rm -f               $libm$brig_ext $libm$debug_ext
echo "$pfx removing $test1$brig_ext $test1$debug_ext $test1""_linked$debug_ext $test1""_linked_stored$debug_ext $test1""_linked$brig_ext $test1""_linked.log"
rm -f               $test1$brig_ext $test1$debug_ext $test1"_linked"$debug_ext $test1"_linked_stored"$debug_ext $test1"_linked"$brig_ext $test1"_linked.log"
echo "$pfx removing $test2$brig_ext $test2$debug_ext $test2""_linked$debug_ext $test2""_linked_stored$debug_ext $test2""_linked$brig_ext $test2""_linked.log"
rm -f               $test2$brig_ext $test2$debug_ext $test2"_linked"$debug_ext $test2"_linked_stored"$debug_ext $test2"_linked"$brig_ext $test2"_linked.log"
if [ "$1" == "clean" ]
then
  exit 0
fi

if [ ! -f $test1_ref ]
then
  echo "$pfx Reference file $test1_ref does not exist, please run './generate_reference_files.sh $1' first"
  exit 1
fi
if [ ! -f $test2_ref ]
then
  echo "$pfx Reference file $test2_ref does not exist, please run './generate_reference_files.sh $1' first"
  exit 1
fi

# runs HSAILasm with enabled debug info generation for given filename (without extension)
run_hsailasm () {
  echo "$pfx assembling $1$brig_ext with enabled debug info..."
  $hsailasm -assemble -g -o=$1$brig_ext -odebug=$1$debug_ext $1$hsail_ext
}

# runs device linker and gather parameters required to run reference linker in g_ref_link_cmd
run_devlink () {
  devlink_log="$3.log"
  echo "$pfx linking $1$brig_ext with $2$brig_ext and enabled debug info linking..."
  $devlink -link-dwarf -no-dedeader -debug-dwarffile=$3$debug_ext -detail-debug-parser -debug-deduper -detail-debug-dedeader -o=$3$brig_ext $1$brig_ext $2$brig_ext > $devlink_log
  echo "$pfx dumping .debug section content from $3$debug_ext..."
  $hsailasm -disassemble -o=/dev/null -validate-linked-code -odebug=$3"_stored"$debug_ext $3$brig_ext
  $bdx $3$brig_ext
}

# runs reference DWARF linker and compares its output with device linker's output
compare_with_reference_linker_output () {
  echo "$pfx comparing debug info linker's output with reference DWARF linker's output ($1$debug_ext vs $2)"
  cmp "$1$debug_ext" "$2"
  if [ $? -ne 0 ] 
  then
    echo "$pfx TEST FAILED"
    exit 1
  fi
  echo "$pfx comparing .debug section dump with reference DWARF linker's output ($1""_stored$debug_ext vs $2)"
  cmp "$1""_stored""$debug_ext" "$2"
  if [ $? -ne 0 ] 
  then
    echo "$pfx TEST FAILED"
    exit 1
  fi
}

# assemble
run_hsailasm $libm
run_hsailasm $test1
run_hsailasm $test2

# run devlink and compare
run_devlink $test1 $libm $test1"_linked" 
compare_with_reference_linker_output $test1"_linked" $test1_ref

run_devlink $test2 $libm $test2"_linked"
compare_with_reference_linker_output $test2"_linked" $test2_ref

echo "$pfx OK"
exit 0
