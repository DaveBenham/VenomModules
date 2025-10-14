// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

struct QuadVCPolarizer : VenomModule {

  enum ParamId {
    NORM_PARAM,
    VCA_MODE_PARAM,
    UNITY_PARAM,
    CLIP_PARAM,
    ENUMS(LEVEL_PARAM,4),
    ENUMS(LEVEL_AMT_PARAM,4),
    OVER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(LEVEL_INPUT,4),
    ENUMS(POLY_INPUT,4),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(POLY_OUTPUT,4),
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  int oversample = -1;
  int oversampleEnd = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  OversampleFilter_4 inUpSample[4][4]{}, cvUpSample[4][4]{}, outDownSample[4][4]{};
  
  QuadVCPolarizer() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(NORM_PARAM, 0.f, 1.f, 0.f, "Input normal", {"5V", "10V"});
    configSwitch<FixedSwitchQuantity>(VCA_MODE_PARAM, 0.f, 2.f, 2.f, "VCA CV", {"Unipolar clamped", "Bipolar clamped", "Bipolar unbounded"});
    configSwitch<FixedSwitchQuantity>(UNITY_PARAM, 0.f, 1.f, 0.f, "Unity voltage", {"5V", "10V"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 4.f, 0.f, "Output clipping", {"Off", "Hard +/-10V", "Hard +/-5V", "Soft +/-12V", "Soft +/-6V"});
    for (int i=0; i<4; i++){
      configParam(LEVEL_PARAM+i, -1.f, 1.f, 0.f, string::f("Level %d", i + 1), "%", 0, 100, 0);
      configParam(LEVEL_AMT_PARAM+i, -1.f, 1.f, 0.f, string::f("Level %d CV amount", i + 1), "%", 0, 100, 0);
      configInput(LEVEL_INPUT+i, string::f("Level %d CV", i + 1));
      configInput(POLY_INPUT+i, string::f("Poly %d", i + 1));
      configOutput(POLY_OUTPUT+i, string::f("Poly %d", i + 1));
    }
    oversampleStages = 5;
  }
  
  void setOversample() override {
    for (int i=0; i<4; i++){
      for (int j=0; j<4; j++){
        inUpSample[i][j].setOversample(oversample, oversampleStages);
        cvUpSample[i][j].setOversample(oversample, oversampleStages);
        outDownSample[i][j].setOversample(oversample, oversampleStages);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    int channels[4]{}, outPort[4]{-1,-1,-1,-1};
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      oversampleEnd = oversample-1;
      setOversample();
    }
    for (int i=3, j=2; i>=0; i=j--){
      if (outputs[POLY_OUTPUT+i].isConnected()){
        outPort[i] = i;
        channels[i] = std::max({1, inputs[POLY_INPUT+i].getChannels(), inputs[LEVEL_INPUT+i].getChannels()});
        for (; j>=0 && !outputs[POLY_OUTPUT+j].isConnected(); j--){
          outPort[j] = i;
          channels[i] = std::max({channels[i], inputs[POLY_INPUT+j].getChannels(), inputs[LEVEL_INPUT+j].getChannels()});
        }
      }
    }
    float norm = params[NORM_PARAM].getValue() ? 10.f : 5.f;
    float unity = params[UNITY_PARAM].getValue() ? 10.f : 5.f;
    int vca = static_cast<int>(params[VCA_MODE_PARAM].getValue());
    int clip = static_cast<int>(params[CLIP_PARAM].getValue());
    for (int o=0; o<oversample; o++){
      float_4 out[4][4]{};
      for (int i=0; i<4 && outPort[i]>=0; i++){
        float_4 cv{};
        float_4 in{};
        for (int c=0, j=0; c<channels[outPort[i]]; c+=4, j++){
          if (c==0 || inputs[LEVEL_INPUT+i].isPolyphonic()){
            cv = o ? float_4::zero() : inputs[LEVEL_INPUT+i].getPolyVoltageSimd<float_4>(c);
            if (oversample>1){
              if (!o) cv*=oversample;
              cv = cvUpSample[i][j].process(cv);
            }
          }
          cv *= params[LEVEL_AMT_PARAM+i].getValue()/unity;
          if (vca<2)
            cv = simd::clamp(cv, vca ? -1.f : 0.f, 1.f);
          if (c==0 || inputs[POLY_INPUT+i].isPolyphonic()){
            in = o ? float_4::zero() : inputs[POLY_INPUT+i].getNormalPolyVoltageSimd<float_4>(norm, c);
            if (oversample>1){
              if (!o) in*=oversample;
              in = inUpSample[i][j].process(in);
            }
          }
          out[outPort[i]][j] += in * simd::clamp(cv + params[LEVEL_PARAM+i].getValue(), -2.f, 2.f);
          if (i == outPort[i]){
            switch(clip) {
              case 1: // hard 10V
                out[i][j] = clamp(out[i][j], -10.f, 10.f);
                break;
              case 2: // hard 5V
                out[i][j] = clamp(out[i][j], -5.f, 5.f);
                break;
              case 3: // soft 10V
              case 4: // soft 6V
                float limit = 10.f / (clip==3 ? 12.f : 6.f);
                out[i][j] = softClip(out[i][j]*limit) / limit;
                break;
            }
            if (oversample>1)
              out[i][j] = outDownSample[i][j].process(out[i][j]);
            if (o==oversampleEnd)  
              outputs[POLY_OUTPUT+i].setVoltageSimd(out[i][j], c);
          }
        }
        if (o==oversampleEnd && i == outPort[i])
          outputs[POLY_OUTPUT+i].setChannels(channels[i]);
      }
    }
  }

};

struct QuadVCPolarizerWidget : VenomWidget {

  QuadVCPolarizer* mod = NULL;

  struct LabelsWidget : widget::Widget {
    QuadVCPolarizer* mod=NULL;
    std::string modName;
    std::string fontPath;
  
    LabelsWidget() {
      fontPath = asset::system("res/fonts/DejaVuSans.ttf");
    }
  
    void draw(const DrawArgs& args) override {
      int theme = mod ? mod->currentTheme : 0;
      if (!theme) theme = settings::preferDarkPanels ? getDefaultDarkTheme()+1 : getDefaultTheme()+1;
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (!font)
        return;
      nvgFontFaceId(args.vg, font->handle);
      nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_BOTTOM);
      nvgFontSize(args.vg, 7);
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
      std::string text = mod ? (mod->params[QuadVCPolarizer::NORM_PARAM].getValue()==0.f ? "5V" : "10V") : "5V";
      for (int i=0; i<4; i++) {
        nvgText(args.vg, 15.f, 78.75f+i*80, text.c_str(), NULL);
      }
    }
  };

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  struct NormSwitch : GlowingSvgSwitchLockable {
    NormSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  struct VCASwitch : GlowingSvgSwitchLockable {
    VCASwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  struct UnitySwitch : GlowingSvgSwitchLockable {
    UnitySwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  QuadVCPolarizerWidget(QuadVCPolarizer* module) {
    setModule(module);
    setVenomPanel("QuadVCPolarizer");

    LabelsWidget* text = createWidget<LabelsWidget>(Vec(0.f,0.f));
    text->mod = module;
    text->box.size = box.size;
    addChild(text);

    addParam(createLockableParamCentered<OverSwitch>(Vec(9.78f,38.202f), module, QuadVCPolarizer::OVER_PARAM));
    addParam(createLockableParamCentered<NormSwitch>(Vec(23.64f,38.202f), module, QuadVCPolarizer::NORM_PARAM));
    addParam(createLockableParamCentered<VCASwitch>(Vec(37.50f,38.202f), module, QuadVCPolarizer::VCA_MODE_PARAM));
    addParam(createLockableParamCentered<UnitySwitch>(Vec(51.36f,38.202f), module, QuadVCPolarizer::UNITY_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(65.22f,38.202f), module, QuadVCPolarizer::CLIP_PARAM));

    for (int i=0; i<4; i++){
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(15.f, 56.f + 80.f*i), module, QuadVCPolarizer::LEVEL_PARAM+i));
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(60.f, 56.f + 80.f*i), module, QuadVCPolarizer::LEVEL_AMT_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(37.5f, 73.f + 80.f*i), module, QuadVCPolarizer::LEVEL_INPUT+i));
      addInput(createInputCentered<PolyPort>(Vec(15.f, 94.f + 80.f*i), module, QuadVCPolarizer::POLY_INPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(60.f, 94.f + 80.f*i), module, QuadVCPolarizer::POLY_OUTPUT+i));
    }
    
  }

};

Model* modelVenomQuadVCPolarizer = createModel<QuadVCPolarizer, QuadVCPolarizerWidget>("QuadVCPolarizer");
