// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3 

#include "plugin.hpp"

Plugin* pluginInstance;

void readDefaultThemes();

void init(Plugin* p) {
  pluginInstance = p;

  // Add modules here
  p->addModel(modelAD_ASR);
  p->addModel(modelAuxClone);
  p->addModel(modelBayInput);
  p->addModel(modelBayNorm);
  p->addModel(modelBayOutput);
  p->addModel(modelBenjolinOsc);
  p->addModel(modelBenjolinGatesExpander);
  p->addModel(modelBenjolinVoltsExpander);
  p->addModel(modelBernoulliSwitch);
  p->addModel(modelBernoulliSwitchExpander);
  p->addModel(modelBlocker);
  p->addModel(modelBypass);
  p->addModel(modelCloneMerge);
  p->addModel(modelCompare2);
  p->addModel(modelCrossFade3D);
  p->addModel(modelHQ);
  p->addModel(modelKnob5);
  p->addModel(modelLinearBeats);
  p->addModel(modelLinearBeatsExpander);
  p->addModel(modelLogic);
  p->addModel(modelMix4);
  p->addModel(modelMix4Stereo);
  p->addModel(modelMixFade);
  p->addModel(modelMixFade2);
  p->addModel(modelMixMute);
  p->addModel(modelMixOffset);
  p->addModel(modelMixPan);
  p->addModel(modelMixSend);
  p->addModel(modelMixSolo);
  p->addModel(modelMousePad);
  p->addModel(modelMultiMerge);
  p->addModel(modelMultiSplit);
  p->addModel(modelOscillator);
  p->addModel(modelNORS_IQ);
  p->addModel(modelNORSIQChord2Scale);
  p->addModel(modelPan3D);
  p->addModel(modelPolyClone);
  p->addModel(modelPolyFade);
  p->addModel(modelPolyOffset);
  p->addModel(modelPolySHASR);
  p->addModel(modelPolyScale);
  p->addModel(modelPolyUnison);
  p->addModel(modelPush5);
  p->addModel(modelQuadVCPolarizer);
  p->addModel(modelRecurse);
  p->addModel(modelRecurseStereo);
  p->addModel(modelReformation);
  p->addModel(modelRhythmExplorer);
  p->addModel(modelShapedVCA);
  p->addModel(modelSlew);
  p->addModel(modelSphereToXYZ);
  p->addModel(modelThru);
  p->addModel(modelVCAMix4);
  p->addModel(modelVCAMix4Stereo);
  p->addModel(modelVCOUnit);
  p->addModel(modelVenomBlank);
  p->addModel(modelWaveFolder);
  p->addModel(modelWaveMangler);
  p->addModel(modelWaveMultiplier);
  p->addModel(modelWidgetMenuExtender);
  p->addModel(modelWinComp);
  p->addModel(modelXM_OP);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
  readDefaultThemes();
}
