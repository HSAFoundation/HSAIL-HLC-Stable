#/usr/bin/perl
$numRegs = 256;
$numTotalRegs = 1013;
$highRegs = 1000;
$testNum = 1;
open OUTPUT, ">AMDILRegisterDefsScalar.td" or die$!;
while($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    $b = $testNum;
    print OUTPUT "def Rx$testNum : AMDILReg<$b, \"r$testNum\">, DwarfRegNum<[$b]>;\n";
    print OUTPUT "def Ry$testNum : AMDILReg<$b, \"r$testNum\">, DwarfRegAlias<Rx$b>;\n";
    print OUTPUT "def Rz$testNum : AMDILReg<$b, \"r$testNum\">, DwarfRegAlias<Rx$b>;\n";
    print OUTPUT "def Rw$testNum : AMDILReg<$b, \"r$testNum\">, DwarfRegAlias<Rx$b>;\n";
  }
  ++$testNum;
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterDefsV2.td" or die$!;
while($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    $b = $testNum;
    print OUTPUT "def Rxy$testNum : AMDILRegWithSubReg<$b, \"r$testNum\", ";
    print OUTPUT "[";
    print OUTPUT "Rx$testNum, Ry$testNum";
    print OUTPUT "], [sub_x_comp, sub_y_comp]>, DwarfRegAlias<Rx$b>;\n";
    print OUTPUT "def Rzw$testNum : AMDILRegWithSubReg<$b, \"r$testNum\", ";
    print OUTPUT "[";
    print OUTPUT "Rz$testNum, Rw$testNum";
    print OUTPUT "], [sub_z_comp, sub_w_comp]>, DwarfRegAlias<Rx$b>;\n";
  }
  ++$testNum;
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterDefsV4.td" or die$!;
while($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    $b = $testNum;
    print OUTPUT "def R$testNum : AMDILRegWithSubReg<$b, \"r$testNum\", ";
    print OUTPUT "[";
    print OUTPUT "Rxy$testNum, Rzw$testNum";
    print OUTPUT "], [sub_xy_comp, sub_zw_comp]>, DwarfRegAlias<Rx$b>;\n";
  }
  ++$testNum;
}
close(OUTPUT);

$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesScalar.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rx$testNum, Ry$testNum, Rz$testNum, Rw$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesScalarX.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rx$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesScalarY.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Ry$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesScalarZ.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rz$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesScalarW.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rw$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesV2.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rxy$testNum, Rzw$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesV2XY.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rxy$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesV2ZW.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "Rzw$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);
$testNum = 1;
open OUTPUT, ">AMDILRegisterUsesV4.td" or die$!;
while ($testNum < $numTotalRegs) {
  if ($testNum < $numRegs || $testNum >= $highRegs) {
    print OUTPUT "R$testNum";
  }
  ++$testNum;
  if ($testNum < $numRegs || ($testNum >= $highRegs && $testNum < $numTotalRegs)) {
    print OUTPUT ", ";
  }
}
close(OUTPUT);

