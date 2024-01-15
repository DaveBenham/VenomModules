// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "Filter.hpp"
#include "plugin.hpp"

struct WinComp : VenomModule {
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
    OVERSAMPLE_LIGHT,
    ENUMS(EQ_LIGHT, 2),
    ENUMS(NEQ_LIGHT, 2),
    ENUMS(LSEQ_LIGHT, 2),
    ENUMS(GREQ_LIGHT, 2),
    ENUMS(GR_LIGHT, 2),
    ENUMS(LS_LIGHT, 2),
    LIGHTS_LEN
  };
  
  float gateTypes[6][3] = {{0.f,1.f,0.5f},{-1.f,1.f,0.f},{0.f,5.f,2.5f},{-5.f,5.f,0.f},{0.f,10.f,5.f},{-10.f,10.f,0.f}};
  int gateType = 4;
  
  enum ModPorts {
    MIN_PORT,
    MAX_PORT,
    CLAMP_PORT,
    OVER_PORT
  };
  
  bool absPort[4] = {false,false,false,false};
  bool invPort[4] = {false,false,false,false};
  
  int oversample = 1;
  std::vector<std::string> oversampleLabels = {"Off","x2","x4","x8","x16","x32"};
  std::vector<int> oversampleValues = {1,2,4,8,16,32};

  bool absMinOld = false;
  bool absMaxOld = false;
  bool absClampOld = false;
  bool absOverOld = false;

  bool invMinOld = false;
  bool invMaxOld = false;
  bool invClampOld = false;
  bool invOverOld = false;

  OversampleFilter_4 aUpSample[4], bUpSample[4], tolUpSample[4],
                     minDownSample[4], maxDownSample[4],
                     clampDownSample[4], overDownSample[4],
                     eqDownSample[4], neqDownSample[4],
                     leqDownSample[4], geqDownSample[4],
                     lsDownSample[4], grDownSample[4];

  dsp::ClockDivider lightDivider;

  void initializeOversample(){
    for (int c=0; c<4; c++){
      aUpSample[c].setOversample(oversample);
      bUpSample[c].setOversample(oversample);
      tolUpSample[c].setOversample(oversample);
      minDownSample[c].setOversample(oversample);
      maxDownSample[c].setOversample(oversample);
      clampDownSample[c].setOversample(oversample);
      overDownSample[c].setOversample(oversample);
      eqDownSample[c].setOversample(oversample);
      neqDownSample[c].setOversample(oversample);
      leqDownSample[c].setOversample(oversample);
      geqDownSample[c].setOversample(oversample);
      lsDownSample[c].setOversample(oversample);
      grDownSample[c].setOversample(oversample);
    }
  }

  WinComp() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
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
    
    configLight(OVERSAMPLE_LIGHT, "Oversample indicator");
    configLight(MIN_ABS_LIGHT, "Minimum absolute value indicator");
    configLight(MAX_ABS_LIGHT, "Maximum absolute value indicator");
    configLight(CLAMP_ABS_LIGHT, "A Clamped absolute value indicator");
    configLight(OVER_ABS_LIGHT, "A Overflow absolute value indicator");
    configLight(MIN_INV_LIGHT, "Minimum inverted indicator");
    configLight(MAX_INV_LIGHT, "Maximum inverted indicator");
    configLight(CLAMP_INV_LIGHT, "A Clamped inverted indicator");
    configLight(OVER_INV_LIGHT, "A Overflow inverted indicator");
    configLight(EQ_LIGHT, "A=B (yellow mono, blue poly)");
    configLight(NEQ_LIGHT, "A<>B indicator (yellow mono, blue poly)");
    configLight(LSEQ_LIGHT, "A<=B indicator (yellow mono, blue poly)");
    configLight(GREQ_LIGHT, "A>=B indicator (yellow mono, blue poly)");
    configLight(GR_LIGHT, "A>B indicator (yellow mono, blue poly)");
    configLight(LS_LIGHT, "A<B indicator (yellow mono, blue poly)");

    initializeOversample();
    lightDivider.setDivision(32);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
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

    bool aOS = oversample>1 && inputs[A_INPUT].isConnected();
    bool bOS = oversample>1 && inputs[B_INPUT].isConnected();
    bool tolOS = oversample>1 && inputs[TOL_INPUT].isConnected();

    bool compMin = outputs[MIN_OUTPUT].isConnected();
    bool compMax = outputs[MAX_OUTPUT].isConnected();
    bool compClamp = outputs[CLAMP_OUTPUT].isConnected();
    bool compOver = outputs[OVER_OUTPUT].isConnected();
    bool compEq = outputs[EQ_OUTPUT].isConnected();
    bool compNeq = outputs[NEQ_OUTPUT].isConnected();
    bool compLeq = outputs[LSEQ_OUTPUT].isConnected();
    bool compGeq = outputs[GREQ_OUTPUT].isConnected();
    bool compLs = outputs[LS_OUTPUT].isConnected();
    bool compGr = outputs[GR_OUTPUT].isConnected();

    bool processLights = lightDivider.process();
    using float_4 = simd::float_4;
    float_4 low = gateTypes[gateType][0];
    float_4 high = gateTypes[gateType][1];
    float mid = gateTypes[gateType][2];
    for (int c = 0; c < channels; c += 4) {
      float_4 a = inputs[A_INPUT].getPolyVoltageSimd<float_4>(c) + aOffset;
      float_4 b = inputs[B_INPUT].getPolyVoltageSimd<float_4>(c) + bOffset;
      float_4 tol = simd::fabs(inputs[TOL_INPUT].getPolyVoltageSimd<float_4>(c)) + tolOffset;
      float_4 minVal=float_4::zero(), maxVal=float_4::zero(), clampVal=float_4::zero(), overVal=float_4::zero(),
            eqVal=float_4::zero(), neqVal=float_4::zero(), leqVal=float_4::zero(), geqVal=float_4::zero(),
            lsVal=float_4::zero(), grVal=float_4::zero();
      for (int i=0; i<oversample; i++){
        if (aOS) a = aUpSample[c/4].process(i ? float_4::zero() : a*oversample);
        if (bOS) b = bUpSample[c/4].process(i ? float_4::zero() : b*oversample);
        if (tolOS) tol = tolUpSample[c/4].process(i ? float_4::zero() : tol*oversample);
        tol = simd::fmax(tol, 1e-6);

        float_4 bMin = b - tol;
        float_4 bMax = b + tol;

        minVal = simd::fmin(a, b);
        if (absPort[MIN_PORT]) minVal = simd::fabs(minVal);
        if (invPort[MIN_PORT]) minVal = -minVal;

        maxVal = simd::fmax(a, b);
        if (absPort[MAX_PORT]) maxVal = simd::fabs(maxVal);
        if (invPort[MAX_PORT]) maxVal = -maxVal;

        float_4 clamp = a;
        float_4 aOverB = a > bMax;
        float_4 aUnderB = a < bMin;
        clamp = simd::ifelse(aOverB, bMax, clamp);
        clamp = simd::ifelse(aUnderB, bMin, clamp);
        clampVal = absPort[CLAMP_PORT] ? simd::fabs(clamp) : clamp;
        if (invPort[CLAMP_PORT]) clampVal = -clampVal;

        overVal = a - clamp;
        if (absPort[OVER_PORT]) overVal = simd::fabs(overVal);
        if (invPort[OVER_PORT]) overVal = -overVal;

        eqVal = simd::ifelse(aOverB, low, high);
        eqVal = simd::ifelse(aUnderB, low, eqVal);
        neqVal = simd::ifelse(aOverB, high, low);
        neqVal = simd::ifelse(aUnderB, high, neqVal);

        leqVal = simd::ifelse(a <= b + tol, high, low);
        geqVal = simd::ifelse(a >= b - tol, high, low);

        lsVal = simd::ifelse(a < b - tol, high, low);
        grVal = simd::ifelse(a > b + tol, high, low);

        if (oversample>1) {
          if (compMin) minVal = minDownSample[c/4].process(minVal);
          if (compMax) maxVal = maxDownSample[c/4].process(maxVal);
          if (compClamp) clampVal = clampDownSample[c/4].process(clampVal);
          if (compOver) overVal = overDownSample[c/4].process(overVal);
          if (compEq) eqVal = eqDownSample[c/4].process(eqVal);
          if (compNeq) neqVal = neqDownSample[c/4].process(neqVal);
          if (compLeq) leqVal = leqDownSample[c/4].process(leqVal);
          if (compGeq) geqVal = geqDownSample[c/4].process(geqVal);
          if (compLs) lsVal = lsDownSample[c/4].process(lsVal);
          if (compGr) grVal = grDownSample[c/4].process(grVal);
        }
        if (processLights && i == oversample-1) {
          anyEq = anyEq || eqVal.s[0]>mid || eqVal.s[1]>mid || eqVal.s[2]>mid || eqVal.s[3]>mid;
          anyNeq = anyNeq || neqVal.s[0]>mid || neqVal.s[1]>mid || neqVal.s[2]>mid || neqVal.s[3]>mid;
          anyLsEq = anyLsEq || leqVal.s[0]>mid || leqVal.s[1]>mid || leqVal.s[2]>mid || leqVal.s[3]>mid;
          anyGrEq = anyGrEq || geqVal.s[0]>mid || geqVal.s[1]>mid || geqVal.s[2]>mid || geqVal.s[3]>mid;
          anyLs = anyLs || lsVal.s[0]>mid || lsVal.s[1]>mid || lsVal.s[2]>mid || lsVal.s[3]>mid;
          anyGr = anyGr || grVal.s[0]>mid || grVal.s[1]>mid || grVal.s[2]>mid || grVal.s[3]>mid;
        }
      }
      outputs[MIN_OUTPUT].setVoltageSimd(minVal, c);
      outputs[MAX_OUTPUT].setVoltageSimd(maxVal, c);
      outputs[CLAMP_OUTPUT].setVoltageSimd(clampVal, c);
      outputs[OVER_OUTPUT].setVoltageSimd(overVal, c);
      outputs[EQ_OUTPUT].setVoltageSimd(eqVal, c);
      outputs[NEQ_OUTPUT].setVoltageSimd(neqVal, c);
      outputs[LSEQ_OUTPUT].setVoltageSimd(leqVal, c);
      outputs[GREQ_OUTPUT].setVoltageSimd(geqVal, c);
      outputs[LS_OUTPUT].setVoltageSimd(lsVal, c);
      outputs[GR_OUTPUT].setVoltageSimd(grVal, c);
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

    if (processLights) {
      float lightTime = args.sampleTime * lightDivider.getDivision();
      
      lights[OVERSAMPLE_LIGHT].setBrightness(oversample>1);

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
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "absMin", json_boolean(absPort[MIN_PORT]));
    json_object_set_new(rootJ, "absMax", json_boolean(absPort[MAX_PORT]));
    json_object_set_new(rootJ, "absClamp", json_boolean(absPort[CLAMP_PORT]));
    json_object_set_new(rootJ, "absOver", json_boolean(absPort[OVER_PORT]));
    json_object_set_new(rootJ, "invMin", json_boolean(invPort[MIN_PORT]));
    json_object_set_new(rootJ, "invMax", json_boolean(invPort[MAX_PORT]));
    json_object_set_new(rootJ, "invClamp", json_boolean(invPort[CLAMP_PORT]));
    json_object_set_new(rootJ, "invOver", json_boolean(invPort[OVER_PORT]));
    json_object_set_new(rootJ, "oversample", json_integer(oversample));
    json_object_set_new(rootJ, "gateType", json_integer(gateType));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
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
    val = json_object_get(rootJ, "oversample");
    if (val)
      oversample = json_integer_value(val);
    val = json_object_get(rootJ, "gateType");
    if (val)
      gateType = json_integer_value(val);

    initializeOversample();
  }

};


struct WinCompWidget : VenomWidget {

  struct AbsInvPort : PolyPort {
    int modId;
    void appendContextMenu(Menu* menu) override {
      WinComp* module = dynamic_cast<WinComp*>(this->module);
      assert(module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Absolute value", "", &module->absPort[modId]));
      menu->addChild(createBoolPtrMenuItem("Invert", "", &module->invPort[modId]));
      PolyPort::appendContextMenu(menu);
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
    setVenomPanel("WinComp");

    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.299, 15.93)), module, WinComp::A_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.134, 15.93)), module, WinComp::A_PARAM));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.299, 30.05)), module, WinComp::B_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.134, 30.05)), module, WinComp::B_PARAM));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.299, 43.87)), module, WinComp::TOL_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.134, 43.87)), module, WinComp::TOL_PARAM));

    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(7.299, 58.3)), module, WinComp::MIN_OUTPUT, WinComp::MIN_PORT));
    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(18.134, 58.3)), module, WinComp::MAX_OUTPUT, WinComp::MAX_PORT));
    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(7.299, 72.75)), module, WinComp::CLAMP_OUTPUT, WinComp::CLAMP_PORT));
    addOutput(createAbsInvOutputCentered<AbsInvPort>(mm2px(Vec(18.134, 72.75)), module, WinComp::OVER_OUTPUT, WinComp::OVER_PORT));

    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(7.299, 87.10)), module, WinComp::EQ_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(18.134, 87.10)), module, WinComp::NEQ_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(7.299, 101.55)), module, WinComp::LSEQ_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(18.134, 101.55)), module, WinComp::GREQ_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(7.299, 116.0)), module, WinComp::LS_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(18.134, 116.0)), module, WinComp::GR_OUTPUT));


    addChild(createLightCentered<SmallLight<BluLight<>>>(mm2px(Vec(12.7, 51.5)), module, WinComp::OVERSAMPLE_LIGHT));
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
    std::vector<std::string> gateLabels = {
      "0,1",
      "+/-1",
      "0,5",
      "+/-5",
      "0,10",
      "+/-10"
    };
    menu->addChild(createIndexSubmenuItem("Gate voltages", gateLabels,
      [=]() {return module->gateType;},
      [=](int i) {module->gateType = i;}
    ));
    menu->addChild(createIndexSubmenuItem("Oversample", module->oversampleLabels,
      [=]() {
          std::vector<int> vals = module->oversampleValues;
          std::vector<int>::iterator itr = std::find(vals.begin(), vals.end(), module->oversample);
          return std::distance(vals.begin(), itr);
        },
      [=](int i) {
          module->oversample = module->oversampleValues[i];
          module->initializeOversample();
        }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelWinComp = createModel<WinComp, WinCompWidget>("WinComp");
