//===----------------------------------------------------------------------===//
// AMDIL processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
: Processor<Name, NoItineraries, Features>;

def : Proc<"cedar",      [FeatureByteAddress, FeatureImages]>;
def : Proc<"redwood",    [FeatureByteAddress, FeatureImages]>;
def : Proc<"juniper",    [FeatureByteAddress, FeatureImages]>;
def : Proc<"cypress",    [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"barts",      [FeatureByteAddress, FeatureImages]>;
def : Proc<"turks",      [FeatureByteAddress, FeatureImages]>;
def : Proc<"caicos",     [FeatureByteAddress, FeatureImages]>;
def : Proc<"kauai",      [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"cayman",     [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"tahiti",     [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"pitcairn",   [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"capeverde",  [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"trinity",    [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"oland",      [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"hainan",     [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"bonaire",    [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"spectre",    [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"spooky",     [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"kalindi",    [FeatureByteAddress, FeatureImages, FeatureFP64]>;
def : Proc<"hawaii",     [FeatureByteAddress, FeatureImages, FeatureFP64]>;

include "Internal/InternalProcessors.td"
