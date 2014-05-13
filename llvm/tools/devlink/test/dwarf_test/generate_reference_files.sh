#!/bin/bash

readonly pfx="grf:"
if [ $# -eq 0 ]
then 
  echo "$pfx usage: $0 [target] [clean|clean_all]"
  exit 1
fi

g_generate=1
g_clean=0
g_clean_outfiles=0
case "$1"
in
  "w864a" | "w8" | "lnx64a" | "lnx")
    readonly xdir="$1/B_dbg"
    ;;
  "rel.w864a")
    readonly xdir="w864a/B_rel"
    ;;
  "rel.w8")
    readonly xdir="w8/B_rel"
    ;;
  "rel.lnx64a")
    readonly xdir="lnx64a/B_rel"
    ;;
  "rel.lnx")
    readonly xdir="lnx/B_rel"
    ;;
  "clean_all")
    readonly xdir="dummy"
    g_generate=0
    g_clean=1
    g_clean_outfiles=1
    ;;
  "clean")
    readonly xdir="dummy"
    g_generate=0
    g_clean=1
    ;;
  *)
    echo "$pfx Wrong parameter 1: $1."
    exit 1
    ;;
esac

if [ $# -gt 1 ]
then
  case "$2"
  in
  "clean_all")
    echo "$pfx Wrong parameter 2: $2. It is meaningless to generate ref files and then remove them"
    exit 1
    ;;
  "clean")
    g_clean=1
    ;;
  *)
    echo "$pfx Wrong parameter 2: $2."
    exit 1
    ;;
  esac
fi

[ "Linux" == "`uname -s`" ] && readonly exe_ext="" || readonly exe_ext=".exe"
readonly hsailasm="../../../../../assembler/HSAILAsm/build/$xdir/HSAILasm"$exe_ext
readonly devlink="../../build/$xdir/devlink"$exe_ext

readonly hsail_ext='.hsail'
readonly brig_ext='.brig'
readonly debug_ext='.debug'

readonly libm='amp_libm2_1'
readonly test1='test_libm1'
readonly test2='test_libm2'
readonly dwarftool='../../../../../tools/dwarftool/dwarftool.sh'

# runs HSAILasm with enabled debug info generation for given filename (without extension)
run_hsailasm () {
  echo "$pfx assembling $1$brig_ext with enabled debug info..."
  $hsailasm -assemble -g -o=$1$brig_ext -odebug=$1$debug_ext $1$hsail_ext
}

g_ref_link_cmd=""
# runs device linker and gather parameters required to run reference linker in g_ref_link_cmd
run_devlink () {
  devlink_log=$3".devlink.log"
  echo "$pfx linking $1$brig_ext with $2$brig_ext and enabled debug info linking..."
  $devlink -link-dwarf -debug-print-offsets -o=$3$brig_ext $1$brig_ext $2$brig_ext > $devlink_log
  offset_records=`cat $devlink_log | sed -n 's/offsets for \([^$]\+\)\.brig\: c_offset \([0-9]\+\) d_offset \([0-9]\+\)/\1.debug \2 \3 /p'`
  g_ref_link_cmd=""
  for off_rec in $offset_records
  do
    g_ref_link_cmd=$g_ref_link_cmd" "$off_rec
  done
}

# runs reference DWARF linker
run_reference_linker () {
  echo "$pfx running reference linker: $dwarftool $g_ref_link_cmd"
  $dwarftool $g_ref_link_cmd > "$1.ref_linker.log"
  cp ./a.out.debug $1"_ref.debug"
  rm -f ./a.out.debug
}

if [ $g_generate -ne 0 ]
then
  # assemble
  run_hsailasm $libm
  run_hsailasm $test1
  run_hsailasm $test2

  # run devlink and reference linker
  run_devlink $test1 $libm $test1"_linked"
  run_reference_linker $test1"_linked"

  run_devlink $test2 $libm $test2"_linked"
  run_reference_linker $test2"_linked"
fi

if [ $g_clean -ne 0 ]
then
    echo "$pfx removing $test1$brig_ext and $test1$debug_ext"
    rm -f $test1$brig_ext $test1$debug_ext 
    echo "$pfx removing $test2$brig_ext and $test2$debug_ext"
    rm -f $test2$brig_ext $test2$debug_ext
    echo "$pfx removing $libm$brig_ext  and $libm$debug_ext"
    rm -f $libm$brig_ext  $libm$debug_ext
    echo "$pfx removing $test1""_linked""$brig_ext and $test2""_linked""$brig_ext"
    rm -f $test1"_linked"$brig_ext
    rm -f $test2"_linked"$brig_ext
    echo "$pfx removing $test1""_linked.devlink.log and $test2""_linked.devlink.log"
    rm -f $test1"_linked.devlink.log"
    rm -f $test2"_linked.devlink.log"
    echo "$pfx removing $test1""_linked.ref_linker.log and $test2""_linked.ref_linker.log"
    rm -f $test1"_linked.ref_linker.log"
    rm -f $test2"_linked.ref_linker.log"
    # remove possible dwarftool leftovers
    echo "$pfx removing a.out.debug*"
    rm -f a.out.debug
    rm -f a.out.debug.*
fi

if [ $g_clean_outfiles -ne 0 ]
then

    echo "$pfx removing $test1""_linked_ref.debug $test2""_linked_ref.debug"
    rm -f               $test1"_linked_ref.debug" $test2"_linked_ref.debug"
fi
