FILE(REMOVE_RECURSE
  "AMDILGenRegisterInfo.inc.tmp"
  "AMDILGenRegisterInfo.inc"
  "AMDILGenInstrInfo.inc.tmp"
  "AMDILGenInstrInfo.inc"
  "AMDILGenAsmWriter.inc.tmp"
  "AMDILGenAsmWriter.inc"
  "AMDILGenDAGISel.inc.tmp"
  "AMDILGenDAGISel.inc"
  "AMDILGenCallingConv.inc.tmp"
  "AMDILGenCallingConv.inc"
  "AMDILGenSubtarget.inc.tmp"
  "AMDILGenSubtarget.inc"
  "AMDILGenEDInfo.inc.tmp"
  "AMDILGenEDInfo.inc"
  "AMDILGenIntrinsics.inc.tmp"
  "AMDILGenIntrinsics.inc"
  "CMakeFiles/AMDILCommonTableGen"
  "AMDILGenRegisterInfo.inc"
  "AMDILGenInstrInfo.inc"
  "AMDILGenAsmWriter.inc"
  "AMDILGenDAGISel.inc"
  "AMDILGenCallingConv.inc"
  "AMDILGenSubtarget.inc"
  "AMDILGenEDInfo.inc"
  "AMDILGenIntrinsics.inc"
  "AMDILGenRegisterInfo.inc.tmp"
  "AMDILGenInstrInfo.inc.tmp"
  "AMDILGenAsmWriter.inc.tmp"
  "AMDILGenDAGISel.inc.tmp"
  "AMDILGenCallingConv.inc.tmp"
  "AMDILGenSubtarget.inc.tmp"
  "AMDILGenEDInfo.inc.tmp"
  "AMDILGenIntrinsics.inc.tmp"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/AMDILCommonTableGen.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)