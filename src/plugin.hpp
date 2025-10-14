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
extern Model* modelSVF;
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

// MenuTextField extracted from pachde1 components.hpp
// Textfield as menu item, originally adapted from SubmarineFree
struct MenuTextField : ui::TextField {
  std::function<void(std::string)> changeHandler;
  std::function<void(std::string)> commitHandler;
  void step() override {
    // Keep selected
    APP->event->setSelectedWidget(this);
    TextField::step();
  }
  void setText(const std::string& text) {
    this->text = text;
    selectAll();
  }

  void onChange(const ChangeEvent& e) override {
    ui::TextField::onChange(e);
    if (changeHandler) { 
      changeHandler(text);
    }
  }

  void onSelectKey(const event::SelectKey &e) override {
    if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
      if (commitHandler) {
        commitHandler(text);
      }
      ui::MenuOverlay *overlay = getAncestorOfType<ui::MenuOverlay>();
      overlay->requestDelete();
      e.consume(this);
    }
    if (!e.getTarget())
      TextField::onSelectKey(e);
  }
};

struct VenomModule : Module {

  int currentTheme = 0;
  int defaultTheme = getDefaultTheme();
  int defaultDarkTheme = getDefaultDarkTheme();
  int prevTheme = -1;
  int prevDarkTheme = -1;
  int oversampleStages = 0; // default to 0 = unused
  virtual void setOversample(){};
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
    PortInfo* pi = NULL;
    PortExtension* pe = NULL;
    if (e->nameLink >= 0){
      pi = e->inputLink ? inputInfos[e->nameLink] : outputInfos[e->nameLink];
      pe = e->inputLink ? &inputExtensions[e->nameLink] : &outputExtensions[e->nameLink];
    }
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Parameter name", q->name,
      [=](Menu *menu){
        MenuTextField *editField = new MenuTextField();
        editField->box.size.x = 250;
        editField->setText(q->name);
        editField->changeHandler = [=](std::string text) {
          q->name = text;
          if (pi) pi->name = text;
        };
        menu->addChild(editField);
      }
    ));
    if (!e->factoryName.size())
      e->factoryName = q->name;
    else if (e->factoryName != q->name) {
      menu->addChild(createMenuItem("Restore factory name: "+e->factoryName, "",
        [=]() {
          q->name = e->factoryName;
          if (pi) pi->name = e->factoryName;
        }
      ));
    }  
    if (pe && !pe->factoryName.size())
      pe->factoryName = e->factoryName;
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
    if (e->factoryDflt != (e->locked ? e->dflt : q->defaultValue)){
      menu->addChild(createMenuItem("Restore factory default", "",
        [=]() {
          if (e->locked) e->dflt = e->factoryDflt;
          else q->defaultValue = e->factoryDflt;
        }
      ));
    }
  }
  
  void appendPortMenu(Menu *menu, engine::Port::Type type, int portId){
    PortInfo* pi = (type==engine::Port::INPUT ? inputInfos[portId] : outputInfos[portId]);
    PortExtension* e = (type==engine::Port::INPUT ? &inputExtensions[portId] : &outputExtensions[portId]);
    ParamQuantity* q = NULL;
    ParamExtension* qe = NULL;
    PortInfo* piLink = NULL;
    PortExtension* eLink = NULL;
    if (e->nameLink >= 0){
      q = paramQuantities[e->nameLink];
      qe = &paramExtensions[e->nameLink];
    }
    if (e->portNameLink >= 0){
      piLink = (type==engine::Port::INPUT ? outputInfos[e->portNameLink] : inputInfos[e->portNameLink]);
      eLink = (type==engine::Port::INPUT ? &outputExtensions[e->portNameLink] : &inputExtensions[e->portNameLink]);
    }
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Port name", pi->name,
      [=](Menu *menu){
        MenuTextField *editField = new MenuTextField();
        editField->box.size.x = 250;
        editField->setText(pi->name);
        editField->changeHandler = [=](std::string text) {
          pi->name = text;
          if (q) q->name = text;
          if (piLink) {
            if (!eLink->factoryName.size())
              eLink->factoryName = piLink->name;
            piLink->name = text;
          }
        };
        menu->addChild(editField);
      }
    ));
    if (qe && !qe->factoryName.size())
      qe->factoryName = pi->name;
    if (!e->factoryName.size())
      e->factoryName = pi->name;
    else if (e->factoryName != pi->name) {
      menu->addChild(createMenuItem("Restore factory name: "+e->factoryName, "",
        [=]() {
          pi->name = e->factoryName;
          if (q) q->name = e->factoryName;
          if (piLink) piLink->name = e->factoryName;
        }
      ));
    }  
  }

  struct ParamExtension {
    bool locked;
    bool initLocked;
    bool lockable;
    bool initDfltValid;
    bool inputLink;
    int  nameLink;
    float min, max, dflt, initDflt, factoryDflt;
    std::string factoryName;
    ParamExtension(){
      locked = false;
      initLocked = false;
      lockable = false;
      initDfltValid = false;
      factoryName = "";
      inputLink = false;
      nameLink = -1;
    }
  };
  
  struct PortExtension {
    int nameLink;
    int portNameLink;
    std::string factoryName;
    PortExtension(){
      factoryName = "";
      nameLink = -1;
      portNameLink = -1;
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
        q->description = "Locked";
        q->minValue = q->maxValue = q->defaultValue = q->getValue();
      }
      else {
        q->description = "";
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
  std::vector<PortExtension> inputExtensions;
  std::vector<PortExtension> outputExtensions;

  void venomConfig(int paramCnt, int inCnt, int outCnt, int lightCnt){
    config(paramCnt, inCnt, outCnt, lightCnt);
    for (int i=0; i<paramCnt; i++)
      paramExtensions.push_back(ParamExtension());
    for (int i=0; i<inCnt; i++)
      inputExtensions.push_back(PortExtension());
    for (int i=0; i<outCnt; i++)
      outputExtensions.push_back(PortExtension());
  }
  
  // Hack workaround for VCV bug when deleting a module - failure to trigger onExpanderChange()
  // Remove if/when VCV fixes the bug
  void onRemove(const RemoveEvent& e) override {
    if (rack::string::Version("2.5.0") < rack::string::Version(rack::APP_VERSION)) return;
    Module::ExpanderChangeEvent event;
    Module::Expander expander = getRightExpander();
    if (expander.module){
      expander.module->getLeftExpander().module = NULL;
      expander.module->getLeftExpander().moduleId = -1;
      event.side = 0;
      expander.module->onExpanderChange(event);
    }
    expander = getLeftExpander();
    if (expander.module){
      expander.module->getRightExpander().module = NULL;
      expander.module->getRightExpander().moduleId = -1;
      event.side = 1;
      expander.module->onExpanderChange(event);
    }
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
      initialPostDrawnProcess();
      paramsInitialized = true;
      extProcNeeded = false;
    }
  }
  
  virtual void initialPostDrawnProcess(){}

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    for (int i=0; i<getNumParams(); i++){
      ParamExtension* e = &paramExtensions[i];
      ParamQuantity* pq = paramQuantities[i];
      std::string idStr = std::to_string(i);
      std::string nm = "paramLock"+idStr;
      json_object_set_new(rootJ, nm.c_str(), json_boolean(e->locked));
      nm = "paramDflt"+idStr;
      json_object_set_new(rootJ, nm.c_str(), json_real(e->locked ? e->dflt : pq->defaultValue));
      nm = "paramVal"+idStr;
      json_object_set_new(rootJ, nm.c_str(), json_real(pq->getImmediateValue()));
      nm = "paramName"+idStr;
      json_object_set_new(rootJ, nm.c_str(), json_string(pq->name.c_str()));
    }
    for (int i=0; i<getNumInputs(); i++){
      PortInfo* pi = inputInfos[i];
      std::string nm = "inputName"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_string(pi->name.c_str()));
    }
    for (int i=0; i<getNumOutputs(); i++){
      PortInfo* pi = outputInfos[i];
      std::string nm = "outputName"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_string(pi->name.c_str()));
    }
    json_object_set_new(rootJ, "currentTheme", json_integer(currentTheme));
    if (oversampleStages)
      json_object_set_new(rootJ, "oversampleStages", json_integer(oversampleStages));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val;
    for (int i=0; i<getNumParams(); i++){
      ParamExtension* e = &paramExtensions[i];
      ParamQuantity* pq = paramQuantities[i];
      setLock(false, i);
      std::string idStr = std::to_string(i);
      std::string nm = "paramDflt"+idStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        if (paramsInitialized) {
          pq->defaultValue = json_real_value(val);
        }
        else {
          e->initDflt = json_real_value(val);
          e->initDfltValid = true;
        }
      }
      nm = "paramVal"+idStr;
      if ((val = json_object_get(rootJ, nm.c_str())))
        pq->setImmediateValue(json_real_value(val));
      nm = "paramLock"+idStr;
      if ((val = json_object_get(rootJ, nm.c_str())))
        e->initLocked = json_boolean_value(val);
      if (!e->factoryName.size())
        e->factoryName = pq->name;
      nm = "paramName"+idStr;
      if ((val = json_object_get(rootJ, nm.c_str())))
        pq->name = json_string_value(val);
    }
    for (int i=0; i<getNumInputs(); i++){
      PortExtension* e = &inputExtensions[i];
      PortInfo* pi = inputInfos[i];
      std::string nm = "inputName"+std::to_string(i);
      if (!e->factoryName.size())
        e->factoryName = pi->name;
      if ((val = json_object_get(rootJ, nm.c_str())))
        pi->name = json_string_value(val);
    }
    for (int i=0; i<getNumOutputs(); i++){
      PortExtension* e = &outputExtensions[i];
      PortInfo* pi = outputInfos[i];
      std::string nm = "outputName"+std::to_string(i);
      if (!e->factoryName.size())
        e->factoryName = pi->name;
      if ((val = json_object_get(rootJ, nm.c_str())))
        pi->name = json_string_value(val);
    }
    val = json_object_get(rootJ, "currentTheme");
    if (val)
      currentTheme = json_integer_value(val);
    extProcNeeded = true;
    drawn = false;
    if (oversampleStages) {
      val = json_object_get(rootJ, "oversampleStages");
      oversampleStages = val ? json_integer_value(val) : 3;
    }
  }

};

struct VenomWidget : ModuleWidget {
  std::string moduleName;
  void draw(const DrawArgs & args) override {
    ModuleWidget::draw(args);
    if (module) static_cast<VenomModule*>(this->module)->drawn = true;
  }

  void setVenomPanel(std::string name){
    moduleName = name;
    VenomModule* mod = this->module ? static_cast<VenomModule*>(this->module) : NULL;
    if (mod) mod->moduleName = name;
    setPanel(createPanel(
      asset::plugin( pluginInstance, faceplatePath(name, mod ? mod->currentThemeStr() : themes[getDefaultTheme()])),
      asset::plugin( pluginInstance, faceplatePath(name, mod ? mod->currentThemeStr(true) : themes[getDefaultDarkTheme()]))
    ));
  }

  void appendContextMenu(Menu* menu) override {
    VenomModule* module = static_cast<VenomModule*>(this->module);
    
    if (module->oversampleStages){
      menu->addChild(new MenuSeparator);
      menu->addChild(createIndexSubmenuItem("Oversample filter quality",
        {"6th order", "8th order", "10th order"},
        [=]() {
          return module->oversampleStages - 3;
        },
        [=](int val) {
          module->oversampleStages = val + 3;
          module->setOversample();
        }
      ));
    }

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
  
  void addExpander( Model* model, ModuleWidget* parentModWidget, bool left = false ) {
    Module* module = model->createModule();
    APP->engine->addModule(module);
    ModuleWidget* modWidget = model->createModuleWidget(module);
    APP->scene->rack->setModulePosForce( modWidget, Vec( parentModWidget->box.pos.x + (left ? -modWidget->box.size.x : parentModWidget->box.size.x), parentModWidget->box.pos.y));
    APP->scene->rack->addModule(modWidget);
    history::ModuleAdd* h = new history::ModuleAdd;
    h->name = "create "+model->name;
    h->setModule(modWidget);
    APP->history->push(h);
  }  

};

struct FixedSwitchQuantity : SwitchQuantity {
  std::string getDisplayValueString() override {
    return labels[getImmediateValue()];
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
struct WhiteYellowRedLight : TBase {
  WhiteYellowRedLight() {
    this->addBaseColor(SCHEME_WHITE);
    this->addBaseColor(SCHEME_YELLOW);
    this->addBaseColor(SCHEME_RED);
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

struct RoundTinyBlackKnobLockable : RoundKnob {
  RoundTinyBlackKnobLockable() {
    setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundTinyBlackKnob.svg")));
    bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundTinyBlackKnob_bg.svg")));
  }
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

struct CKSSLockable : CKSS {
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

template <typename TLight = WhiteLight>
struct VCVLightButtonLockable : VCVLightButton<TLight> {
  void appendContextMenu(Menu* menu) override {
    if (this->module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

template <typename TLight = WhiteLight>
struct VCVLightButtonLatchLockable : VCVLightLatch<TLight> {
  void appendContextMenu(Menu* menu) override {
    if (this->module)
      dynamic_cast<VenomModule*>(this->module)->appendParamMenu(menu, this->paramId);
  }
};

struct GlowingTinyButtonLockable : GlowingSvgSwitchLockable {
  GlowingTinyButtonLockable() {
    momentary = true;
    addFrame(Svg::load(asset::plugin( pluginInstance, "res/smallOffButtonSwitch.svg")));
    addFrame(Svg::load(asset::plugin( pluginInstance, "res/smallWhiteButtonSwitch.svg")));
  }
};
 
struct GlowingTinyButtonLatchLockable : GlowingTinyButtonLockable {
  GlowingTinyButtonLatchLockable() {
    momentary = false;
    latch = true;
  }
};
 
struct ShapeQuantity : ParamQuantity {
  std::string getUnit() override {
    float val = this->getValue();
    return val > 0.f ? "% log" : val < 0.f ? "% exp" : " = linear";
  }  
};

struct PolyPJ301MPort : app::SvgPort {
  PolyPJ301MPort() {
    setSvg(Svg::load(asset::plugin( pluginInstance, "res/PJ301M-poly.svg")));
  }
};

struct VenomPort : app::SvgPort {
  void appendContextMenu(Menu* menu) override {
    if (this->module)
      dynamic_cast<VenomModule*>(this->module)->appendPortMenu(menu, this->type, this->portId);
  }
};

struct MonoPort : VenomPort {
  MonoPort() {
    setSvg(Svg::load(asset::system("res/ComponentLibrary/PJ301M.svg")));
  }
};

struct PolyPort : VenomPort {
  PolyPort() {
    setSvg(Svg::load(asset::plugin( pluginInstance, "res/PJ301M-poly.svg")));
  }
};
