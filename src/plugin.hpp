// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelVenomAD_ASR;
extern Model* modelVenomAuxClone;
extern Model* modelVenomBayInput;
extern Model* modelVenomBayNorm;
extern Model* modelVenomBayOutput;
extern Model* modelVenomBenjolinOsc;
extern Model* modelVenomBenjolinGatesExpander;
extern Model* modelVenomBenjolinVoltsExpander;
extern Model* modelVenomBernoulliSwitch;
extern Model* modelVenomBernoulliSwitchExpander;
extern Model* modelVenomBlocker;
extern Model* modelVenomBypass;
extern Model* modelVenomCloneMerge;
extern Model* modelVenomCompare2;
extern Model* modelVenomCrossFade3D;
extern Model* modelVenomHQ;
extern Model* modelVenomKnob5;
extern Model* modelVenomLinearBeats;
extern Model* modelVenomLinearBeatsExpander;
extern Model* modelVenomLogic;
extern Model* modelVenomMix4;
extern Model* modelVenomMix4Stereo;
extern Model* modelVenomMixFade;
extern Model* modelVenomMixFade2;
extern Model* modelVenomMixMute;
extern Model* modelVenomMixOffset;
extern Model* modelVenomMixPan;
extern Model* modelVenomMixSend;
extern Model* modelVenomMixSolo;
extern Model* modelVenomMousePad;
extern Model* modelVenomMultiMerge;
extern Model* modelVenomMultiSplit;
extern Model* modelVenomSVF;
extern Model* modelVenomOscillator;
extern Model* modelVenomNORS_IQ;
extern Model* modelVenomNORSIQChord2Scale;
extern Model* modelVenomPan3D;
extern Model* modelVenomPolyClone;
extern Model* modelVenomPolyFade;
extern Model* modelVenomPolyOffset;
extern Model* modelVenomPolySHASR;
extern Model* modelVenomPolyScale;
extern Model* modelVenomPolyUnison;
extern Model* modelVenomPush5;
extern Model* modelVenomQuadVCPolarizer;
extern Model* modelVenomRecurse;
extern Model* modelVenomRecurseStereo;
extern Model* modelVenomReformation;
extern Model* modelVenomRhythmExplorer;
extern Model* modelVenomShapedVCA;
extern Model* modelVenomSlew;
extern Model* modelVenomSphereToXYZ;
extern Model* modelVenomThru;
extern Model* modelVenomVCAMix4;
extern Model* modelVenomVCAMix4Stereo;
extern Model* modelVenomVCOUnit;
extern Model* modelVenomBlank;
extern Model* modelVenomWaveFolder;
extern Model* modelVenomWaveMangler;
extern Model* modelVenomWaveMultiplier;
extern Model* modelVenomWidgetMenuExtender;
extern Model* modelVenomWinComp;
extern Model* modelVenomXM_OP;
