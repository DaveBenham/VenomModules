// Venom Modules (c) 2023, 2024, 2025 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

struct Compare2 : VenomModule {

  enum ParamId {
    SHIFT1_PARAM,
    SIZE1_PARAM,
    SIZE2_PARAM,
    SHIFT2_PARAM,
    RANGE_PARAM,
    OVER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SHIFT1_INPUT,
    SIZE1_INPUT,
    SIZE2_INPUT,
    SHIFT2_INPUT,
    IN1_INPUT,
    IN2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GTR1_OUTPUT,
    NGTR1_OUTPUT,
    NGTR2_OUTPUT,
    GTR2_OUTPUT,
    EQ1_OUTPUT,
    NEQ1_OUTPUT,
    NEQ2_OUTPUT,
    EQ2_OUTPUT,
    LSS1_OUTPUT,
    NLSS1_OUTPUT,
    NLSS2_OUTPUT,
    LSS2_OUTPUT,
    GTR_AND_OUTPUT,
    GTR_OR_OUTPUT,
    GTR_XOR_OUTPUT,
    GTR_FF_OUTPUT,
    EQ_AND_OUTPUT,
    EQ_OR_OUTPUT,
    EQ_XOR_OUTPUT,
    EQ_FF_OUTPUT,
    LSS_AND_OUTPUT,
    LSS_OR_OUTPUT,
    LSS_XOR_OUTPUT,
    LSS_FF_OUTPUT,
    OUTPUTS_LEN
  };

  using float_4 = simd::float_4;
  
  int oversample = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  int monitor = 1;
  int lightWait = 0;
  float_4 xorOld[4][3]{}, ffOld[4][3]{};
  OversampleFilter_4 upSample[4][INPUTS_LEN]{}, downSample[4][OUTPUTS_LEN]{};
  float outScale[6]{1.f,5.f,10.f,2.f,10.f,20.f},
        outOffset[6]{0.f,0.f,0.f,-1.f,-5.f,-10.f};

  void configOut(int id, std::string label){
    configOutput(id,label);
    configLight(id,label+" indicator");
  }

  Compare2() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, OUTPUTS_LEN);

    configParam(SHIFT1_PARAM, -10.f, 10.f, 0.f, "Shift 1");
    configParam(SIZE1_PARAM, 0.f, 10.f, 5.f, "Size 1");
    configParam(SIZE2_PARAM, 0.f, 10.f, 5.f, "Size 2");
    configParam(SHIFT2_PARAM, -10.f, 10.f, 0.f, "Shift 2");
    
    configInput(SHIFT1_INPUT, "Shift 1");
    configInput(SIZE1_INPUT, "Size 1");
    configInput(SIZE2_INPUT, "Size 2");
    configInput(SHIFT2_INPUT, "Shift 2");

    configSwitch<FixedSwitchQuantity>(RANGE_PARAM, 0.f, 5.f, 2.f, "Output range", {"0-1V", "0-5V", "0-10V", "+/- 1V", "+/- 5V", "+/- 10V"});
    configInput(IN1_INPUT, "In 1");
    configInput(IN2_INPUT, "In 2");
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configOut(GTR1_OUTPUT,"In 1 greater");
    configOut(NGTR1_OUTPUT,"In 1 not greater");
    configOut(NGTR2_OUTPUT,"In 2 not greater");
    configOut(GTR2_OUTPUT,"In 2 greater");

    configOut(EQ1_OUTPUT,"In 1 equal");
    configOut(NEQ1_OUTPUT,"In 1 not equal");
    configOut(NEQ2_OUTPUT,"In 2 not equal");
    configOut(EQ2_OUTPUT,"In 2 equal");

    configOut(LSS1_OUTPUT,"In 1 less");
    configOut(NLSS1_OUTPUT,"In 1 not less");
    configOut(NLSS2_OUTPUT,"In 2 not less");
    configOut(LSS2_OUTPUT,"In 2 less");
    
    configOut(GTR_AND_OUTPUT,"In 1 AND In 2 greater");
    configOut(GTR_OR_OUTPUT,"In 1 OR In 2 greater");
    configOut(GTR_XOR_OUTPUT,"In 1 XOR In 2 greater");
    configOut(GTR_FF_OUTPUT,"XOR greater flip flop");
    
    configOut(EQ_AND_OUTPUT,"In 1 AND In 2 equal");
    configOut(EQ_OR_OUTPUT,"In 1 OR In 2 equal");
    configOut(EQ_XOR_OUTPUT,"In 1 XOR In 2 equal");
    configOut(EQ_FF_OUTPUT,"XOR equal flip flop");
    
    configOut(LSS_AND_OUTPUT,"In 1 AND In 2 less");
    configOut(LSS_OR_OUTPUT,"In 1 OR In 2 less");
    configOut(LSS_XOR_OUTPUT,"In 1 XOR In 2 less");
    configOut(LSS_FF_OUTPUT,"XOR less flip flop");

    oversampleStages = 5;
  }

  void setOversample() override {
    oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
    if (oversample > 1) {
      for (int s=0; s<4; s++){
        for (int i=0; i<INPUTS_LEN; i++)
          upSample[s][i].setOversample(oversample, oversampleStages);
        for (int i=0; i<OUTPUTS_LEN; i++)
          downSample[s][i].setOversample(oversample, oversampleStages);
      }
    }
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    bool hasIn[INPUTS_LEN]{}, hasOut[OUTPUTS_LEN]{};
    float_4 in[INPUTS_LEN]{}, out[4][OUTPUTS_LEN]{}, out2[OUTPUTS_LEN]{};
    float_4 f4_0=float_4::zero(), f4_1=1.f, f4_2=2.f;
    
    // update oversample configuration
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())])
      setOversample();
    // compute channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max({channels, inputs[i].getChannels()});
    // compute patched ports
    for (int i=0; i<INPUTS_LEN; i++)
      hasIn[i] = inputs[i].isConnected();
    hasIn[SHIFT2_INPUT] |= hasIn[SHIFT1_INPUT];
    hasIn[SIZE2_INPUT] |= hasIn[SIZE1_INPUT];
    hasIn[IN2_INPUT] |= hasIn[IN1_INPUT];
    for (int i=0; i<OUTPUTS_LEN; i++)
      hasOut[i] = outputs[i].isConnected();
    // get output range
    float scale = outScale[static_cast<int>(params[RANGE_PARAM].getValue())];
    float offset = outOffset[static_cast<int>(params[RANGE_PARAM].getValue())];
    // channel loop
    for (int s=0, c=0; c<channels; s++, c+=4){
      // oversample loop
      for (int o=0; o<oversample; o++) {
        // read inputs
        if (!o) {
          in[SHIFT1_INPUT] = inputs[SHIFT1_INPUT].getPolyVoltageSimd<float_4>(c);
          in[SHIFT2_INPUT] = inputs[SHIFT2_INPUT].getNormalPolyVoltageSimd(in[SHIFT1_INPUT], c);
          in[SIZE1_INPUT] = inputs[SIZE1_INPUT].getPolyVoltageSimd<float_4>(c);
          in[SIZE2_INPUT] = inputs[SIZE2_INPUT].getNormalPolyVoltageSimd(in[SIZE1_INPUT], c);
          in[IN1_INPUT] = inputs[IN1_INPUT].getPolyVoltageSimd<float_4>(c);
          in[IN2_INPUT] = inputs[IN2_INPUT].getNormalPolyVoltageSimd(in[IN1_INPUT], c);
        }
        // upsample inputs
        if (oversample > 1){
          for (int i=0; i<INPUTS_LEN; i++) {
            if (hasIn[i])
              in[i] = upSample[s][i].process(o ? float_4::zero() : in[i]*oversample);
          }
        }
        // compute windows
        float_4 size = in[SIZE1_INPUT] + params[SIZE1_PARAM].getValue();
        size = ifelse(size >= f4_0, fmax(size, 2e-6f), fmin(size, -2e-6f));
        float_4 hi1 = in[SHIFT1_INPUT] + params[SHIFT1_PARAM].getValue() + size/2.f;
        float_4 lo1 = hi1 - size;
        size = fmax(in[SIZE2_INPUT] + params[SIZE2_PARAM].getValue(), 1e-6f);
        size = ifelse(size >= f4_0, fmax(size, 2e-6f), fmin(size, -2e-6f));
        float_4 hi2 = in[SHIFT2_INPUT] + params[SHIFT2_PARAM].getValue() + size/2.f;
        float_4 lo2 = hi2 - size;
        // compute compare outs
        out[s][GTR1_OUTPUT] = ifelse(in[IN1_INPUT] > hi1, f4_1, f4_0);
        out[s][LSS1_OUTPUT] = ifelse(in[IN1_INPUT] < lo1, f4_1, f4_0);
        out[s][EQ1_OUTPUT] = ifelse(out[s][GTR1_OUTPUT]+out[s][LSS1_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][NGTR1_OUTPUT] = ifelse(out[s][GTR1_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][NEQ1_OUTPUT] = ifelse(out[s][EQ1_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][NLSS1_OUTPUT] = ifelse(out[s][LSS1_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][GTR2_OUTPUT] = ifelse(in[IN2_INPUT] > hi2, f4_1, f4_0);
        out[s][LSS2_OUTPUT] = ifelse(in[IN2_INPUT] < lo2, f4_1, f4_0);
        out[s][EQ2_OUTPUT] = ifelse(out[s][GTR2_OUTPUT]+out[s][LSS2_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][NGTR2_OUTPUT] = ifelse(out[s][GTR2_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][NEQ2_OUTPUT] = ifelse(out[s][EQ2_OUTPUT] == f4_0, f4_1, f4_0);
        out[s][NLSS2_OUTPUT] = ifelse(out[s][LSS2_OUTPUT] == f4_0, f4_1, f4_0);
        // compute logic outs
        out[s][GTR_AND_OUTPUT] = ifelse(out[s][GTR1_OUTPUT]+out[s][GTR2_OUTPUT] == f4_2, f4_1, f4_0);
        out[s][GTR_OR_OUTPUT] = ifelse(out[s][GTR1_OUTPUT]+out[s][GTR2_OUTPUT] > f4_0, f4_1, f4_0);
        out[s][GTR_XOR_OUTPUT] = ifelse(out[s][GTR1_OUTPUT] != out[s][GTR2_OUTPUT], f4_1, f4_0);
        out[s][EQ_AND_OUTPUT] = ifelse(out[s][EQ1_OUTPUT]+out[s][EQ2_OUTPUT] == f4_2, f4_1, f4_0);
        out[s][EQ_OR_OUTPUT] = ifelse(out[s][EQ1_OUTPUT]+out[s][EQ2_OUTPUT] > f4_0, f4_1, f4_0);
        out[s][EQ_XOR_OUTPUT] = ifelse(out[s][EQ1_OUTPUT] != out[s][EQ2_OUTPUT], f4_1, f4_0);
        out[s][LSS_AND_OUTPUT] = ifelse(out[s][LSS1_OUTPUT]+out[s][LSS2_OUTPUT] == f4_2, f4_1, f4_0);
        out[s][LSS_OR_OUTPUT] = ifelse(out[s][LSS1_OUTPUT]+out[s][LSS2_OUTPUT] > f4_0, f4_1, f4_0);
        out[s][LSS_XOR_OUTPUT] = ifelse(out[s][LSS1_OUTPUT] != out[s][LSS2_OUTPUT], f4_1, f4_0);
        // compute flip flop outs
        for (int i=0; i<4; i++){
          if (xorOld[s][0][i] != out[s][GTR_XOR_OUTPUT][i]){
            xorOld[s][0][i] = out[s][GTR_XOR_OUTPUT][i];
            if (out[s][GTR_XOR_OUTPUT][i])
              ffOld[s][0][i] = !ffOld[s][0][i];
          }
          if (xorOld[s][1][i] != out[s][EQ_XOR_OUTPUT][i]){
            xorOld[s][1][i] = out[s][EQ_XOR_OUTPUT][i];
            if (out[s][EQ_XOR_OUTPUT][i])
              ffOld[s][1][i] = !ffOld[s][1][i];
          }
          if (xorOld[s][2][i] != out[s][LSS_XOR_OUTPUT][i]){
            xorOld[s][2][i] = out[s][LSS_XOR_OUTPUT][i];
            if (out[s][LSS_XOR_OUTPUT][i])
              ffOld[s][2][i] = !ffOld[s][2][i];
          }
        }
        out[s][GTR_FF_OUTPUT] = ffOld[s][0];
        out[s][EQ_FF_OUTPUT] = ffOld[s][1];
        out[s][LSS_FF_OUTPUT] = ffOld[s][2];
        // downsample outputs if needed
        for (int i=0; i<OUTPUTS_LEN; i++){
          out2[i] = oversample>1 && hasOut[i] ? downSample[s][i].process(out[s][i]) : out[s][i];
        }
      } // end oversample loop
      // write outputs
      for (int i=0; i<OUTPUTS_LEN; i++)
        outputs[i].setVoltageSimd(out2[i]*scale+offset, c);
    } // end channel loop
    // set output channel counts
    for (int i=0; i<OUTPUTS_LEN; i++)
      outputs[i].setChannels(channels);
    if (++lightWait>5) {
      lightWait = 0;
      int start = monitor<=1 ? 0 : monitor-2,
          end = monitor==0 ? -1 : (monitor==1 ? channels : monitor - 1),
          div = end - start;
      if (end>channels)
        end = -1;
      for (int i=0; i<OUTPUTS_LEN; i++){
        float val = 0;
        for (int c=start; c<end; c++)
          val += out[c/4][i][c%4];
        lights[i].setBrightnessSmooth(val/div, args.sampleTime*5);
      }
    }
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "monitorChannel", json_integer(monitor));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "monitorChannel")))
      monitor = json_integer_value(val);
  }

};

struct Compare2Widget : VenomWidget {

  struct RangeSwitch : GlowingSvgSwitchLockable {
    RangeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_0-1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_0-5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_0-10.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_pm1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_pm5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_pm10.svg")));
    }
  };

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_off.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_8.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_16.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_32.svg")));
    }
  };

  Compare2Widget(Compare2* module) {
    setModule(module);
    setVenomPanel("Compare2");

    addParam(createLockableParam<RoundSmallBlackKnobLockable>(Vec(14.5f, 38.5f), module, Compare2::SHIFT1_PARAM));
    addParam(createLockableParam<RoundSmallBlackKnobLockable>(Vec(47.5f, 38.5f), module, Compare2::SIZE1_PARAM));
    addParam(createLockableParam<RoundSmallBlackKnobLockable>(Vec(80.5f, 38.5f), module, Compare2::SIZE2_PARAM));
    addParam(createLockableParam<RoundSmallBlackKnobLockable>(Vec(113.5f, 38.5f), module, Compare2::SHIFT2_PARAM));

    addInput(createInput<PolyPort>(Vec(14.f, 68.5f), module, Compare2::SHIFT1_INPUT));
    addInput(createInput<PolyPort>(Vec(47.f, 68.5f), module, Compare2::SIZE1_INPUT));
    addInput(createInput<PolyPort>(Vec(80.f, 68.5f), module, Compare2::SIZE2_INPUT));
    addInput(createInput<PolyPort>(Vec(113.f, 68.5f), module, Compare2::SHIFT2_INPUT));

    addParam(createLockableParam<RangeSwitch>(Vec(13.5f, 114.f), module, Compare2::RANGE_PARAM));
    addInput(createInput<PolyPort>(Vec(47.f, 114.f), module, Compare2::IN1_INPUT));
    addInput(createInput<PolyPort>(Vec(80.f, 114.f), module, Compare2::IN2_INPUT));
    addParam(createLockableParam<OverSwitch>(Vec(112.5f, 114.f), module, Compare2::OVER_PARAM));

    int id=0;
    // compare outputs
    for (float y=159.f; y<=221.f; y+=31.f){
      for (float x=14.f; x<=113.f; x+=33.f){
        addOutput(createOutput<PolyPort>(Vec(x,y), module, id));
        addChild(createLight<SmallLight<YellowLight>>(Vec(x+21.f, y-2.f), module, id++));
      }
    }
    // logic outputs
    for (float y=264.f; y<=326.f; y+=31.f){
      for (float x=14.f; x<=113.f; x+=33.f){
        addOutput(createOutput<PolyPort>(Vec(x,y), module, id));
        addChild(createLight<SmallLight<YellowLight>>(Vec(x+21.f, y-2.f), module, id++));
      }
    }
  }

  void appendContextMenu(Menu* menu) override {
    Compare2* module = static_cast<Compare2*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexPtrSubmenuItem(
      "Monitor channel",
      {"Off","All","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      &module->monitor
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelCompare2 = createModel<Compare2, Compare2Widget>("Compare2");
