// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "ThemeStrings.hpp"

#define MODULE_NAME WinComp
static const std::string moduleName = "WinComp";

struct WinComp : Module {
  enum ParamId {
    A_PARAM,
    B_PARAM,
    TOL_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    TOL_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    MIN_OUTPUT,
    MAX_OUTPUT,
    CLAMP_OUTPUT,
    OVER_OUTPUT,
    EQ_OUTPUT,
    NEQ_OUTPUT,
    LSEQ_OUTPUT,
    GREQ_OUTPUT,
    LS_OUTPUT,
    GR_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    MIN_ABS_LIGHT,
    MAX_ABS_LIGHT,
    CLAMP_ABS_LIGHT,
    OVER_ABS_LIGHT,
    MIN_INV_LIGHT,
    MAX_INV_LIGHT,
    CLAMP_INV_LIGHT,
    OVER_INV_LIGHT,
    ENUMS(EQ_LIGHT, 2),
    ENUMS(NEQ_LIGHT, 2),
    ENUMS(LSEQ_LIGHT, 2),
    ENUMS(GREQ_LIGHT, 2),
    ENUMS(GR_LIGHT, 2),
    ENUMS(LS_LIGHT, 2),
    LIGHTS_LEN
  };
  
  float gateTypes[6][2] = {{0.f,1.f},{-1.f,1.f},{0.f,5.f},{-5.f,5.f},{0.f,10.f},{-10.f,10.f}};
  int gateType = 4;
  
  enum ModPorts {
    MIN_PORT,
    MAX_PORT,
    CLAMP_PORT,
    OVER_PORT
  };
  
  bool absPort[5] = {false,false,false,false,false};
  bool invPort[5] = {false,false,false,false,false};

  bool absMinOld = false;
  bool absMaxOld = false;
  bool absClampOld = false;
  bool absOverOld = false;

  bool invMinOld = false;
  bool invMaxOld = false;
  bool invClampOld = false;
  bool invOverOld = false;

  #include "ThemeModVars.hpp"

  dsp::ClockDivider lightDivider;

  WinComp() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(A_PARAM, -10.f, 10.f, 0.f, "A offset", " V");
    configParam(B_PARAM, -10.f, 10.f, 0.f, "B offset", " V");
    configParam(TOL_PARAM, -10.f, 10.f, 0.f, "Tolerance", " V");
    configInput(A_INPUT, "A");
    configInput(B_INPUT, "B");
    configInput(TOL_INPUT, "Tolerance");
    configOutput(MIN_OUTPUT, "Minimum");
    configOutput(MAX_OUTPUT, "Maximum");
    configOutput(CLAMP_OUTPUT, "A Clamped");
    configOutput(OVER_OUTPUT, "A Overflow");
    configOutput(EQ_OUTPUT, "A=B");
    configOutput(NEQ_OUTPUT, "A<>B");
    configOutput(LSEQ_OUTPUT, "A<=B");
    configOutput(GREQ_OUTPUT, "A>=B");
    configOutput(LS_OUTPUT, "A<B");
    configOutput(GR_OUTPUT, "A>B");

    lightDivider.setDivision(32);
  }

  void process(const ProcessArgs& args) override {
    int channels = std::max({1, inputs[A_INPUT].getChannels(), inputs[B_INPUT].getChannels(), inputs[TOL_INPUT].getChannels()});
    float aOffset = params[A_PARAM].getValue();
    float bOffset = params[B_PARAM].getValue();
    float tolOffset = params[TOL_PARAM].getValue();

    bool anyEq = false;
    bool anyNeq = false;
    bool anyLsEq = false;
    bool anyGrEq = false;
    bool anyGr = false;
    bool anyLs = false;

    for (int c = 0; c < channels; c++) {
      float a = inputs[A_INPUT].getPolyVoltage(c) + aOffset;
      float b = inputs[B_INPUT].getPolyVoltage(c) + bOffset;
      float tol = std::fabs(inputs[TOL_INPUT].getPolyVoltage(c) + tolOffset);
      float bMin = b - tol;
      float bMax = b + tol;
      float val;

      val = std::min(a, b);
      if (absPort[MIN_PORT]) val = std::fabs(val);
      if (invPort[MIN_PORT]) val = -val;
      outputs[MIN_OUTPUT].setVoltage(val, c);
      val = std::max(a, b);
      if (absPort[MAX_PORT]) val = std::fabs(val);
      if (invPort[MAX_PORT]) val = -val;
      outputs[MAX_OUTPUT].setVoltage(val, c);

      float clamp = a;
      bool over = false;
      if (a > bMax) {
        clamp = bMax;
        over = true;
      }
      else if (a < bMin) {
        clamp = bMin;
        over = true;
      }
      val = absPort[CLAMP_PORT] ? std::fabs(clamp) : clamp;
      if (invPort[CLAMP_PORT]) val = -val;
      outputs[CLAMP_OUTPUT].setVoltage(val, c);
      val = a - clamp;
      if (absPort[OVER_PORT]) val = std::fabs(val);
      if (invPort[OVER_PORT]) val = -val;
      outputs[OVER_OUTPUT].setVoltage(val, c);

      float low = gateTypes[gateType][0];
      float high = gateTypes[gateType][1];

      outputs[EQ_OUTPUT].setVoltage(over ? low : high, c);
      anyEq = anyEq || !over;
      outputs[NEQ_OUTPUT].setVoltage(over ? high : low, c);
      anyNeq = anyNeq || over;

      bool cond = a <= b + tol;
      outputs[LSEQ_OUTPUT].setVoltage(cond ? high : low, c);
      anyLsEq = anyLsEq || cond;
      cond = a >= b - tol;
      outputs[GREQ_OUTPUT].setVoltage(cond ? high : low, c);
      anyGrEq = anyGrEq || cond;

      cond = a < b - tol;
      outputs[LS_OUTPUT].setVoltage(cond ? high : low, c);
      anyLs = anyLs || cond;
      cond = a > b + tol;
      outputs[GR_OUTPUT].setVoltage(cond ? high : low, c);
      anyGr = anyGr || cond;
    }

    outputs[MAX_OUTPUT].setChannels(channels);
    outputs[MIN_OUTPUT].setChannels(channels);
    outputs[CLAMP_OUTPUT].setChannels(channels);
    outputs[OVER_OUTPUT].setChannels(channels);
    outputs[EQ_OUTPUT].setChannels(channels);
    outputs[NEQ_OUTPUT].setChannels(channels);
    outputs[LSEQ_OUTPUT].setChannels(channels);
    outputs[GREQ_OUTPUT].setChannels(channels);
    outputs[GR_OUTPUT].setChannels(channels);
    outputs[LS_OUTPUT].setChannels(channels);

    if (lightDivider.process()) {
      float lightTime = args.sampleTime * lightDivider.getDivision();
      if (absPort[MIN_PORT] != absMinOld) {
        absMinOld = absPort[MIN_PORT];
        lights[MIN_ABS_LIGHT].setBrightness(absPort[MIN_PORT]);
      }
      if (invPort[MIN_PORT] != invMinOld) {
        invMinOld = invPort[MIN_PORT];
        lights[MIN_INV_LIGHT].setBrightness(invPort[MIN_PORT]);
      }

      if (absPort[MAX_PORT] != absMaxOld) {
        absMaxOld = absPort[MAX_PORT];
        lights[MAX_ABS_LIGHT].setBrightness(absPort[MAX_PORT]);
      }
      if (invPort[MAX_PORT] != invMaxOld) {
        invMaxOld = invPort[MAX_PORT];
        lights[MAX_INV_LIGHT].setBrightness(invPort[MAX_PORT]);
      }

      if (absPort[CLAMP_PORT] != absClampOld) {
        absClampOld = absPort[CLAMP_PORT];
        lights[CLAMP_ABS_LIGHT].setBrightness(absPort[CLAMP_PORT]);
      }
      if (invPort[CLAMP_OUTPUT] != invClampOld) {
        invClampOld = invPort[CLAMP_PORT];
        lights[CLAMP_INV_LIGHT].setBrightness(invPort[CLAMP_PORT]);
      }

      if (absPort[OVER_PORT] != absOverOld) {
        absOverOld = absPort[OVER_PORT];
        lights[OVER_ABS_LIGHT].setBrightness(absPort[OVER_PORT]);
      }
      if (invPort[OVER_PORT] != invOverOld) {
        invOverOld = invPort[OVER_PORT];
        lights[OVER_INV_LIGHT].setBrightness(invPort[OVER_PORT]);
      }

      lights[EQ_LIGHT + 0].setBrightnessSmooth(anyEq && channels <= 1, lightTime);
      lights[EQ_LIGHT + 1].setBrightnessSmooth(anyEq && channels > 1, lightTime);

      lights[NEQ_LIGHT + 0].setBrightnessSmooth(anyNeq && channels <= 1, lightTime);
      lights[NEQ_LIGHT + 1].setBrightnessSmooth(anyNeq && channels > 1, lightTime);

      lights[LSEQ_LIGHT + 0].setBrightnessSmooth(anyLsEq && channels <= 1, lightTime);
      lights[LSEQ_LIGHT + 1].setBrightnessSmooth(anyLsEq && channels > 1, lightTime);

      lights[GREQ_LIGHT + 0].setBrightnessSmooth(anyGrEq && channels <= 1, lightTime);
      lights[GREQ_LIGHT + 1].setBrightnessSmooth(anyGrEq && channels > 1, lightTime);

      lights[LS_LIGHT + 0].setBrightnessSmooth(anyLs && channels <= 1, lightTime);
      lights[LS_LIGHT + 1].setBrightnessSmooth(anyLs && channels > 1, lightTime);

      lights[GR_LIGHT + 0].setBrightnessSmooth(anyGr && channels <= 1, lightTime);
      lights[GR_LIGHT + 1].setBrightnessSmooth(anyGr && channels > 1, lightTime);
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "absMin", json_boolean(absPort[MIN_PORT]));
    json_object_set_new(rootJ, "absMax", json_boolean(absPort[MAX_PORT]));
    json_object_set_new(rootJ, "absClamp", json_boolean(absPort[CLAMP_PORT]));
    json_object_set_new(rootJ, "absOver", json_boolean(absPort[OVER_PORT]));
    json_object_set_new(rootJ, "invMin", json_boolean(invPort[MIN_PORT]));
    json_object_set_new(rootJ, "invMax", json_boolean(invPort[MAX_PORT]));
    json_object_set_new(rootJ, "invClamp", json_boolean(invPort[CLAMP_PORT]));
    json_object_set_new(rootJ, "invOver", json_boolean(invPort[OVER_PORT]));
    json_object_set_new(rootJ, "gateType", json_integer(gateType));
    #include "ThemeToJson.hpp"
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val = json_object_get(rootJ, "absMin");
    if (val)
      absPort[MIN_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "absMax");
    if (val)
      absPort[MAX_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "absClamp");
    if (val)
      absPort[CLAMP_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "absOver");
    if (val)
      absPort[OVER_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "invMin");
    if (val)
      invPort[MIN_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "invMax");
    if (val)
      invPort[MAX_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "invClamp");
    if (val)
      invPort[CLAMP_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "invOver");
    if (val)
      invPort[OVER_PORT] = json_boolean_value(val);
    val = json_object_get(rootJ, "gateType");
    if (val)
      gateType = json_integer_value(val);
    #include "ThemeFromJson.hpp"
  }

};


struct WinCompWidget : ModuleWidget {

  struct AbsInvPort : PJ301MPort {
    int modId;
    void appendContextMenu(Menu* menu) override {
      WinComp* module = dynamic_cast<WinComp*>(this->module);
      assert(module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Absolute value", "", &module->absPort[modId]));
      menu->addChild(createBoolPtrMenuItem("Invert", "", &module->invPort[modId]));
    }
  };
  
  template <class TAbsInvWidget>
  TAbsInvWidget* createAbsInvOutputCentered(math::Vec pos, engine::Module* module, int outputId, int modIdArg) {
    TAbsInvWidget* o = createOutputCentered<TAbsInvWidget>(pos, module, outputId);
    o->modId = modIdArg;
    return o;
  }

  WinCompWidget(WinComp* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.299, 15.93)), module, WinComp::A_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.134, 15.93)), module, WinComp::A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.299, 30.05)), module, WinComp::B_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.134, 30.05)), module, WinComp::B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.299, 43.87)), module, WinComp::TOL_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.134, 43.87)), module, WinComp::TOL_PARAM));

    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(7.299, 58.3)), module, WinComp::MIN_OUTPUT, WinComp::MIN_PORT));
    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(18.136, 58.3)), module, WinComp::MAX_OUTPUT, WinComp::MAX_PORT));
    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(7.297, 72.75)), module, WinComp::CLAMP_OUTPUT, WinComp::CLAMP_PORT));
    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(18.134, 72.75)), module, WinComp::OVER_OUTPUT, WinComp::OVER_PORT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.297, 87.10)), module, WinComp::EQ_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.134, 87.10)), module, WinComp::NEQ_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.297, 101.55)), module, WinComp::LSEQ_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.134, 101.55)), module, WinComp::GREQ_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.297, 116.0)), module, WinComp::LS_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.134, 116.0)), module, WinComp::GR_OUTPUT));


    addChild(createLightCentered<SmallLight<GrnLight<>>>(mm2px(Vec( 3.547, 54.58+7.46)), module, WinComp::MIN_ABS_LIGHT));
    addChild(createLightCentered<SmallLight<RdLight<>>>(mm2px(Vec(11.047, 54.58+7.46)), module, WinComp::MIN_INV_LIGHT));
    addChild(createLightCentered<SmallLight<GrnLight<>>>(mm2px(Vec(14.384, 54.58+7.46)), module, WinComp::MAX_ABS_LIGHT));
    addChild(createLightCentered<SmallLight<RdLight<>>>(mm2px(Vec(21.884, 54.58+7.46)), module, WinComp::MAX_INV_LIGHT));
    addChild(createLightCentered<SmallLight<GrnLight<>>>(mm2px(Vec( 3.547, 69.03+7.46)), module, WinComp::CLAMP_ABS_LIGHT));
    addChild(createLightCentered<SmallLight<RdLight<>>>(mm2px(Vec(11.047, 69.03+7.46)), module, WinComp::CLAMP_INV_LIGHT));
    addChild(createLightCentered<SmallLight<GrnLight<>>>(mm2px(Vec(14.384, 69.03+7.46)), module, WinComp::OVER_ABS_LIGHT));
    addChild(createLightCentered<SmallLight<RdLight<>>>(mm2px(Vec(21.884, 69.03+7.46)), module, WinComp::OVER_INV_LIGHT));
    addChild(createLightCentered<SmallLight<YellowBlueLight<>>>(mm2px(Vec(11.047, 83.38+7.46)), module, WinComp::EQ_LIGHT));
    addChild(createLightCentered<SmallLight<YellowBlueLight<>>>(mm2px(Vec(21.884, 83.38+7.46)), module, WinComp::NEQ_LIGHT));
    addChild(createLightCentered<SmallLight<YellowBlueLight<>>>(mm2px(Vec(11.047, 97.83+7.46)), module, WinComp::LSEQ_LIGHT));
    addChild(createLightCentered<SmallLight<YellowBlueLight<>>>(mm2px(Vec(21.884, 97.83+7.46)), module, WinComp::GREQ_LIGHT));
    addChild(createLightCentered<SmallLight<YellowBlueLight<>>>(mm2px(Vec(11.047, 112.28+7.46)), module, WinComp::LS_LIGHT));
    addChild(createLightCentered<SmallLight<YellowBlueLight<>>>(mm2px(Vec(21.884, 112.28+7.46)), module, WinComp::GR_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    WinComp* module = dynamic_cast<WinComp*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);
    std::vector<std::string> gateLabels;
    gateLabels.push_back("0,1");
    gateLabels.push_back("+/-1");
    gateLabels.push_back("0,5");
    gateLabels.push_back("+/-5");
    gateLabels.push_back("0,10");
    gateLabels.push_back("+/-10");
    menu->addChild(createIndexSubmenuItem("Gate voltages", gateLabels,
      [=]() {return module->gateType;},
      [=](int i) {module->gateType = i;}
    ));
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Minimum absolute value", "", &module->absPort[WinComp::MIN_PORT]));
    menu->addChild(createBoolPtrMenuItem("Minimum invert", "", &module->invPort[WinComp::MIN_PORT]));
    menu->addChild(createBoolPtrMenuItem("Maximum absolute value", "", &module->absPort[WinComp::MAX_PORT]));
    menu->addChild(createBoolPtrMenuItem("Maximum invert", "", &module->invPort[WinComp::MAX_PORT]));
    menu->addChild(createBoolPtrMenuItem("Clamp absolute value", "", &module->absPort[WinComp::CLAMP_PORT]));
    menu->addChild(createBoolPtrMenuItem("Clamp invert", "", &module->invPort[WinComp::CLAMP_PORT]));
    menu->addChild(createBoolPtrMenuItem("Overflow absolute value", "", &module->absPort[WinComp::OVER_PORT]));
    menu->addChild(createBoolPtrMenuItem("Overflow invert", "", &module->invPort[WinComp::OVER_PORT]));
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};


Model* modelWinComp = createModel<WinComp, WinCompWidget>("WinComp");
