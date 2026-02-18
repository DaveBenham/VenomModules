// Venom Modules (c) 2023, 2024, 2025, 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct MergeSplit : VenomModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(MERGE_INPUT,4),
    SPLIT_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    MERGE_OUTPUT,
    ENUMS(SPLIT_OUTPUT,4),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(MERGE_LIGHT,4),
    SPLIT_LIGHT,
    RESPLIT_LIGHT,
    LIGHTS_LEN
  };
   
  int outChannels[5]{};
  std::string outLabels[16]{"Auto/1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};
  
  void setOutputDescription(int id){
    outputInfos[id]->description = "Channels: "+outLabels[outChannels[id]];
  }
  
  MergeSplit() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configOutput(MERGE_OUTPUT, "Merge");
    configInput(SPLIT_INPUT, "Split");
    configLight(SPLIT_LIGHT, "Split input dropped channel(s) indicator");
    configLight(RESPLIT_LIGHT, "Successful auto resplit indicator");
    for (int i=0; i<4; i++) {
      std::string name = "Merge " + std::to_string(i+1);
      configInput(MERGE_INPUT+i, name);
      configLight(MERGE_LIGHT+i, name+" input dropped channel(s) indicator");
      name = "Split " + std::to_string(i+1);
      configOutput(SPLIT_OUTPUT+i, name);
      setOutputDescription(SPLIT_OUTPUT+i);
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<4; i++)
      lights[MERGE_LIGHT+i].setBrightness(0);
    int maxPort=3;
    while (maxPort>MERGE_INPUT && !inputs[MERGE_INPUT+maxPort].isConnected())
      maxPort--;
    int mergeChannels = 0,
        maxChannels = 16;
    bool resplit = (outChannels[1]+outChannels[2]+outChannels[3]+outChannels[4]) == 0;
    for (int i=0; i<=maxPort; i++) {
      int cnt = std::max(inputs[MERGE_INPUT+i].getChannels(),1);
      if (cnt>maxChannels){
        cnt=maxChannels;
        lights[MERGE_LIGHT+i].setBrightness(1.f);
        resplit = false;
      }
      for (int c=0; c<cnt; c++)
        outputs[MERGE_OUTPUT].setVoltage(inputs[MERGE_INPUT+i].getVoltage(c), mergeChannels++);
      maxChannels -= cnt;
    }
    outputs[MERGE_OUTPUT].setChannels(mergeChannels);
    int activeChannel = 0,
        inChannels = std::max(inputs[SPLIT_INPUT].getChannels(), 1);
    if (mergeChannels != inChannels)
      resplit = false;
    for (int i=0; i<4; i++) {
      int cnt = resplit ? std::max(inputs[MERGE_INPUT+i].getChannels(),1) : outChannels[i]+1;
      for (int y=0; y<cnt; y++){
        outputs[SPLIT_OUTPUT+i].setVoltage(activeChannel<inChannels ? inputs[SPLIT_INPUT].getVoltage(activeChannel++) : 0.f, y);
      }
      outputs[SPLIT_OUTPUT+i].setChannels(cnt);
    }
    lights[SPLIT_LIGHT].setBrightness(activeChannel<inChannels);
    lights[RESPLIT_LIGHT].setBrightness(resplit);
  }                         

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<4; i++){
      json_array_append_new(array, json_integer(outChannels[i]));
    }
    json_object_set_new(rootJ, "outChannels", array);
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* array = json_object_get(rootJ, "outChannels");
    if (array){
      size_t i;
      json_t* val;
      json_array_foreach(array, i, val) {
        outChannels[i] = json_integer_value(val);
        setOutputDescription(i);
      }
    }
  }
  
};

struct MergeSplitWidget : VenomWidget {

  struct OutPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      MergeSplit* module = static_cast<MergeSplit*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createIndexSubmenuItem(
        "Channels",
        {"Auto/1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
        [=]() {
          return module->outChannels[portId];
        },
        [=](int cnt) {
          module->outChannels[portId] = cnt;
          module->setOutputDescription(portId);
        }
      ));    
      PolyPort::appendContextMenu(menu);
    }
  };

  template <class TWidget>
  TWidget* createConfigChannelsOutputCentered(math::Vec pos, engine::Module* module, int outputId) {
    TWidget* o = createOutputCentered<TWidget>(pos, module, outputId);
    o->portId = outputId;
    return o;
  }

  MergeSplitWidget(MergeSplit* module) {
    setModule(module);
    setVenomPanel("MergeSplit");

    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 171.5f), module, MergeSplit::MERGE_OUTPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 219.5f), module, MergeSplit::SPLIT_INPUT));
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(35.f, 209.f), module, MergeSplit::SPLIT_LIGHT));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 192.111f), module, MergeSplit::RESPLIT_LIGHT));
    for (int i=0; i<4; i++){
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 51.5f+28.f*i), module, MergeSplit::MERGE_INPUT+i));
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(35.f, 41.f+28.f*i), module, MergeSplit::MERGE_LIGHT+i));
      addOutput(createConfigChannelsOutputCentered<OutPort>(Vec(22.5f, 256.5f+28.f*i), module, MergeSplit::SPLIT_OUTPUT+i));
    }
  }

  void appendContextMenu(Menu* menu) override {
    MergeSplit* module = static_cast<MergeSplit*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Restore automatic split outputs", "",
      [=]() {
        for (int i=1; i<5; i++) {
          module->outChannels[i]=0;
          module->setOutputDescription(i);
        }
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

}

Model* modelVenomMergeSplit = createModel<Venom::MergeSplit, Venom::MergeSplitWidget>("MergeSplit");
