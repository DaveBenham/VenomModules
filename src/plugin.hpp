// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelBernoulliSwitch;
extern Model* modelCloneMerge;
extern Model* modelHQ;
extern Model* modelMix4;
extern Model* modelMix4Stereo;
extern Model* modelPolyClone;
extern Model* modelRecurse;
extern Model* modelRecurseStereo;
extern Model* modelRhythmExplorer;
extern Model* modelVCAMix4;
extern Model* modelVCAMix4Stereo;
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
void setDefaultTheme(int theme);

struct VenomModule : Module {

  int currentTheme = 0;
  int defaultTheme = getDefaultTheme();
  int prevTheme = -1;

  std::string currentThemeStr(){
    return modThemes[currentTheme==0 ? defaultTheme+1 : currentTheme];
  }

  bool lockableParams = false;
  void appendParamMenu(Menu* menu, int parmId) {
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolMenuItem("Lock parameter", "",
      [=]() {
        return paramExtensions[parmId].locked;
      },
      [=](bool val){
        setLock(val, parmId);
      }
    ));
  }

  struct ParamExtension {
    bool locked;
    bool initLocked;
    bool lockable;
    float min, max, dflt;
    ParamExtension(){
      locked = false;
      initLocked = false;
      lockable = false;
    }
  };

  void setLock(bool val, int id) {
    if (paramExtensions[id].lockable && paramExtensions[id].locked != val){
      paramExtensions[id].locked = val;
      ParamQuantity* q = paramQuantities[id];
      if (val){
        paramExtensions[id].min = q->minValue;
        paramExtensions[id].max = q->maxValue;
        paramExtensions[id].dflt = q->defaultValue;
        q->name += " (locked)";
        q->minValue = q->maxValue = q->defaultValue = q->getValue();
      }
      else {
        q->name.erase(q->name.length()-9);
        q->minValue = paramExtensions[id].min;
        q->maxValue = paramExtensions[id].max;
        q->defaultValue = paramExtensions[id].dflt;
      }
    }
  }

  void setLockAll(bool val){
    for (int i=0; i<getNumParams(); i++)
      setLock(val, i);
  }

  std::vector<ParamExtension> paramExtensions;
  bool paramInitRequired = false;

  void venomConfig(int paramCnt, int inCnt, int outCnt, int lightCnt){
    config(paramCnt, inCnt, outCnt, lightCnt);
    for (int i=0; i<paramCnt; i++)
      paramExtensions.push_back(ParamExtension());
  }

  void process(const ProcessArgs& args) override {
    if (paramInitRequired){
      paramInitRequired = false;
      for (int i=0; i<getNumParams(); i++){
        setLock(paramExtensions[i].initLocked, i);
      }
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    for (int i=0; i<getNumParams(); i++){
      std::string nm = "paramLock"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_boolean(paramExtensions[i].locked));
    }
    json_object_set_new(rootJ, "currentTheme", json_integer(currentTheme));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val;
    for (int i=0; i<getNumParams(); i++){
      std::string nm = "paramLock"+std::to_string(i);
      if ((val = json_object_get(rootJ, nm.c_str())))
        paramExtensions[i].initLocked = json_boolean_value(val);
    }
    val = json_object_get(rootJ, "currentTheme");
    if (val)
      currentTheme = json_integer_value(val);
  }

};

struct VenomWidget : ModuleWidget {
  bool initialDraw = false;
  std::string moduleName;
  void draw(const DrawArgs & args) override {
    ModuleWidget::draw(args);
    if (!initialDraw && module){
      dynamic_cast<VenomModule*>(this->module)->paramInitRequired = true;
      initialDraw = true;
    }
  }

  void setVenomPanel(std::string name){
    moduleName = name;
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(name, module ? dynamic_cast<VenomModule*>(this->module)->currentThemeStr() : themes[getDefaultTheme()]))));
  }

/*
  VenomWidget(VenomModule* module){
  }
*/

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
      if (module->prevTheme != module->currentTheme){
        module->prevTheme = module->currentTheme;
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, faceplatePath(moduleName, module->currentThemeStr()))));
      }
    }
    Widget::step();
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

struct RoundHugeBlackKnobSnapLockable : RoundHugeBlackKnobLockable {
  RoundHugeBlackKnobSnapLockable() {
    snap = true;
  }
};

struct RoundBigBlackKnobLockable : RoundBigBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundBigBlackKnobSnapLockable : RoundBigBlackKnobLockable {
  RoundBigBlackKnobSnapLockable() {
    snap = true;
  }
};

struct RoundLargeBlackKnobLockable : RoundLargeBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundLargeBlackKnobSnapLockable : RoundLargeBlackKnobLockable {
  RoundLargeBlackKnobSnapLockable() {
    snap = true;
  }
};

struct RoundBlackKnobLockable : RoundBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundBlackKnobSnapLockable : RoundBlackKnobLockable {
  RoundBlackKnobSnapLockable() {
    snap = true;
  }
};

struct RoundSmallBlackKnobLockable : RoundSmallBlackKnob {
  void appendContextMenu(Menu* menu) override {
    if (module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct RoundSmallBlackKnobSnapLockable : RoundSmallBlackKnobLockable {
  RoundSmallBlackKnobSnapLockable() {
    snap = true;
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

