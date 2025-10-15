#include "Venom.hpp"

namespace Venom {

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

}