// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelAD_ASR;
extern Model* modelAuxClone;
extern Model* modelBayInput;
extern Model* modelBayNorm;
extern Model* modelBayOutput;
extern Model* modelBenjolinOsc;
extern Model* modelBenjolinGatesExpander;
extern Model* modelBenjolinVoltsExpander;
extern Model* modelBernoulliSwitch;
extern Model* modelBernoulliSwitchExpander;
extern Model* modelBlocker;
extern Model* modelBypass;
extern Model* modelCloneMerge;
extern Model* modelCompare2;
extern Model* modelCrossFade3D;
extern Model* modelHQ;
extern Model* modelKnob5;
extern Model* modelLinearBeats;
extern Model* modelLinearBeatsExpander;
extern Model* modelLogic;
extern Model* modelMix4;
extern Model* modelMix4Stereo;
extern Model* modelMixFade;
extern Model* modelMixFade2;
extern Model* modelMixMute;
extern Model* modelMixOffset;
extern Model* modelMixPan;
extern Model* modelMixSend;
extern Model* modelMixSolo;
extern Model* modelMousePad;
extern Model* modelMultiMerge;
extern Model* modelMultiSplit;
extern Model* modelOscillator;
extern Model* modelNORS_IQ;
extern Model* modelNORSIQChord2Scale;
extern Model* modelPan3D;
extern Model* modelPolyClone;
extern Model* modelPolyFade;
extern Model* modelPolyOffset;
extern Model* modelPolySHASR;
extern Model* modelPolyScale;
extern Model* modelPolyUnison;
extern Model* modelPush5;
extern Model* modelQuadVCPolarizer;
extern Model* modelRecurse;
extern Model* modelRecurseStereo;
extern Model* modelReformation;
extern Model* modelRhythmExplorer;
extern Model* modelShapedVCA;
extern Model* modelSlew;
extern Model* modelSphereToXYZ;
extern Model* modelThru;
extern Model* modelVCAMix4;
extern Model* modelVCAMix4Stereo;
extern Model* modelVCOUnit;
extern Model* modelVenomBlank;
extern Model* modelWaveFolder;
extern Model* modelWaveMangler;
extern Model* modelWaveMultiplier;
extern Model* modelWidgetMenuExtender;
extern Model* modelWinComp;
extern Model* modelXM_OP;
