// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

Plugin* pluginInstance;
static std::string venomSettingsFileName = asset::user("Venom.json");
int defaultTheme = 0;

void setDefaultTheme(int theme){
  if (defaultTheme != theme){
    FILE *file = fopen(venomSettingsFileName.c_str(), "w");
    if (file){
      defaultTheme = theme;
      json_t *rootJ = json_object();
      json_object_set_new(rootJ, "defaultTheme", json_integer(theme));
      json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
      fclose(file);
      json_decref(rootJ);
    }
  }
}

void readDefaultTheme(){
  FILE *file = fopen(venomSettingsFileName.c_str(), "r");
  if (file){
    json_error_t error;
    json_t *rootJ = json_loadf(file, 0, &error);
    json_t *jsonVal = json_object_get(rootJ, "defaultTheme");
    if (jsonVal)
      defaultTheme = json_integer_value(jsonVal);
    fclose(file);
    json_decref(rootJ);
  }
}

int getDefaultTheme(){
  return defaultTheme;
}

void init(Plugin* p) {
  pluginInstance = p;

  // Add modules here
  p->addModel(modelBernoulliSwitch);
  p->addModel(modelCloneMerge);
  p->addModel(modelHQ);
  p->addModel(modelMix4);
  p->addModel(modelMix4Stereo);
  p->addModel(modelPolyClone);
  p->addModel(modelRecurse);
  p->addModel(modelRecurseStereo);
  p->addModel(modelRhythmExplorer);
  p->addModel(modelVCAMix4);
  p->addModel(modelVCAMix4Stereo);
  p->addModel(modelWinComp);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
  readDefaultTheme();
}
