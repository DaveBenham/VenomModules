// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3 

#include "plugin.hpp"

Plugin* pluginInstance;

void readDefaultThemes();

void init(Plugin* p) {
  pluginInstance = p;

  // Add modules here
  p->addModel(modelVenomAD_ASR);
  p->addModel(modelVenomAuxClone);
  p->addModel(modelVenomBayInput);
  p->addModel(modelVenomBayNorm);
  p->addModel(modelVenomBayOutput);
  p->addModel(modelVenomBenjolinOsc);
  p->addModel(modelVenomBenjolinGatesExpander);
  p->addModel(modelVenomBenjolinVoltsExpander);
  p->addModel(modelVenomBernoulliSwitch);
  p->addModel(modelVenomBernoulliSwitchExpander);
  p->addModel(modelVenomBlocker);
  p->addModel(modelVenomBypass);
  p->addModel(modelVenomCloneMerge);
  p->addModel(modelVenomCompare2);
  p->addModel(modelVenomCrossFade3D);
  p->addModel(modelVenomHQ);
  p->addModel(modelVenomKnob5);
  p->addModel(modelVenomLinearBeats);
  p->addModel(modelVenomLinearBeatsExpander);
  p->addModel(modelVenomLogic);
  p->addModel(modelVenomMix4);
  p->addModel(modelVenomMix4Stereo);
  p->addModel(modelVenomMixFade);
  p->addModel(modelVenomMixFade2);
  p->addModel(modelVenomMixMute);
  p->addModel(modelVenomMixOffset);
  p->addModel(modelVenomMixPan);
  p->addModel(modelVenomMixSend);
  p->addModel(modelVenomMixSolo);
  p->addModel(modelVenomMousePad);
  p->addModel(modelVenomMultiMerge);
  p->addModel(modelVenomMultiSplit);
  p->addModel(modelVenomOscillator);
  p->addModel(modelVenomNORS_IQ);
  p->addModel(modelVenomNORSIQChord2Scale);
  p->addModel(modelVenomPan3D);
  p->addModel(modelVenomPolyClone);
  p->addModel(modelVenomPolyFade);
  p->addModel(modelVenomPolyOffset);
  p->addModel(modelVenomPolySHASR);
  p->addModel(modelVenomPolyScale);
  p->addModel(modelVenomPolyUnison);
  p->addModel(modelVenomPush5);
  p->addModel(modelVenomQuadVCPolarizer);
  p->addModel(modelVenomRecurse);
  p->addModel(modelVenomRecurseStereo);
  p->addModel(modelVenomReformation);
  p->addModel(modelVenomRhythmExplorer);
  p->addModel(modelVenomShapedVCA);
  p->addModel(modelVenomSlew);
  p->addModel(modelVenomSphereToXYZ);
  p->addModel(modelVenomThru);
  p->addModel(modelVenomVCAMix4);
  p->addModel(modelVenomVCAMix4Stereo);
  p->addModel(modelVenomVCOUnit);
  p->addModel(modelVenomBlank);
  p->addModel(modelVenomWaveFolder);
  p->addModel(modelVenomWaveMangler);
  p->addModel(modelVenomWaveMultiplier);
  p->addModel(modelVenomWidgetMenuExtender);
  p->addModel(modelVenomWinComp);
  p->addModel(modelVenomXM_OP);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
  readDefaultThemes();
}
