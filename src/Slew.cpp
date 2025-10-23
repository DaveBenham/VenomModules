// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"

namespace Venom {

struct Slew : VenomModule { 
  enum ParamId {
    SPEED_PARAM,
    OVER_PARAM,
    RISE_TIME_PARAM,
    FALL_TIME_PARAM,
    RISE_TIME_CV_PARAM,
    FALL_TIME_CV_PARAM,
    RISE_SHAPE_PARAM,
    FALL_SHAPE_PARAM,
    RISE_SHAPE_CV_PARAM,
    FALL_SHAPE_CV_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RISE_TIME_CV_INPUT,
    FALL_TIME_CV_INPUT,
    RISE_SHAPE_CV_INPUT,
    FALL_SHAPE_CV_INPUT,
    RAW_INPUT,
    VOCT_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    RISE_OUTPUT,
    FALL_OUTPUT,
    FLAT_OUTPUT,
    SLEW_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
 
  using float_4 = simd::float_4;

  int oversample=0;
  int oversampleValues[6]{1,2,4,8,16,32};
  OversampleFilter_4 upSample[6][4]{}, downSample[4][4]{};
  float_4 oldOut[4]{};
  

  Slew() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SPEED_PARAM, 0.f, 1.f, 0.f, "Speed", {"Slow", "Fast"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(RISE_TIME_PARAM, -5.f, 5.f, 0.f, "Rise time 10V", " msec", 2.0f, 0.25f, 0.f);
    configParam(FALL_TIME_PARAM, -5.f, 5.f, 0.f, "Fall time 10V", " msec", 2.0f, 0.25f, 0.f);

    configParam(RISE_TIME_CV_PARAM, -1.f, 1.f, 0.f, "Rise time CV amount", "%", 0, 100, 0);
    configParam(FALL_TIME_CV_PARAM, -1.f, 1.f, 0.f, "Fall time CV amount", "%", 0, 100, 0);
    
    configInput(RISE_TIME_CV_INPUT, "Rise time CV");
    configInput(FALL_TIME_CV_INPUT, "Fall time CV");
    
    configParam(RISE_SHAPE_PARAM, 0.f, 1.f, 0.f, "Rise shape (curve amount)", "%", 0, 100, 0);
    configParam(FALL_SHAPE_PARAM, 0.f, 1.f, 0.f, "Fall shape (curve amount)", "%", 0, 100, 0);
    
    configParam(RISE_SHAPE_CV_PARAM, -1.f, 1.f, 0.f, "Rise shape CV amount", "%", 0, 100, 0);
    configParam(FALL_SHAPE_CV_PARAM, -1.f, 1.f, 0.f, "Fall shape CV amount", "%", 0, 100, 0);
    
    configInput(RISE_SHAPE_CV_INPUT, "Rise time CV");
    configInput(FALL_SHAPE_CV_INPUT, "Fall time CV");

    configInput(RAW_INPUT, "Raw");
    configInput(VOCT_INPUT, "V/Oct");

    configOutput(RISE_OUTPUT, "Rise gate");
    configOutput(FALL_OUTPUT, "Fall gate");
    configOutput(FLAT_OUTPUT, "Flat gate");
    configOutput(SLEW_OUTPUT, "Slew");

    configBypass(RAW_INPUT, SLEW_OUTPUT);

    oversampleStages = 5;
  }

  void setOversample() override {
    for (int s=0; s<4; s++){
      for (int i=0; i<INPUTS_LEN; i++){
        upSample[i][s].setOversample(oversample, oversampleStages);
      }
      for (int i=0; i<OUTPUTS_LEN; i++){
        downSample[i][s].setOversample(oversample, oversampleStages);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    // update oversample configuration
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
    }
    // configure time knobs
    bool fast = params[SPEED_PARAM].getValue();
    float msecScale = 1000.f/(fast ? 523.26f : 4.f);
    paramQuantities[RISE_TIME_PARAM]->displayMultiplier = msecScale;
    paramQuantities[FALL_TIME_PARAM]->displayMultiplier = msecScale;
    // speed dependent slew constants    
    float kLin = 10*(fast ? 523.26f : 4.f)/args.sampleRate/oversample;
    float kCurve = fast ? 30.f : 4000.f;
    // get channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max({channels, inputs[i].getChannels()});
    float_4 in[INPUTS_LEN]{}, out[OUTPUTS_LEN]{};
    // channel loop
    for (int s=0, c=0; c<channels; s++, c+=4){
      // oversample loop
      for (int o=0; o<oversample; o++) {
        // read inputs
        if (!o) {
          for (int i=0; i<INPUTS_LEN; i++)
            in[i] = inputs[i].getPolyVoltageSimd<float_4>(c);
        }
        // upsample inputs
        if (oversample > 1){
          for (int i=0; i<INPUTS_LEN; i++) {
            if (inputs[i].isConnected())
              in[i] = upSample[i][s].process(o ? float_4::zero() : in[i]*oversample);
          }
        }
        // compute outputs
        float_4 riseMult = pow(2.f, in[VOCT_INPUT] - params[RISE_TIME_PARAM].getValue() - in[RISE_TIME_CV_INPUT]*params[RISE_TIME_CV_PARAM].getValue());
        float_4 fallMult = pow(2.f, in[VOCT_INPUT] - params[FALL_TIME_PARAM].getValue() - in[FALL_TIME_CV_INPUT]*params[FALL_TIME_CV_PARAM].getValue());
        float_4 diff = in[RAW_INPUT] - oldOut[s];
        out[RISE_OUTPUT] = ifelse(diff>1e-6f, 10.f, 0.f);
        out[FALL_OUTPUT] = ifelse(diff<-1e-6f, 10.f, 0.f);
        out[FLAT_OUTPUT] = ifelse(out[RISE_OUTPUT]+out[FALL_OUTPUT]==float_4::zero(), 10.f, 0.f);
        float_4 lin = oldOut[s] + ifelse(diff>float_4::zero(), fmin(diff, kLin*riseMult), -fmin(-diff, kLin*fallMult));
        float_4 curve = clamp(oldOut[s] + diff * 48000.f * ifelse(diff>float_4::zero(), riseMult, fallMult) / kCurve / args.sampleRate / oversample, -20.f, 20.f);
        float_4 curveAmt = clamp(ifelse( diff>float_4::zero(), 
                                         params[RISE_SHAPE_PARAM].getValue() + in[RISE_SHAPE_CV_INPUT]*params[RISE_SHAPE_CV_PARAM].getValue()/10.f,
                                         params[FALL_SHAPE_PARAM].getValue() + in[FALL_SHAPE_CV_INPUT]*params[FALL_SHAPE_CV_PARAM].getValue()/10.f ));
        out[SLEW_OUTPUT] = curve*curveAmt + lin*(1-curveAmt);
        //save old slew value
        oldOut[s] = out[SLEW_OUTPUT];
        // downsample output
        if (oversample > 1) {
          for (int i=0; i<OUTPUTS_LEN; i++) {
            if (outputs[i].isConnected())
              out[i] = downSample[i][s].process(out[i]);
          }
        }
      } // end oversample loop
      // write output
      for (int i=0; i<OUTPUTS_LEN; i++)
        outputs[i].setVoltageSimd(out[i], c);
    } // end channel loop
    // set output channel count
    for (int i=0; i<OUTPUTS_LEN; i++)
      outputs[i].setChannels(channels);
    
  }

};

struct SlewWidget : VenomWidget {

  struct SpeedSwitch : GlowingSvgSwitchLockable {
    SpeedSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
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

  SlewWidget(Slew* module) {
    setModule(module);
    setVenomPanel("Slew");

    addParam(createLockableParamCentered<SpeedSwitch>(Vec(28.f, 26.5f), module, Slew::SPEED_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(65.f, 26.5f), module, Slew::OVER_PARAM));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(20.f, 60.5f), module, Slew::RISE_TIME_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.f, 60.5f), module, Slew::FALL_TIME_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(20.f, 96.5f), module, Slew::RISE_TIME_CV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(55.f, 96.5f), module, Slew::FALL_TIME_CV_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.f, 130.f), module, Slew::RISE_TIME_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(55.f, 130.f), module, Slew::FALL_TIME_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(20.f, 163.f), module, Slew::RISE_SHAPE_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(55.f, 163.f), module, Slew::FALL_SHAPE_PARAM));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(20.f, 194.f), module, Slew::RISE_SHAPE_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.f, 194.f), module, Slew::FALL_SHAPE_CV_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.f, 225.5f), module, Slew::RISE_SHAPE_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(55.f, 225.5f), module, Slew::FALL_SHAPE_CV_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(20.f, 264.5), module, Slew::RAW_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(55.f, 264.5), module, Slew::VOCT_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(20.f, 304.5), module, Slew::RISE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.f, 304.5), module, Slew::FALL_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(20.f, 341.5), module, Slew::FLAT_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.f, 341.5), module, Slew::SLEW_OUTPUT));
  }

};

}

Model* modelVenomSlew = createModel<Venom::Slew, Venom::SlewWidget>("Slew");
