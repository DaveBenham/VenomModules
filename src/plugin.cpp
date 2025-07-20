// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3 

#include "plugin.hpp"

Plugin* pluginInstance;
static std::string venomSettingsFileName = asset::user("Venom.json");
int defaultTheme = 0;
int defaultDarkTheme = 1;

void readDefaultThemes(){
  FILE *file = fopen(venomSettingsFileName.c_str(), "r");
  if (file){
    json_error_t error;
    json_t *rootJ = json_loadf(file, 0, &error);
    json_t *jsonVal = json_object_get(rootJ, "defaultTheme");
    if (jsonVal)
      defaultTheme = json_integer_value(jsonVal);
    jsonVal = json_object_get(rootJ, "defaultDarkTheme");
    if (jsonVal)
      defaultDarkTheme = json_integer_value(jsonVal);
    fclose(file);
    json_decref(rootJ);
  }
}

void writeDefaultThemes(){
  FILE *file = fopen(venomSettingsFileName.c_str(), "w");
  if (file){
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "defaultTheme", json_integer(defaultTheme));
    json_object_set_new(rootJ, "defaultDarkTheme", json_integer(defaultDarkTheme));
    json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
    fclose(file);
    json_decref(rootJ);
  }
}

void setDefaultTheme(int theme){
  if (defaultTheme != theme){
    defaultTheme = theme;
    writeDefaultThemes();
  }
}

void setDefaultDarkTheme(int theme){
  if (defaultDarkTheme != theme){
    defaultDarkTheme = theme;
    writeDefaultThemes();
  }
}

int getDefaultTheme(){
  return defaultTheme;
}

int getDefaultDarkTheme(){
  return defaultDarkTheme;
}

void init(Plugin* p) {
  pluginInstance = p;

  // Add modules here
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
  p->addModel(modelSphereToXYZ);
  p->addModel(modelShapedVCA);
  p->addModel(modelThru);
  p->addModel(modelVCAMix4);
  p->addModel(modelVCAMix4Stereo);
  p->addModel(modelVCOUnit);
  p->addModel(modelVenomBlank);
  p->addModel(modelWaveFolder);
  p->addModel(modelWaveMultiplier);
  p->addModel(modelWidgetMenuExtender);
  p->addModel(modelWinComp);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
  readDefaultThemes();
  
}
