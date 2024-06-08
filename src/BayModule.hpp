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

};

struct BayInput : BayModule {

  int64_t oldId = -1;
  int64_t originalOldId = -1;
  bool loadComplete = false;

  BayInput() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, 0, LIGHTS_LEN);
    for (int i=0; i < INPUTS_LEN; i++) {
      configInput(POLY_INPUT+i, string::f("Port %d", i + 1));
    }
  }

  void onAdd(const AddEvent& e) override {
    sources[id] = this;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oldId != -1){
      if (loadComplete){
        originalOldId = oldId;
        oldId = -1;
      }
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
    return rootJ;
  
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "oldId")))
      oldId = json_integer_value(val);
  }

};

struct BayOutputModule : BayModule {
  
  int64_t srcId = -1;
  BayInput* srcMod = NULL;
  
  std::vector<BayInput*> bayInputs{};
  std::vector<int64_t> bayInputIds{};

  int bayOutputType = 0;
  
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
  }  

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    if (sources.count(srcId))
      json_object_set_new(rootJ, "srcId", json_integer(srcId));
    return rootJ;
  
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "srcId")))
      srcId = json_integer_value(val);
  }

  void appendWidgetContextMenu(Menu* menu) {
    std::vector<std::string> labels{"None"};
    bayInputs.clear();
    bayInputIds.clear();
    for (auto const& it : sources){
      if (APP->engine->getModule(it.first)) {
        labels.push_back(std::to_string(it.first));
        bayInputIds.push_back(it.first);
        bayInputs.push_back(it.second);
      }
    }
    menu->addChild(new MenuSeparator);
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
  }

};

struct BayOutputModuleWidget : VenomWidget {

  void appendContextMenu(Menu* menu) override {
    BayOutputModule* thisMod = static_cast<BayOutputModule*>(this->module);
    thisMod->appendWidgetContextMenu(menu);
    VenomWidget::appendContextMenu(menu);
  }

};