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
  
  int64_t modId = -1;

  struct BayLink {
    int64_t id;
    BayModule* mod;
    BayLink( int64_t idParm = -1, BayModule* modParm = NULL ){
      id = idParm;
      mod = modParm;
    }
  };

  std::vector<BayLink> links{};

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (modId != id) { // first execution only - remove invalid links
      for (int i=static_cast<int>(links.size())-1; i>=0; i--){
        if (links[i].mod == NULL )
          links.erase(links.begin()+i);
      }
      modId = id;
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "modId", json_integer(modId));
    if (links.size()) {
      json_t* array = json_array();
      for (unsigned i=0; i<links.size(); i++)
        json_array_append_new(array, json_integer(links[i].id));
      json_object_set_new(rootJ, "links", array);
    }
    return rootJ;
  
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    json_t* array;
    size_t index;
    if ((val = json_object_get(rootJ, "modId")))
      modId = json_integer_value(val);
    if ((array = json_object_get(rootJ, "links"))) {
      json_array_foreach(array, index, val)
        links.push_back(BayLink(json_integer_value(val)));
    }
  }
  
};

struct BayInputModule : BayModule {

  void onAdd(const AddEvent& e) override {
    unsigned todo = links.size();
    if (modId != id && todo) { // remap links
      for (int64_t modId : APP->engine->getModuleIds()){
        BayModule* targetMod = dynamic_cast<BayModule*>(APP->engine->getModule(modId));
        if (targetMod && targetMod->links.size()==1 && !(targetMod->links[0].mod) && targetMod->links[0].id == modId) {
          for (BayLink& target : links) {
            if (target.id == targetMod->modId) {
              target.id = targetMod->id;
              target.mod = targetMod;
              targetMod->links[0].id = id;
              targetMod->links[0].mod = this;
              break;
            }
          }
          if (!--todo)
            break;
        }
      }
    }
  }

  void onRemove(const RemoveEvent& e) override { // remove link to this module from all target's
    for (BayLink& target : links) {
      BayModule* targetMod = static_cast<BayModule*>(target.mod);
      if (targetMod->links.size()==1)
        targetMod->links.pop_back();
    }
  }
  
};

struct BayOutputModule : BayModule {

  void onAdd(const AddEvent& e) override {
    if (modId != id && links.size()==1 && links[0].id >= 0) { // remap link
      for (int64_t modId : APP->engine->getModuleIds()){
        BayModule* sourceMod = dynamic_cast<BayModule*>(APP->engine->getModule(modId));
        if (sourceMod && sourceMod->modId == links[0].id) {
          for (BayLink& target : sourceMod->links) {
            if (target.id == modId) {
              target.id = id;
              target.mod = this;
              links[0].id = sourceMod->id;
              links[0].mod = sourceMod;
              break;
            }
          }
          break;
        }
      }
    }
  }

  void onRemove(const RemoveEvent& e) override {  // remove this target link from the source
    BayModule* sourceMod = links.size() ? static_cast<BayModule*>(links[0].mod) : NULL;
    if (sourceMod) {
      for (unsigned i=sourceMod->links.size(); i; i--) {
        if (sourceMod->links[i].id == id) {
          sourceMod->links.erase(sourceMod->links.begin()+i);
          break;
        }
      }
    }
  }

};