#include "plugin.hpp"

struct BayInput;

struct BayModule : VenomModule {

  enum BayParamId {
    PARAMS_LEN
  };
  
  enum BayInputId {
    ENUMS(POLY_INPUT, 8),
    INPUTS_LEN
  };
  
  enum BayOutputId {
    ENUMS(POLY_OUTPUT, 8),
    OUTPUTS_LEN
  };
  
  enum BayLightId {
    LIGHTS_LEN
  };
  
  static std::map<int64_t, BayInput*> sources;
  std::string modName;
  std::string defaultPortName[8]{"Port 1","Port 2","Port 3","Port 4","Port 5","Port 6","Port 7","Port 8"};
  std::string defaultNormalName[8]{"Port 1 normal","Port 2 normal","Port 3 normal","Port 4 normal","Port 5 normal","Port 6 normal","Port 7 normal","Port 8 normal"};

};

struct BayInput : BayModule {

  int64_t oldId = -1;
  bool loadComplete = false;

  BayInput() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, 0, LIGHTS_LEN);
    for (int i=0; i < INPUTS_LEN; i++) {
      configInput(POLY_INPUT+i, defaultPortName[i]);
    }
    modName = "Bay Input";
  }

  void onAdd(const AddEvent& e) override {
    sources[id] = this;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oldId != -1){
      if (loadComplete)
        oldId = -1;
      else
        loadComplete = true;
    }
  }

  void onRemove(const RemoveEvent& e) override {
    sources.erase(id);
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "oldId", json_integer(id));
    json_object_set_new(rootJ, "modName", json_string(modName.c_str()));
    return rootJ;
  
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "oldId")))
      oldId = json_integer_value(val);
    if ((val = json_object_get(rootJ, "modName")))
      modName = json_string_value(val);
  }

};

struct BayOutputModule : BayModule {
  
  int64_t srcId = -1;
  BayInput* srcMod = NULL;
  
  std::vector<BayInput*> bayInputs{};
  std::vector<int64_t> bayInputIds{};

  int bayOutputType = 0;
  bool zeroChannel = false;
  
  dsp::ClockDivider clockDivider;

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (!srcMod && srcId >= 0) {
      if (sources.count(srcId) && sources[srcId]->oldId==srcId) {
        srcMod = sources[srcId];
      }
      else {
        for (auto const& it : sources) {
          if (it.second->oldId == srcId) {
            srcMod = it.second;
            srcId = srcMod->id;
            break;
          }
        }
        if (!srcMod)
          srcId = -1;
      }  
    }
    if (srcMod && !sources.count(srcId)) {
      srcId = -1;
      srcMod = NULL;
    }
    if (clockDivider.process())
      propagateSrcLabels();
  }  

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "modName", json_string(modName.c_str()));
    if (sources.count(srcId))
      json_object_set_new(rootJ, "srcId", json_integer(srcId));
    json_object_set_new(rootJ, "zeroChannel", json_boolean(zeroChannel));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "modName")))
      modName = json_string_value(val);
    if ((val = json_object_get(rootJ, "srcId")))
      srcId = json_integer_value(val);
    if ((val = json_object_get(rootJ, "zeroChannel")))
      zeroChannel = json_boolean_value(val);
  }

  void appendWidgetContextMenu(Menu* menu) {
    std::vector<std::string> labels{"None"};
    bayInputs.clear();
    bayInputIds.clear();
    for (auto const& it : sources){
      if (APP->engine->getModule(it.first)) {
        labels.push_back(string::f("%s (%lld)", it.second->modName.c_str(), static_cast<long long int>(it.first)));
        bayInputIds.push_back(it.first);
        bayInputs.push_back(it.second);
      }
    }
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem(bayOutputType ? "Bay Norm name" : "Bay Output name", modName,
      [=](Menu *menu){
        MenuTextField *editField = new MenuTextField();
        editField->box.size.x = 250;
        editField->setText(modName);
        editField->changeHandler = [=](std::string text) {
          modName = text;
        };
        menu->addChild(editField);
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      bayOutputType ? "Bay Norm source" : "Bay Output source",
      labels,
      [=]() {
        if (sources.count(srcId)) {
          for (unsigned i=0; i<bayInputs.size(); i++) {
            if (bayInputs[i] == srcMod)
              return static_cast<int>(i+1);
          }
        }
        return 0;
      },
      [=](int val) {
        if (val) {
          srcMod = bayInputs[val-1];
          srcId = bayInputIds[val-1];
        }
        else {
          srcId = -1;
          srcMod = NULL;
        }
      }
    ));
    menu->addChild(createBoolMenuItem("Enable 0 channel output", "",
      [=]() {
        return zeroChannel;
      },
      [=](bool val){
        zeroChannel = val;
      }
    ));
  }
  
  void propagateSrcLabels() {
    for (int i=0; i<OUTPUTS_LEN; i++) {
      PortInfo* oi = outputInfos[i];
      PortExtension* oe = &outputExtensions[i];
      bool propagate = (oi->name == oe->factoryName);
      if (srcMod) {
        if (oe->factoryName != srcMod->inputInfos[i]->name) {
          oe->factoryName = srcMod->inputInfos[i]->name;
          if (propagate)
            oi->name = oe->factoryName;
        }
      }
      else {
        if (oe->factoryName != defaultPortName[i]) {
          oe->factoryName = defaultPortName[i];
          if (propagate)
            oi->name = oe->factoryName;
        }
      }
      if (bayOutputType) {
        PortInfo* ii = inputInfos[i];
        PortExtension* ie = &inputExtensions[i];
        propagate = (ii->name == ie->factoryName);
        int ilen = ie->factoryName.length();
        int olen = oi->name.length();
        std::string suffix=" normal";
        if (ilen != olen+7 || ie->factoryName.compare(0,olen,oi->name) || ie->factoryName.compare(olen,7,suffix)) {
          ie->factoryName = oi->name + suffix;
          if (propagate)
            ii->name = ie->factoryName;
        }
      }  
    }
  }

};

struct BayOutputModuleWidget : VenomWidget {

  void appendContextMenu(Menu* menu) override {
    BayOutputModule* thisMod = static_cast<BayOutputModule*>(this->module);
    thisMod->appendWidgetContextMenu(menu);
    VenomWidget::appendContextMenu(menu);
  }

};

struct BayOutputLabelsWidget : widget::Widget {
  BayOutputModule* mod=NULL;
  std::string modName;
  std::string fontPath;

  BayOutputLabelsWidget() {
    fontPath = asset::system("res/fonts/DejaVuSans.ttf");
  }

  void draw(const DrawArgs& args) override {
    if (mod) modName = mod->modName;
    int theme = mod ? mod->currentTheme : 0;
    if (!theme) theme = settings::preferDarkPanels ? getDefaultDarkTheme()+1 : getDefaultTheme()+1;
    std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
    if (!font)
      return;
    nvgFontFaceId(args.vg, font->handle);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
    nvgFontSize(args.vg, 11);
    switch (theme) {
      case 1: // ivory
        nvgFillColor(args.vg, nvgRGB(0x25, 0x25, 0x25));
        break;
      case 2: // coal
        nvgFillColor(args.vg, nvgRGB(0xed, 0xe7, 0xdc));
        break;
      case 3: // earth
        nvgFillColor(args.vg, nvgRGB(0xd2, 0xac, 0x95));
        break;
      default: // danger
        nvgFillColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
        break;
    }
    nvgText(args.vg, 37.5f, 13.f, modName.c_str(), NULL);
    nvgFontSize(args.vg, 9.5);
    switch (theme) {
      case 1: // ivory
        nvgFillColor(args.vg, nvgRGB(0xed, 0xe7, 0xdc));
        break;
      case 2: // coal
        nvgFillColor(args.vg, nvgRGB(0x25, 0x25, 0x25));
        break;
      case 3: // earth
        nvgFillColor(args.vg, nvgRGB(0x42, 0x39, 0x32));
        break;
      default: // danger
        nvgFillColor(args.vg, nvgRGB(0xf2, 0xf2, 0xf2));
        break;
    }
    std::string text;
    for (int i=0; i<BayModule::OUTPUTS_LEN; i++) {
      text = mod ? mod->outputInfos[i]->name : string::f("Port %d", i+1);
      nvgText(args.vg, 37.5f, 31+i*42, text.c_str(), NULL);
    }
  }
};
