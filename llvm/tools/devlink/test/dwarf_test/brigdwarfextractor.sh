#!/bin/bash

IFS=$'\n'

readonly pfx="bdx:"
echo "$pfx BRIG DWARF extraction tool (c) AMD 2012"

# BRIG enums
readonly BrigEBlockStart=19
readonly BrigEBlockNumeric=20
readonly BrigEBlockString=21
readonly BrigEBlockEnd=22

# BRIG strings
# expected BRIG block section's name (may be different from ELF)
readonly BrigDebugBlockSection="debug"
# expected BRIG block section data type
readonly BrigDebugFormatName="hsa_dwarf_debug"

#
# some useful routines
#

g_num_dump=""
# read out array of records from binary size
# arg1 - file
# arg2 - record count
# arg3 - record size (1, 2, 4, 8)
# arg4 - offset
# arg5 - (optional) format
# g_num_dump - result
read_dec_numbers () {
  if [ $# -gt 4 ]
  then
    g_num_dump=(`hexdump -v -e "$2/$3 \" $5\"" -s $4 -n $(($2*$3)) $1`)
  elif [ $# -eq 4 ]
  then
    g_num_dump=(`hexdump -v -e "$2/$3 \" %u\"" -s $4 -n $(($2*$3)) $1`)
  else
    echo "$pfx not enough arguments for read_dec_numbers()"
    exit
  fi
}

g_strtab_entry=""
# read out string from string table
# arg1 - file
# arg2 - offset in hexadecimal form without preceding 0x
# g_strtab_entry - result
read_strtab () {
  g_strtab_entry=`readelf -p .strtab $1 | sed -n 's/^[ ]\+\[[ ]\+'$2'\][ ]\+//p'`
}

# processes BRIG debug section
# arg1 - file
# arg2 - section name
# arg3 - offset, decimal
# arg4 - size, decimal
process_debug () {
  local brig_file=$1
  local brig_section_name=$2
  local brig_section_offs=$3
  local brig_section_size=$4

  # skip 4 bytes 
  local input_file_skip=4

  echo "$pfx found $brig_section_name section at $brig_section_offs, $brig_section_size bytes total";

  #
  # Check that we have BrigEBlockStart
  #

  # read 4 directive bytes
  local current_brig_offs=$(($brig_section_offs + $input_file_skip))
  read_dec_numbers $brig_file 2 2 $current_brig_offs
  local syze=${g_num_dump[0]} # size
  local kind=${g_num_dump[1]} # kind

  # check that this is block start
  if [ $kind -ne $BrigEBlockStart ] 
  then
    echo "$pfx BRIG block section in file $brig_file at offset $current_brig_offs has unexpected type $kind, BrigEBlockStart($BrigEBlockStart) expected"
    exit
  fi

  # skip 8 bytes and read offset of block name
  current_brig_offs=$(($brig_section_offs + $input_file_skip + 8))
  read_dec_numbers $brig_file 1 4 $current_brig_offs "%x "
  local block_name_index=${g_num_dump[0]}

  # read name string from strtab
  read_strtab $brig_file $block_name_index

  # check that name is debug, not the "rti" (we do not support rti at this time)
  if [ "$g_strtab_entry" != "$BrigDebugBlockSection" ]
  then
    echo "$pfx BRIG block section in file $brig_file at offset $current_brig_offs has unexpected name \"$g_strtab_entry\", \"$BrigDebugBlockSection\" expected"
    exit
  fi

  #skip this directive
  local input_file_skip=$(($input_file_skip + $syze))

  #
  # Check that we have BrigEBlockString
  #

  # read 4 directive bytes
  current_brig_offs=$(($brig_section_offs + $input_file_skip))
  read_dec_numbers $brig_file 2 2 $current_brig_offs
  syze=${g_num_dump[0]} # size
  kind=${g_num_dump[1]} # type

  # check that this is block string
  if [ $kind -ne $BrigEBlockString ]
  then
    echo "$pfx BRIG block section in file $brig_file at offset $current_brig_offs has unexpected type $kind, BrigEBlockString($BrigEBlockString) expected"
    exit
  fi

  # skip 4 bytes and read offset of debug information type name
  current_brig_offs=$(($brig_section_offs + $input_file_skip + 4))
  read_dec_numbers $brig_file 1 4 $current_brig_offs "%x "
  block_name_index=${g_num_dump[0]}

  # read name string from strtab
  read_strtab $brig_file $block_name_index

  # check that debug information type is DWARF
  if [ "$g_strtab_entry" != "$BrigDebugFormatName" ]
  then
    echo "$pfx File $brig_file has unsupported debug information format \"$g_strtab_entry\", \"$BrigDebugFormatName\" expected"
    exit
  fi

  #skip this directive
  input_file_skip=$(($input_file_skip + $syze))

  #
  # Check that we have one or more BrigEBlockNumeric's and store ELF DWARF data in .elf file
  #
  local output_elf_file_name=$brig_file$brig_section_name".dump"
  if [ -f $output_elf_file_name ]
  then
    echo "$pfx Warning: file $output_elf_file_name exists, removing it from the current directory"
    rm -f $output_elf_file_name
  fi

  # read 4 directive bytes
  read_dec_numbers $brig_file 2 2 $(($3 + $input_file_skip))
  syze=${g_num_dump[0]} # size
  kind=${g_num_dump[1]} # type

  while [ $kind -ne $BrigEBlockEnd ]
  do
    # check that this is block numeric data
    if [ $kind -ne $BrigEBlockNumeric ]
    then
      echo "$pfx BRIG block section in file $1 at offset $(($3 + $input_file_skip)) has unexpected type $kind, BrigEBlockNumeric($BrigEBlockNumeric) expected"
      exit
    fi

    # TODO check data type which should be Brigb8

    # read out ELF data length
    read_dec_numbers $1 1 2 $(($3 + $input_file_skip + 8))
    local elf_chunk_size=${g_num_dump[0]} # size
    local elf_chunk_offset=$(($3 + $input_file_skip + 10)) # offset from the beginning of the file +10 for the BrigEBlockNumberic data
    echo "$pfx found debug ELF data chunk at $elf_chunk_offset, $elf_chunk_size bytes total"

    # read out ELF data and write it to the separate file
    touch $output_elf_file_name
    local output_elf_file_offset=`stat -c '%s' $output_elf_file_name`
    dd if=$1 of=$output_elf_file_name bs=1 skip=$elf_chunk_offset count=$elf_chunk_size seek=$output_elf_file_offset 2>/dev/null

    #proceed to the next directive
    input_file_skip=$(($input_file_skip + $syze))

    # read 4 directive bytes
    read_dec_numbers $1 2 2 $(($3 + $input_file_skip))
    syze=${g_num_dump[0]} # size
    kind=${g_num_dump[1]} # type
  done

}


# process BRIG file
# arg1 - BRIG file
process_brig_file () {
  echo "$pfx processing $1"

  # match all sections except 0th which is NULL
  local section_lines=`readelf -S $1 | sed -n 's/^[ \t]*\[[ \t]*\([1-9][0-9]*\)[ \t]*\]/\1 /p'`

  IFS=$'\n'

  # process debug sections
  for lne in $section_lines
  do
    IFS=$' '
    local attr=( $lne )
    # echo $lne
    local section_num=${attr[0]}
    local section_name=${attr[1]}
    #section_type=${attr[2]}
    #section_addr=${attr[3]}
    local section_offs="0x${attr[4]}"
    section_offs=$((section_offs))
    local section_size="0x${attr[5]}"
    section_size=$((section_size))
    case $section_name in
      ".debug") process_debug $1 $section_name $section_offs $section_size;;
      *) ;;
    esac
  done
  IFS=$'\n'
}


while [ $# -gt 0 ]
do
  process_brig_file $1
  # proceed to the next file
  shift
done

exit 0
