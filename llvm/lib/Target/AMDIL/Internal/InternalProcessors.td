//===----------------------------------------------------------------------===//
// AMDIL processors supported.
//===----------------------------------------------------------------------===//


def : Proc<"owls",       [FeatureByteAddress, FeatureImages, FeatureFP64]>; //<--Internal Only! Tiran
def : Proc<"eagle",      [FeatureByteAddress, FeatureImages, FeatureFP64]>; //<--Internal Only! Maui

// FIXME: Disable 16-bit instructions to workaround integer_ops
// failures when 16-bit instructions are used. Re-enable once the
// register / instruction definitions for 16-bit types aren't insane.
def : Proc<"iceland",    [FeatureByteAddress, FeatureImages, FeatureFP64
                          /*FeatureShortOps, FeatureHalfOps*/]>;
def : Proc<"tonga",      [FeatureByteAddress, FeatureImages, FeatureFP64
                       /* FeatureShortOps, FeatureHalfOps */]>;
def : Proc<"mullins",    [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"goose",      [FeatureByteAddress, FeatureImages, FeatureFP64
                          /* FeatureShortOps, FeatureHalfOps */]>; //<--Internal Only! Bermuda
def : Proc<"hoatzin",    [FeatureByteAddress, FeatureImages, FeatureFP64
                          /*FeatureShortOps, FeatureHalfOps*/ ]>; //<--Internal Only! Fiji
def : Proc<"peacock",    [FeatureByteAddress, FeatureImages, FeatureFP64
                          /*FeatureShortOps, FeatureHalfOps*/ ]>; //<--Internal Only! Carrizo
