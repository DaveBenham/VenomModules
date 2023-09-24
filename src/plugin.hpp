// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelBernoulliSwitch;
extern Model* modelBernoulliSwitchExpander;
extern Model* modelCloneMerge;
extern Model* modelHQ;
extern Model* modelMix4;
extern Model* modelMix4Stereo;
extern Model* modelMixFade;
extern Model* modelMixFade2;
extern Model* modelMixMute;
extern Model* modelMixOffset;
extern Model* modelMixPan;
extern Model* modelMixSend;
extern Model* modelMixSolo;
extern Model* modelPolyClone;
extern Model* modelPolyUnison;
extern Model* modelRecurse;
extern Model* modelRecurseStereo;
extern Model* modelReformation;
extern Model* modelRhythmExplorer;
extern Model* modelShapedVCA;
extern Model* modelVCAMix4;
extern Model* modelVCAMix4Stereo;
extern Model* modelVenomBlank;
extern Model* modelWinComp;

////////////////////////////////

static const std::vector<std::string> modThemes = {
  "Venom Default",
  "Ivory",
  "Coal",
  "Earth",
  "Danger"
};

static const std::vector<std::string> themes = {
  "Ivory",
  "Coal",
  "Earth",
  "Danger"
};

inline std::string faceplatePath(std::string mod, std::string theme = "Ivory") {
  return "res/"+theme+"/"+mod+"_"+theme+".svg";
}

int getDefaultTheme();
int getDefaultDarkTheme();
void setDefaultTheme(int theme);
void setDefaultDarkTheme(int theme);

struct VenomModule : Module {

  int currentTheme = 0;
  int defaultTheme = getDefaultTheme();
  int defaultDarkTheme = getDefaultDarkTheme();
  int prevTheme = -1;
  int prevDarkTheme = -1;
  bool drawn = false;
  bool paramsInitialized = false;
  bool extProcNeeded = true;
  std::string moduleName = "";

  std::string currentThemeStr(bool dark=false){
    return modThemes[currentTheme==0 ? (dark ? defaultDarkTheme : defaultTheme)+1 : currentTheme];
  }

  bool lockableParams = false;
  void appendParamMenu(Menu* menu, int parmId) {
    ParamQuantity* q = paramQuantities[parmId];
    ParamExtension* e = &paramExtensions[parmId];
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolMenuItem("Lock parameter", "",
      [=]() {
        return e->locked;
      },
      [=](bool val){
        setLock(val, parmId);
      }
    ));
    menu->addChild(createMenuItem("Set default to current value", "",
      [=]() {
        if (e->locked) e->dflt = q->getImmediateValue();
        else q->defaultValue = q->getImmediateValue();
      }
    ));
    if (e->factoryDflt != (e->locked ? e->dflt : q->defaultValue))
      menu->addChild(createMenuItem("Restore factory default", "",
        [=]() {
          if (e->locked) e->dflt = e->factoryDflt;
          else q->defaultValue = e->factoryDflt;
        }
      ));
  }

  struct ParamExtension {
    bool locked;
    bool initLocked;
    bool lockable;
    bool initDfltValid;
    float min, max, dflt, initDflt, factoryDflt;
    ParamExtension(){
      locked = false;
      initLocked = false;
      lockable = false;
      initDfltValid = false;
    }
  };

  void setLock(bool val, int id) {
    ParamExtension* e = &paramExtensions[id];
    if (e->lockable && e->locked != val){
      e->locked = val;
      ParamQuantity* q = paramQuantities[id];
      if (val){
        e->min = q->minValue;
        e->max = q->maxValue;
        e->dflt = q->defaultValue;
        q->name += " (locked)";
        q->minValue = q->maxValue = q->defaultValue = q->getValue();
      }
      else {
        q->name.erase(q->name.length()-9);
        q->minValue = e->min;
        q->maxValue = e->max;
        q->defaultValue = e->dflt;
      }
    }
  }

  void setLockAll(bool val){
    for (int i=0; i<getNumParams(); i++)
      setLock(val, i);
  }

  std::vector<ParamExtension> paramExtensions;

  void venomConfig(int paramCnt, int inCnt, int outCnt, int lightCnt){
    config(paramCnt, inCnt, outCnt, lightCnt);
    for (int i=0; i<paramCnt; i++)
      paramExtensions.push_back(ParamExtension());
  }

  void process(const ProcessArgs& args) override {
    if (drawn && extProcNeeded){
      for (int i=0; i<getNumParams(); i++){
        ParamExtension* e = &paramExtensions[i];
        if (!paramsInitialized){
          ParamQuantity* q = paramQuantities[i];
          e->factoryDflt = q->defaultValue;
          if (e->initDfltValid) q->defaultValue = e->initDflt;
        }
        setLock(e->initLocked, i);
      }
      paramsInitialized = true;
      extProcNeeded = false;
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    for (int i=0; i<getNumParams(); i++){
      ParamExtension* e = &paramExtensions[i];
      std::string nm = "paramLock"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_boolean(e->locked));
      nm = "paramDflt"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_real(e->locked ? e->dflt : paramQuantities[i]->defaultValue));
      nm = "paramVal"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_real(paramQuantities[i]->getImmediateValue()));
    }
    json_object_set_new(rootJ, "currentTheme", json_integer(currentTheme));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val;
    for (int i=0; i<getNumParams(); i++){
      ParamExtension* e = &paramExtensions[i];
      setLock(false, i);
      std::string nm = "paramDflt"+std::to_string(i);
      if ((val = json_object_get(rootJ, nm.c_str()))){
        if (paramsInitialized) {
          paramQuantities[i]->defaultValue = json_real_value(val);
        }
        else {
          e->initDflt = json_real_value(val);
          e->initDfltValid = true;
        }
      }
      nm = "paramVal"+std::to_string(i);
      if ((val = json_object_get(rootJ, nm.c_str())))
        paramQuantities[i]->setImmediateValue(json_real_value(val));
      nm = "paramLock"+std::to_string(i);
      if ((val = json_object_get(rootJ, nm.c_str())))
        e->initLocked = json_boolean_value(val);
    }
    val = json_object_get(rootJ, "currentTheme");
    if (val)
      currentTheme = json_integer_value(val);
    extProcNeeded = true;
    drawn = false;  
  }

};

struct VenomWidget : ModuleWidget {
  std::string moduleName;
  void draw(const DrawArgs & args) override {
    ModuleWidget::draw(args);
    if (module) dynamic_cast<VenomModule*>(this->module)->drawn = true;
  }

  void setVenomPanel(std::string name){
    moduleName = name;
    VenomModule* mod = dynamic_cast<VenomModule*>(this->module);
    if (mod) mod->moduleName = name;
    setPanel(createPanel(
      asset::plugin( pluginInstance, faceplatePath(name, module ? dynamic_cast<VenomModule*>(this->module)->currentThemeStr() : themes[getDefaultTheme()])),
      asset::plugin( pluginInstance, faceplatePath(name, module ? dynamic_cast<VenomModule*>(this->module)->currentThemeStr(true) : themes[getDefaultDarkTheme()]))
    ));
  }

  void appendContextMenu(Menu* menu) override {
    VenomModule* module = dynamic_cast<VenomModule*>(this->module);
    assert(module);

    if (module->lockableParams){
      menu->addChild(new MenuSeparator);
      menu->addChild(createMenuItem("Lock all parameters", "",
        [=]() {
          module->setLockAll(true);
        }
      ));
      menu->addChild(createMenuItem("Unlock all parameters", "",
        [=]() {
          module->setLockAll(false);
        }
      ));
    }

    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "Venom Default Theme",
      themes,
      [=]() {
        return getDefaultTheme();
      },
      [=](int theme) {
        setDefaultTheme(theme);
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Venom Default Dark Theme",
      themes,
      [=]() {
        return getDefaultDarkTheme();
      },
      [=](int theme) {
        setDefaultDarkTheme(theme);
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Theme",
      modThemes,
      [=]() {
        return module->currentTheme;
      },
      [=](int theme) {
        module->currentTheme = theme;
      }
    ));
  }

  void step() override {
    VenomModule* module = dynamic_cast<VenomModule*>(this->module);
    if (module){
      if (module->defaultTheme != getDefaultTheme()){
        module->defaultTheme = getDefaultTheme();
        if(module->currentTheme == 0)
          module->prevTheme = -1;
      }
      if (module->defaultDarkTheme != getDefaultDarkTheme()){
        module->defaultDarkTheme = getDefaultDarkTheme();
        if(module->currentTheme == 0)
          module->prevTheme = -1;
      }
      if (module->prevTheme != module->currentTheme){
        module->prevTheme = module->currentTheme;
        setPanel(createPanel(
          asset::plugin( pluginInstance, faceplatePath(moduleName, module->currentThemeStr())),
          asset::plugin( pluginInstance, faceplatePath( moduleName, module->currentThemeStr(true)))
        ));
      }
    }
    Widget::step();
  }
};

struct FixedSwitchQuantity : SwitchQuantity {
  std::string getDisplayValueString() override {
    return labels[getValue()];
  }
};

template <typename TBase>
struct RotarySwitch : TBase {
  RotarySwitch() {
    this->snap = true;
    this->smooth = false;
  }

  // handle the manually entered values
  void onChange(const event::Change &e) override {
    SvgKnob::onChange(e);
    this->getParamQuantity()->setValue(roundf(this->getParamQuantity()->getValue()));
  }
};

struct CKSSNarrow : app::SvgSwitch {
  CKSSNarrow() {
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/components/SwitchNarrow_0.svg")));
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/components/SwitchNarrow_2.svg")));
  }
};

struct DigitalDisplay : Widget {
  Module* module;
  std::string fontPath;
  std::string bgText;
  std::string text;
  float fontSize;
  NVGcolor bgColor = nvgRGB(0x46,0x46, 0x46);
  NVGcolor fgColor = SCHEME_YELLOW;
  Vec textPos;

  void prepareFont(const DrawArgs& args) {
    // Get font
    std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
    if (!font)
      return;
    nvgFontFaceId(args.vg, font->handle);
    nvgFontSize(args.vg, fontSize);
    nvgTextLetterSpacing(args.vg, 0.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
  }

  void draw(const DrawArgs& args) override {
    // Background
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2);
    nvgFillColor(args.vg, nvgRGB(0x19, 0x19, 0x19));
    nvgFill(args.vg);

    prepareFont(args);

    // Background text
    nvgFillColor(args.vg, bgColor);
    nvgText(args.vg, textPos.x, textPos.y, bgText.c_str(), NULL);
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1 && (!module || (module && !module->isBypassed()))) {
      prepareFont(args);

      // Foreground text
      nvgFillColor(args.vg, fgColor);
      nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
    }
    Widget::drawLayer(args, layer);
  }

  void step() override {
  }
};

struct DigitalDisplay18 : DigitalDisplay {
  DigitalDisplay18() {
    fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
    textPos = Vec(22, 20);
    bgText = "18";
    fontSize = 16;
    box.size = mm2px(Vec(8.197, 8.197)); // 31px square
  }
};

struct DigitalDisplay188 : DigitalDisplay {
  DigitalDisplay188() {
    fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
    textPos = Vec(33.5, 20);
    bgText = "188";
    fontSize = 16;
    box.size = mm2px(Vec(12, 8.197));
  }
};

template <class TWidget>
TWidget* createLockableParam(math::Vec pos, engine::Module* module, int paramId){
  if (module){
    VenomModule* mod = dynamic_cast<VenomModule*>(module);
    mod->lockableParams = true;
    mod->paramExtensions[paramId].lockable = true;
  }
  return createParam<TWidget>(pos, module, paramId);
}

template <class TWidget>
TWidget* createLockableParamCentered(math::Vec pos, engine::Module* module, int paramId){
  if (module){
    VenomModule* mod = dynamic_cast<VenomModule*>(module);
    mod->lockableParams = true;
    mod->paramExtensions[paramId].lockable = true;
  }
  return createParamCentered<TWidget>(pos, module, paramId);
}

template <class TWidget>
TWidget* createLockableLightParamCentered(math::Vec pos, engine::Module* module, int paramId, int firstLightId){
  if (module){
    VenomModule* mod = dynamic_cast<VenomModule*>(module);
    mod->lockableParams = true;
    mod->paramExtensions[paramId].lockable = true;
  }
  return createLightParamCentered<TWidget>(pos, module, paramId, firstLightId);
}

template <typename TBase = GrayModuleLightWidget>
struct YlwLight : TBase {
  YlwLight() {
    this->addBaseColor(SCHEME_YELLOW);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct GrnLight : TBase {
  GrnLight() {
    this->addBaseColor(SCHEME_GREEN);
  }
};
template <typename TBase = GrayModuleLightWidget>
struct BluLight : TBase {
  BluLight() {
    this->addBaseColor(SCHEME_BLUE);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct RdLight : TBase {
  RdLight() {
    this->addBaseColor(SCHEME_RED);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct YellowBlueLight : TBase {
  YellowBlueLight() {
    this->addBaseColor(SCHEME_YELLOW);
    this->addBaseColor(SCHEME_BLUE);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct RedBlueLight : TBase {
  RedBlueLight() {
    this->addBaseColor(SCHEME_RED);
    this->addBaseColor(SCHEME_BLUE);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct YellowRedLight : TBase {
  YellowRedLight() {
    this->addBaseColor(SCHEME_YELLOW);
    this->addBaseColor(SCHEME_RED);
  }
};

struct VCVSliderLockable : VCVSlider {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

template <typename TLightBase = RedLight>
struct VCVLightSliderLockable : LightSlider<VCVSliderLockable, VCVSliderLight<TLightBase>> {
  VCVLightSliderLockable() {}
};

struct GlowingSvgSwitch : app::SvgSwitch {
  GlowingSvgSwitch(){
    shadow->opacity = 0.0;
  }
  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer==1) {
      if (module && !module->isBypassed()) {
        draw(args);
      }
    }
    app::SvgSwitch::drawLayer(args, layer);
  }
};

struct GlowingSvgSwitchLockable : GlowingSvgSwitch {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundHugeBlackKnobLockable : RoundHugeBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundBigBlackKnobLockable : RoundBigBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundLargeBlackKnobLockable : RoundLargeBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundBlackKnobLockable : RoundBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundSmallBlackKnobLockable : RoundSmallBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct TrimpotLockable : Trimpot {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct CKSSThreeLockable : CKSSThree {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct CKSSThreeHorizontalLockable : CKSSThreeHorizontal {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

template <typename TLightBase = WhiteLight>
struct VCVLightBezelLockable : VCVLightBezel<TLightBase> {
  void appendContextMenu(Menu* menu) override {
    if (this->module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

template <typename TLightBase = WhiteLight>
struct VCVLightBezelLatchLockable : VCVLightBezelLatch<TLightBase> {
  void appendContextMenu(Menu* menu) override {
    if (this->module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

template <typename TLight>
struct VCVLightButtonLockable : VCVButton {
  app::ModuleLightWidget* light;

  VCVLightButtonLockable() {
    light = new TLight;
    // Move center of light to center of box
    light->box.pos = box.size.div(2).minus(light->box.size.div(2));
    addChild(light);
  }

  app::ModuleLightWidget* getLight() {
    return light;
  }

  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct ShapeQuantity : ParamQuantity {
  std::string getUnit() override {
    float val = this->getValue();
    return val > 0.f ? "% log" : val < 0.f ? "% exp" : " = linear";
  }  
};
