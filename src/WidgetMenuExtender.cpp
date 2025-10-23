// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

namespace Venom {

struct WidgetMenuExtender : VenomModule {

  enum ParamId {
    ENABLE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUTS_LEN
  };
  enum LightId {
    ENABLE_RED_LIGHT,
    ENABLE_BLUE_LIGHT,
    LIGHTS_LEN
  };

  bool disable = false;
  dsp::SchmittTrigger trigIn;
  
  struct ParamDefault {
    int64_t modId;
    int id;
    float dflt;
    float factoryDflt;
    
    ParamDefault(int64_t pModId=-1, int pId=-1, float pFactoryDflt=0.f, float pDflt=0.f) {
      modId = pModId;
      id = pId;
      factoryDflt = pFactoryDflt;
      dflt = pDflt;
    }
  };
  ParamDefault* currentDefault = NULL;
  
  typedef std::vector<ParamDefault> DefaultVector;
  
  DefaultVector defaults{};
  
  struct WidgetRename {
    int64_t modId;
    int id;
    std::string factoryName;
    std::string name;
    
    WidgetRename(int64_t pModId=-1, int pId=-1, std::string pFactoryName="", std::string pName="") {
      modId = pModId;
      id = pId;
      factoryName = pFactoryName;
      name = pName;
    }
  };
  WidgetRename* currentRename = NULL;
  
  typedef std::vector<WidgetRename> RenameVector;
  
  RenameVector paramRenames{};
  RenameVector inputRenames{};
  RenameVector outputRenames{};
  
  WidgetMenuExtender() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    std::vector<int64_t> mods = APP->engine->getModuleIds();
    for (uint64_t i=0; i<mods.size(); i++){
      WidgetMenuExtender* mod = dynamic_cast<WidgetMenuExtender*>(APP->engine->getModule(mods[i]));
      if (mod && !mod->disable) {
        disable = true;
        break;
      }
    }
    if (disable)
      configSwitch<FixedSwitchQuantity>(ENABLE_PARAM, 0.f, 0.f, 0.f, "Enable", {"Permanently disabled"});
    else
      configSwitch<FixedSwitchQuantity>(ENABLE_PARAM, 0.f, 1.f, 1.f, "Enable", {"Off", "On"});
  }
  
  ~WidgetMenuExtender() {
    for (uint64_t i=0; i<defaults.size(); i++){
      ParamDefault* d = &defaults[i];
      Module* mod = APP->engine->getModule(d->modId);
      if (!mod) continue;
      ParamQuantity* pq = mod->getParamQuantity(d->id);
      if (!pq || pq->defaultValue != d->dflt) continue;
      pq->defaultValue = d->factoryDflt;
    }
    for (uint64_t i=0; i<paramRenames.size(); i++){
      WidgetRename* wr = &paramRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      ParamQuantity* pq = mod->getParamQuantity(wr->id);
      if (!pq || pq->name != wr->name) continue;
      pq->name = wr->factoryName;
    }
    for (uint64_t i=0; i<inputRenames.size(); i++){
      WidgetRename* wr = &inputRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      PortInfo* pi = mod->getInputInfo(wr->id);
      if (!pi || pi->name != wr->name) continue;
      pi->name = wr->factoryName;
    }
    for (uint64_t i=0; i<outputRenames.size(); i++){
      WidgetRename* wr = &outputRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      PortInfo* pi = mod->getOutputInfo(wr->id);
      if (!pi || pi->name != wr->name) continue;
      pi->name = wr->factoryName;
    }
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
  void extendForeignParameterMenu(plugin::Model* model, ParamWidget* paramWidget, Menu* menu) {
    ParamQuantity* paramQuantity = paramWidget->getParamQuantity();
    currentRename = NULL;
    for (uint64_t i=0; i<paramRenames.size(); i++){
      if (paramRenames[i].modId==paramWidget->module->id && paramRenames[i].id==paramWidget->paramId) currentRename = &paramRenames[i];
    }
    if (currentRename && currentRename->name != paramQuantity->name) {
      currentRename->name = paramQuantity->name;
      currentRename->factoryName = paramQuantity->name;
    }
    currentDefault = NULL;
    for (uint64_t i=0; i<defaults.size(); i++){
      if (defaults[i].modId==paramWidget->module->id && defaults[i].id==paramWidget->paramId) currentDefault = &defaults[i];
    }
    if (currentDefault && currentDefault->dflt != paramQuantity->defaultValue) {
      currentDefault->dflt = paramQuantity->defaultValue;
      currentDefault->factoryDflt = paramQuantity->defaultValue;
    }
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Parameter name", "",
      [=](Menu *menu){
        MenuTextField *editField = new MenuTextField();
        editField->box.size.x = 250;
        editField->setText(paramQuantity->name);
        editField->changeHandler = [=](std::string text) {
          if (!currentRename) {
            WidgetRename* newRename = new WidgetRename(paramWidget->module->id, paramWidget->paramId, paramQuantity->name, text);
            paramRenames.push_back(*newRename);
            currentRename = &paramRenames.back();
            delete newRename;
          }
          else currentRename->name = text;
          paramQuantity->name = text;
        };
        menu->addChild(editField);
      }
    ));
    if (currentRename && currentRename->name != currentRename->factoryName) {
      menu->addChild(createMenuItem("Restore factory name: "+currentRename->factoryName, "",
        [=]() {
          currentRename->name = currentRename->factoryName;
          paramQuantity->name = currentRename->factoryName;
        }
      ));
    }
    menu->addChild(createMenuItem("Set default to current value", "",
      [=]() {
        if (!currentDefault) {
          ParamDefault* newDefault = new ParamDefault(paramWidget->module->id, paramWidget->paramId, paramQuantity->defaultValue, paramQuantity->getImmediateValue());
          defaults.push_back(*newDefault);
          currentDefault = &defaults.back();
          delete newDefault;
        }
        else currentDefault->dflt = paramQuantity->getImmediateValue();
        paramQuantity->defaultValue = paramQuantity->getImmediateValue();
      }
    ));
    if (currentDefault && currentDefault->dflt != currentDefault->factoryDflt){
      menu->addChild(createMenuItem("Restore factory default", "",
        [=]() {
          currentDefault->dflt = currentDefault->factoryDflt;
          paramQuantity->defaultValue = currentDefault->factoryDflt;
        }
      ));
    }
  }
  
  void extendForeignPortMenu(PortWidget* portWidget, Menu* menu) {
    PortInfo* portInfo = portWidget->getPortInfo();
    RenameVector* renames = portInfo->type==engine::Port::INPUT ? &inputRenames : &outputRenames;
    currentRename = NULL;
    for (uint64_t i=0; i<renames->size(); i++){
      if ((*renames)[i].modId==portInfo->module->id && (*renames)[i].id==portInfo->portId) currentRename = &(*renames)[i];
    }
    if (currentRename && currentRename->name != portInfo->name) {
      currentRename->name = portInfo->name;
      currentRename->factoryName = portInfo->name;
    }
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Port name", "",
      [=](Menu *menu){
        MenuTextField *editField = new MenuTextField();
        editField->box.size.x = 250;
        editField->setText(portInfo->name);
        editField->changeHandler = [=](std::string text) {
          if (!currentRename) {
            WidgetRename* newRename = new WidgetRename(portInfo->module->id, portInfo->portId, portInfo->name, text);
            renames->push_back(*newRename);
            currentRename = &renames->back();
            delete newRename;
          }
          else currentRename->name = text;
          portInfo->name = text;
        };
        menu->addChild(editField);
      }
    ));
    if (currentRename && currentRename->name != currentRename->factoryName) {
      menu->addChild(createMenuItem("Restore factory name: "+currentRename->factoryName, "",
        [=]() {
          currentRename->name = currentRename->factoryName;
          portInfo->name = currentRename->factoryName;
        }
      ));
    }
  }
  
  void initialPostDrawnProcess() override { // Called once from VenomModule process()
    for (uint64_t i=0; i<defaults.size(); i++){
      ParamDefault* d = &defaults[i];
      Module* mod = APP->engine->getModule(d->modId);
      if (!mod) continue;
      ParamQuantity* pq = mod->getParamQuantity(d->id);
      if (pq) pq->defaultValue = d->dflt;
    }
    for (uint64_t i=0; i<paramRenames.size(); i++){
      WidgetRename* wr = &paramRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      ParamQuantity* pq = mod->getParamQuantity(wr->id);
      if (pq) pq->name = wr->name;
    }
    for (uint64_t i=0; i<inputRenames.size(); i++){
      WidgetRename* wr = &inputRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      PortInfo* pi = mod->getInputInfo(wr->id);
      if (pi) pi->name = wr->name;
    }
    for (uint64_t i=0; i<outputRenames.size(); i++){
      WidgetRename* wr = &outputRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      PortInfo* pi = mod->getOutputInfo(wr->id);
      if (pi) pi->name = wr->name;
    }
  }

  json_t* json_rename(WidgetRename* wr) {
    json_t* obj = json_object();
    json_object_set_new(obj, "modId", json_integer(wr->modId));
    json_object_set_new(obj, "id", json_integer(wr->id));
    json_object_set_new(obj, "factoryName", json_string(wr->factoryName.c_str()));
    json_object_set_new(obj, "name", json_string(wr->name.c_str()));
    return obj;
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    if (disable) {
      json_object_set_new(rootJ, "disable", json_boolean(true));
      return rootJ;
    }
    json_t* array = NULL;
    for (uint64_t i=0; i<defaults.size(); i++){
      Module* mod = APP->engine->getModule(defaults[i].modId);
      if (!mod) continue;
      ParamQuantity* pq = mod->getParamQuantity(defaults[i].id);
      if (!pq || pq->defaultValue != defaults[i].dflt || pq->defaultValue == defaults[i].factoryDflt) continue;
      if (!array) array = json_array();
      json_t* obj = json_object();
      json_object_set_new(obj, "modId", json_integer(defaults[i].modId));
      json_object_set_new(obj, "id", json_integer(defaults[i].id));
      json_object_set_new(obj, "factoryDflt", json_real(defaults[i].factoryDflt));
      json_object_set_new(obj, "dflt", json_real(defaults[i].dflt));
      json_array_append_new(array, obj);
    }
    if(array) json_object_set_new(rootJ, "defaults", array);
    array = NULL;
    for (uint64_t i=0; i<paramRenames.size(); i++){
      WidgetRename* wr = &paramRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      ParamQuantity* pq = mod->getParamQuantity(wr->id);
      if (!pq || pq->name != wr->name || pq->name == wr->factoryName) continue;
      if (!array) array = json_array();
      json_array_append_new(array, json_rename(wr));
    }
    if(array) json_object_set_new(rootJ, "paramRenames", array);
    array = NULL;
    for (uint64_t i=0; i<inputRenames.size(); i++){
      WidgetRename* wr = &inputRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      PortInfo* pi = mod->getInputInfo(wr->id);
      if (!pi || pi->name != wr->name || pi->name == wr->factoryName) continue;
      if (!array) array = json_array();
      json_array_append_new(array, json_rename(wr));
    }
    if(array) json_object_set_new(rootJ, "inputRenames", array);
    array = NULL;
    for (uint64_t i=0; i<outputRenames.size(); i++){
      WidgetRename* wr = &outputRenames[i];
      Module* mod = APP->engine->getModule(wr->modId);
      if (!mod) continue;
      PortInfo* pi = mod->getOutputInfo(wr->id);
      if (!pi || pi->name != wr->name || pi->name == wr->factoryName) continue;
      if (!array) array = json_array();
      json_array_append_new(array, json_rename(wr));
    }
    if(array) json_object_set_new(rootJ, "outputRenames", array);
    return rootJ;
  }
  
  void loadRename(json_t* rootJ, std::string key, RenameVector* renameVector){
    json_t* array;
    json_t* obj;
    size_t index;
    if ((array = json_object_get(rootJ, key.c_str()))){
      json_array_foreach(array, index, obj){
        json_t* modId = json_object_get(obj, "modId");
        json_t* id = json_object_get(obj, "id");
        json_t* factoryName = json_object_get(obj, "factoryName");
        json_t* name = json_object_get(obj, "name");
        if (modId && id && factoryName && name){
          WidgetRename* rename = new WidgetRename(json_integer_value(modId), json_integer_value(id), json_string_value(factoryName), json_string_value(name));
          renameVector->push_back(*rename);
          delete rename;
        }
      }
    }
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    if (paramsInitialized) return;
    if (!disable) {
      json_t* val = json_object_get(rootJ, "disable");
      if (val && json_boolean_value(val)) {
        std::vector< std::string > labels {"Permanently disabled"};
        SwitchQuantity* pq = static_cast<SwitchQuantity*>(paramQuantities[ENABLE_PARAM]);
        pq->defaultValue = 0.f;
        pq->maxValue = 0.f;
        pq->labels = labels;
        disable = true;
      }
    }
    if (disable) return;
    json_t* array;
    json_t* obj;
    size_t index;
    if ((array = json_object_get(rootJ, "defaults"))){
      json_array_foreach(array, index, obj){
        json_t* modId = json_object_get(obj, "modId");
        json_t* id = json_object_get(obj, "id");
        json_t* factoryDflt = json_object_get(obj, "factoryDflt");
        json_t* dflt = json_object_get(obj, "dflt");
        if (modId && id && factoryDflt && dflt){
          ParamDefault* paramDefault = new ParamDefault(json_integer_value(modId), json_integer_value(id), json_real_value(factoryDflt), json_real_value(dflt));
          defaults.push_back(*paramDefault);
          delete paramDefault;
        }
      }
    }
    loadRename(rootJ, "paramRenames", &paramRenames);
    loadRename(rootJ, "inputRenames", &inputRenames);
    loadRename(rootJ, "outputRenames", &outputRenames);
  }

};

struct WidgetMenuExtenderWidget : VenomWidget {
  Widget* lastSelectedWidget = NULL;

  WidgetMenuExtenderWidget(WidgetMenuExtender* module) {
    setModule(module);
    setVenomPanel("WidgetMenuExtender");
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<RedBlueLight<>>>>(Vec(22.5f, 100.f), module, WidgetMenuExtender::ENABLE_PARAM, WidgetMenuExtender::ENABLE_RED_LIGHT));
  }

  void step() override {
    VenomWidget::step();
    WidgetMenuExtender* mod = dynamic_cast<WidgetMenuExtender*>(this->module);
    if (!mod || mod->isBypassed()) return;
    bool enabled = mod->params[WidgetMenuExtender::ENABLE_PARAM].getValue();
    mod->lights[WidgetMenuExtender::ENABLE_RED_LIGHT].setBrightness(mod->disable ? LIGHT_ON : 0.f);
    mod->lights[WidgetMenuExtender::ENABLE_BLUE_LIGHT].setBrightness(enabled ? LIGHT_ON : mod->disable ? 0.f : LIGHT_OFF);
    if (!enabled) return;

    // Remainder of code inspired by Stoermelder Packone ui/ParamWidgetContextExtender.hpp
    Widget* widget = APP->event->getDraggedWidget();
    if (!widget) return;

    if (APP->event->dragButton != GLFW_MOUSE_BUTTON_RIGHT) {
      lastSelectedWidget = NULL;
      return;
    }

    if (widget != lastSelectedWidget) {
      lastSelectedWidget = widget;

      ParamWidget* paramWidget = dynamic_cast<ParamWidget*>(widget);
      PortWidget* portWidget = dynamic_cast<PortWidget*>(widget);
      if (!paramWidget && !portWidget) return;
        
      MenuOverlay* overlay = NULL;
      for (auto rit = APP->scene->children.rbegin(); rit != APP->scene->children.rend(); rit++) {
        overlay = dynamic_cast<MenuOverlay*>(*rit);
        if (overlay) break;
      }
      if (!overlay) return;
      Menu* menu = overlay->getFirstDescendantOfType<Menu>();
      if (!menu) return;

      Module* foreignMod = (paramWidget) ? paramWidget->module : portWidget->module;
      if (!foreignMod) return;
      plugin::Model* model = foreignMod->getModel();
      if (!model) return;
      if (model->plugin->slug == "Venom" && model->slug != "RhythmExplorer") return;

      if (paramWidget) mod->extendForeignParameterMenu(model, paramWidget, menu);
      else /*portWidget*/ mod->extendForeignPortMenu(portWidget, menu);
    }
  }

};

}

Model* modelVenomWidgetMenuExtender = createModel<Venom::WidgetMenuExtender, Venom::WidgetMenuExtenderWidget>("WidgetMenuExtender");
