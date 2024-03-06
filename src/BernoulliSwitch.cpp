// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"

#define LIGHT_OFF 0.02f
#define FADE_RATE 100.f

struct BernoulliSwitch : VenomModule {
  #include "BernoulliSwitchExpander.hpp"
  
  enum ParamId {
    PROB_PARAM,
    TRIG_PARAM,
    MODE_PARAM,
    RISE_PARAM,
    FALL_PARAM,
    OFFSET_A_PARAM,
    OFFSET_B_PARAM,
    SCALE_A_PARAM,
    SCALE_B_PARAM,
    NORMAL_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    TRIG_INPUT,
    PROB_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    A_OUTPUT,
    B_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    NO_SWAP_LIGHT,
    SWAP_LIGHT,
    TRIG_LIGHT,
    ENUMS(AUDIO_LIGHT, 2),
    POLY_SENSE_ALL_LIGHT,
    LIGHTS_LEN
  };
  enum ProbMode {
    TOGGLE_MODE,
    SWAP_MODE,
    GATE_MODE
  };

  dsp::SchmittTrigger trig[PORT_MAX_CHANNELS];
  dsp::SchmittTrigger normTrig[PORT_MAX_CHANNELS];
  bool swap[PORT_MAX_CHANNELS];
  dsp::SlewLimiter fade[PORT_MAX_CHANNELS];
  int oldChannels = 0;
  int lightChannel = 0;
  bool lightOff = false;
  bool inputPolyControl = false;
  std::vector<int> oversampleValues = {1,1,2,4,8,16};
  int audioProc = 0;
  int oldAudioProc = -1;
  bool deClick = false;
  int oversample = 0;

  OversampleFilter_4 aUpSample[4], bUpSample[4],
                     aDownSample[4], bDownSample[4],
                     trigUpSample[4];

  BernoulliSwitch() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(PROB_PARAM, 0.f, 1.f, 0.5f, "Probability", "%", 0.f, 100.f, 0.f);
    configButton(TRIG_PARAM, "Manual 10V Trigger");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0, 2, 1, "Probability Mode", {"Toggle", "Swap", "Gate"});
    configParam(RISE_PARAM, -10.f, 10.f, 1.f, "Rise Threshold", " V");
    configParam(FALL_PARAM, -10.f, 10.f, 0.1f, "Fall Threshold", " V");
    configParam(OFFSET_A_PARAM, -10.f, 10.f, 0.f, "A Offset", " V");
    configParam(OFFSET_B_PARAM, -10.f, 10.f, 0.f, "B Offset", " V");
    configParam(SCALE_A_PARAM, -1.f, 1.f, 1.f, "A Scale", "");
    configParam(SCALE_B_PARAM, -1.f, 1.f, 1.f, "B Scale", "");
    configInput(A_INPUT, "A")->description = "Normalled to trigger input";
    configInput(B_INPUT, "B");
    configInput(TRIG_INPUT, "Trigger");
    configInput(PROB_INPUT, "Probability");
    configSwitch<FixedSwitchQuantity>(NORMAL_PARAM, 0.f, 1.f, 0.f, "Normal value", {"Raw trigger input", "Schmitt trigger result"});
    configOutput(A_OUTPUT, "A");
    configOutput(B_OUTPUT, "B");
    configBypass(A_INPUT, A_OUTPUT);
    configBypass(inputs[B_INPUT].isConnected() ? B_INPUT : A_INPUT, B_OUTPUT);
    configLight(NO_SWAP_LIGHT, "No-swap indicator");
    configLight(SWAP_LIGHT, "Swap indicator");
    configLight(POLY_SENSE_ALL_LIGHT, "Polyphony sense all indicator");
    configLight(AUDIO_LIGHT, "Audio processing indicator")->description = "red = antipop, blue = oversample";
    lights[NO_SWAP_LIGHT].setBrightness(true);
    lights[SWAP_LIGHT].setBrightness(false);
    lights[POLY_SENSE_ALL_LIGHT].setBrightness(false);
    for (int i=0; i<PORT_MAX_CHANNELS; i++)
      fade[i].rise = fade[i].fall = FADE_RATE;
  }

  void onPortChange(const PortChangeEvent& e) override {
    if (e.type == Port::INPUT && e.portId == B_INPUT)
      bypassRoutes[1].inputId = e.connecting ? B_INPUT : A_INPUT;
  }

  void onReset() override {
    oldChannels = 0;
    lights[NO_SWAP_LIGHT].setBrightness(true);
    lights[SWAP_LIGHT].setBrightness(false);
  }
  
  // Bug fix for VCV bug in onExpanderChange (not triggered by module deletion)
  void onRemove(const RemoveEvent& e) override {
    Module* expander = getRightExpander().module;
    if (expander && expander->model == modelBernoulliSwitchExpander)
      expander->lights[EXPAND_LIGHT].setBrightness(0.f);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    float_4 aOut[4], bOut[4];
    Module* expanderCandidate = getRightExpander().module;
    Module* expander = expanderCandidate 
                    && !expanderCandidate->isBypassed() 
                    && expanderCandidate->model == modelBernoulliSwitchExpander 
                     ? expanderCandidate 
                     : NULL;
    float scaleA = params[SCALE_A_PARAM].getValue(),
          scaleB = params[SCALE_B_PARAM].getValue(),
          offA = params[OFFSET_A_PARAM].getValue(),
          offB = params[OFFSET_B_PARAM].getValue(),
          rise = params[RISE_PARAM].getValue(),
          fall = params[FALL_PARAM].getValue(),
          probOff = params[PROB_PARAM].getValue(),
          manual = params[TRIG_PARAM].getValue() > 0.f ? 10.f : 0.f,
          probAttn = 1.f;
    bool schmittNorm = params[NORMAL_PARAM].getValue();
    if (expander ) {
      probAttn = expander->getParam(PROB_CV_PARAM).getValue();
      if (expander->getInput(SCALE_CV_A_INPUT).isConnected())
        scaleA = scaleA + expander->getInput(SCALE_CV_A_INPUT).getVoltage()*expander->getParam(SCALE_CV_A_PARAM).getValue()/10.f;
      if (expander->getInput(SCALE_CV_B_INPUT).isConnected())
        scaleB = scaleB + expander->getInput(SCALE_CV_B_INPUT).getVoltage()*expander->getParam(SCALE_CV_B_PARAM).getValue()/10.f;
      if (expander->getInput(OFFSET_CV_A_INPUT).isConnected())
        offA += expander->getInput(OFFSET_CV_A_INPUT).getVoltage()*expander->getParam(OFFSET_CV_A_PARAM).getValue();
      if (expander->getInput(OFFSET_CV_B_INPUT).isConnected())
        offB += expander->getInput(OFFSET_CV_B_INPUT).getVoltage()*expander->getParam(OFFSET_CV_B_PARAM).getValue();
      if (expander->getInput(RISE_CV_INPUT).isConnected())
        rise += expander->getInput(RISE_CV_INPUT).getVoltage()*expander->getParam(RISE_CV_PARAM).getValue();
      if (expander->getInput(FALL_CV_INPUT).isConnected())
        fall += expander->getInput(FALL_CV_INPUT).getVoltage()*expander->getParam(FALL_CV_PARAM).getValue();
    }  
    bool invTrig = rise < fall;
    int aChannels = std::max(1, inputs[A_INPUT].getChannels());
    int bChannels = std::max(1, inputs[B_INPUT].getChannels());
    int mode = expander && expander->getInput(MODE_CV_INPUT).isConnected() 
             ? clamp(static_cast<int>(expander->getInput(MODE_CV_INPUT).getVoltage())+1, 0, 2) 
             : static_cast<int>(params[MODE_PARAM].getValue());
    lights[TRIG_LIGHT].setBrightness(manual ? 1.f : LIGHT_OFF);
    if (invTrig) {
      rise = -rise;
      fall = -fall;
    }
    int channels = std::max({1, inputs[TRIG_INPUT].getChannels(), inputs[PROB_INPUT].getChannels()});
    if (inputPolyControl)
      channels = std::max({channels, aChannels, bChannels});
    int xChannels = channels;
    if (channels > oldChannels) {
      for (int c=oldChannels; c<channels; c++){
        trig[c].reset();
        normTrig[c].reset();
        swap[c] = false;
        fade[c].out = 0.f;
      }
      oldChannels = channels;
    }
    if (!lightOff && lightChannel >= channels) {
      lights[NO_SWAP_LIGHT].setBrightness(false);
      lights[SWAP_LIGHT].setBrightness(false);
      lightOff = true;
    }
    if (lightOff && lightChannel < channels) {
      lights[NO_SWAP_LIGHT].setBrightness(!swap[lightChannel]);
      lights[SWAP_LIGHT].setBrightness(swap[lightChannel]);
      lightOff = false;
    }
    if (audioProc != oldAudioProc) {
      oldAudioProc = audioProc;
      oversample = oversampleValues[audioProc];
      deClick = (audioProc == 1);
      lights[AUDIO_LIGHT].setBrightness(deClick);
      lights[AUDIO_LIGHT+1].setBrightness(audioProc>1);
      for (int c=0; c<4; c++) {
        aUpSample[c].setOversample(oversample);
        bUpSample[c].setOversample(oversample);
        aDownSample[c].setOversample(oversample);
        bDownSample[c].setOversample(oversample);
        trigUpSample[c].setOversample(oversample);
      }
    }
    lights[POLY_SENSE_ALL_LIGHT].setBrightness(inputPolyControl);

    float_4 trigIn0, trigIn;
    for (int c=0; c<channels; c+=4){
      float_4 prob = inputs[PROB_INPUT].getPolyVoltageSimd<float_4>(c)*probAttn/10.f + probOff;
      trigIn = inputs[TRIG_INPUT].getPolyVoltageSimd<float_4>(c) + manual;
      trigIn0 = trigIn;
      for (int j=0; j<4 && c+j<channels; j++) {
        int cj = c + j;
        normTrig[cj].process(invTrig ? -trigIn0.s[j] : trigIn0.s[j], fall, rise);
        if (schmittNorm)
          trigIn0.s[j] = normTrig[cj].isHigh() ? (invTrig ? -10.f : 10.f) : 0.f;
      }  
      float_4 aIn, bIn, swapGain, remainderGain;
      for (int i=0; i<oversample; i++) {
        if (oversample > 1)
          trigIn = trigUpSample[c/4].process(i ? float_4::zero() : trigIn * oversample);
        for (int j=0; j<4 && c+j<channels; j++) {
          int cj = c + j;
          if(trig[cj].process(invTrig ? -trigIn.s[j] : trigIn.s[j], fall, rise)){
            bool toss = (prob.s[j] == 1.0f || random::uniform() < prob.s[j]);
            switch(mode) {
              case TOGGLE_MODE:
                if (toss) swap[cj] = !swap[cj];
                break;
              case SWAP_MODE:
                swap[cj] = toss;
                break;
              case GATE_MODE:
                swap[cj] = !toss;
                break;
            }
            if (i == oversample-1 && cj == lightChannel) {
              lights[NO_SWAP_LIGHT].setBrightness(!swap[cj]);
              lights[SWAP_LIGHT].setBrightness(swap[cj]);
            }
          }
          if (mode == GATE_MODE && !swap[cj] && !trig[cj].isHigh()) {
            swap[cj] = true;
            if (i == oversample-1 && cj == lightChannel){
              lights[NO_SWAP_LIGHT].setBrightness(false);
              lights[SWAP_LIGHT].setBrightness(true);
            }
          }
          if (deClick)
            fade[cj].process(args.sampleTime, swap[cj]);
          else
            fade[cj].out = swap[cj];
        }

        int c2End = c+1;
        if (channels == 1 && !inputPolyControl) {
          c2End = xChannels = aChannels > bChannels ? aChannels : bChannels;
          trigIn0.s[1] = trigIn0.s[2] = trigIn0.s[3] = trigIn0.s[0];
        }
        for (int c2=c; c2<c2End; c2+=4) {
          int c0 = c2/4;
          aIn = i ? float_4::zero() : inputs[A_INPUT].getNormalPolyVoltageSimd<float_4>(trigIn0, c2) * scaleA + offA;
          bIn = i ? float_4::zero() : inputs[B_INPUT].getPolyVoltageSimd<float_4>(c2) * scaleB + offB;
          if (oversample>1) {
            aIn = aUpSample[c0].process(aIn * oversample);
            bIn = bUpSample[c0].process(bIn * oversample);
          }
          swapGain = (channels == 1 && !inputPolyControl ? float_4(fade[0].out) : float_4(fade[c].out, fade[c+1].out, fade[c+2].out, fade[c+3].out));
          remainderGain = 1.f - swapGain;
          aOut[c0] = aIn*remainderGain + bIn*swapGain;
          bOut[c0] = bIn*remainderGain + aIn*swapGain;
          if (oversample>1) {
            aOut[c0] = aDownSample[c0].process(aOut[c0]);
            bOut[c0] = bDownSample[c0].process(bOut[c0]);
          }
        }
      }
    }
    for (int c=0; c<xChannels; c+=4) {
      outputs[A_OUTPUT].setVoltageSimd(aOut[c/4], c);
      outputs[B_OUTPUT].setVoltageSimd(bOut[c/4], c);
    }
    outputs[A_OUTPUT].setChannels(xChannels);
    outputs[B_OUTPUT].setChannels(xChannels);
  }


  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "monitorChannel", json_integer(lightChannel));
    json_object_set_new(rootJ, "inputPolyControl", json_boolean(inputPolyControl));
    json_object_set_new(rootJ, "audioProc", json_integer(audioProc));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "monitorChannel")))
      lightChannel = json_integer_value(val);
    if ((val = json_object_get(rootJ, "inputPolyControl")))
      inputPolyControl = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "audioProc")))
      audioProc = json_integer_value(val);
  }

};

struct BernoulliSwitchWidget : VenomWidget {

  struct NormalSwitch : GlowingSvgSwitchLockable {
    NormalSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  BernoulliSwitchWidget(BernoulliSwitch* module) {
    setModule(module);
    setVenomPanel("BernoulliSwitch");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(5.0, 18.75)), module, BernoulliSwitch::NO_SWAP_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(20.431, 18.75)), module, BernoulliSwitch::SWAP_LIGHT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(12.7155, 18.75)), module, BernoulliSwitch::PROB_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLockable<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.5, 31.5)), module, BernoulliSwitch::TRIG_PARAM, BernoulliSwitch::TRIG_LIGHT));
    addParam(createLockableParam<CKSSThreeLockable>(mm2px(Vec(17.5, 25.0)), module, BernoulliSwitch::MODE_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(7.297, 43.87)), module, BernoulliSwitch::RISE_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.134, 43.87)), module, BernoulliSwitch::FALL_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(7.297, 58.3)), module, BernoulliSwitch::OFFSET_A_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.136, 58.3)), module, BernoulliSwitch::OFFSET_B_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(7.297, 72.75)), module, BernoulliSwitch::SCALE_A_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.136, 72.75)), module, BernoulliSwitch::SCALE_B_PARAM));

    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.297, 87.10)), module, BernoulliSwitch::A_INPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(18.134, 87.10)), module, BernoulliSwitch::B_INPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(7.297, 101.55)), module, BernoulliSwitch::A_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(18.134, 101.55)), module, BernoulliSwitch::B_OUTPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.297, 116.0)), module, BernoulliSwitch::TRIG_INPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(18.134, 116.0)), module, BernoulliSwitch::PROB_INPUT));
    addParam(createLockableParamCentered<NormalSwitch>(Vec(5.1615,325.3265), module, BernoulliSwitch::NORMAL_PARAM));

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(12.7155, 83.9)), module, BernoulliSwitch::POLY_SENSE_ALL_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<RedBlueLight<>>>(mm2px(Vec(12.7155, 98.35)), module, BernoulliSwitch::AUDIO_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    BernoulliSwitch* module = dynamic_cast<BernoulliSwitch*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexPtrSubmenuItem(
      "Audio process",
      {"Off","Antipop crossfade","oversample x2","oversample x4","oversample x8","oversample x16"},
      &module->audioProc
    ));
    menu->addChild(createIndexPtrSubmenuItem(
      "Polyphony control",
      {"Trig and Prob only", "All inputs"},
      &module->inputPolyControl
    ));
    menu->addChild(createIndexSubmenuItem(
      "Monitor channel",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","Off"},
      [=]() {return module->lightChannel;},
      [=](int i) {
        module->lightChannel = i;
        module->lights[BernoulliSwitch::NO_SWAP_LIGHT].setBrightness(i > module->oldChannels ? false : !module->swap[i]);
        module->lights[BernoulliSwitch::SWAP_LIGHT].setBrightness(i > module->oldChannels ? false : module->swap[i]);
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelBernoulliSwitch = createModel<BernoulliSwitch, BernoulliSwitchWidget>("BernoulliSwitch");
