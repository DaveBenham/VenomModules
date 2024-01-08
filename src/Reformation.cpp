// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
#include <float.h>

#define MAP_COUNT 5

std::string mapLabel[MAP_COUNT] = {"Min", "1/4", "1/2", "3/4", "Max"};
float mapDefault[MAP_COUNT] = {0.f, 0.25f, 0.5f, 0.75f, 1.f};
float rangeMin[MAP_COUNT-1] = {-FLT_MAX, 0.25f, 0.5f, 0.75f};
float rangeMax[MAP_COUNT-1] = {0.25f, 0.5f, 0.75f, FLT_MAX};
  
struct Reformation : VenomModule {
  enum ParamId {
    DRIVE_PARAM,
    LEVEL_PARAM,
    IN_PARAM,
    OUT_PARAM,
    CLIP_PARAM,
    OVER_PARAM,
    ENUMS(MAP_PARAM,MAP_COUNT),
    ENUMS(CV1_PARAM,MAP_COUNT),
    ENUMS(CV2_PARAM,MAP_COUNT),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(CV1_INPUT,MAP_COUNT),
    ENUMS(CV2_INPUT,MAP_COUNT),
    IN_INPUT,
    DRIVE_INPUT,
    LEVEL_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(MAP_LIGHT,MAP_COUNT),
    LIGHTS_LEN
  };

  float oldOversample = -1.f; // force initialization
  int oversample, oversampleValues[5] = {1,4,8};
  OversampleFilter_4 cv1UpSample[4][MAP_COUNT], cv2UpSample[4][MAP_COUNT],
                     inUpSample[4], driveUpSample[4], levelUpSample[4],
                     outDownSample[4];

  Reformation() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(DRIVE_PARAM, 1.f, 10.f, 2.f, "Drive");
    configParam(LEVEL_PARAM, 0.f, 1.f, 0.5f, "Level");
    configSwitch<FixedSwitchQuantity>(IN_PARAM, 0.f, 1.f, 0.f, "Input polarity", {"Unipolar", "Bipolar"});
    configSwitch<FixedSwitchQuantity>(OUT_PARAM, 0.f, 1.f, 0.f, "Output polarity", {"Unipolar", "Bipolar"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 2.f, 1.f, "Clipping", {"Off", "Hard CV clipping", "Soft audio clipping"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 2.f, 0.f, "Oversampling", {"Off", "x4", "x8"});
    for (int i=0; i<MAP_COUNT; i++) {
      configParam(MAP_PARAM+i, 0.f, 1.f, mapDefault[i], mapLabel[i] + " way point");
      lights[MAP_LIGHT+i].setBrightness(1.f);
      configParam(CV1_PARAM+i, -1.f, 1.f, 0.f, mapLabel[i] + " CV1");
      configInput(CV1_INPUT+i, mapLabel[i] + " CV1");
      configParam(CV2_PARAM+i, -1.f, 1.f, 0.f, mapLabel[i] + " CV2");
      configInput(CV2_INPUT+i, mapLabel[i] + " CV2");
    }  
    configInput(IN_INPUT, "Signal");
    configInput(DRIVE_INPUT, "Drive");
    configInput(LEVEL_INPUT, "Level");
    configOutput(OUT_OUTPUT, "Signal");
    configBypass(IN_INPUT, OUT_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    float inOffset = params[IN_PARAM].getValue() == 0.f ? 0.f : 5.f;
    float outOffset = params[OUT_PARAM].getValue() == 0.f ? 5.f : 0.f;
    using float_4 = simd::float_4;
    float_4 cv1[4][MAP_COUNT], cv2[4][MAP_COUNT], map[4][MAP_COUNT], in[4], drive[4], level[4], out[4];
    int clip = static_cast<int>(params[CLIP_PARAM].getValue());

    // configure oversample
    if (params[OVER_PARAM].getValue() != oldOversample) {
      oldOversample = params[OVER_PARAM].getValue();
      oversample = oversampleValues[static_cast<int>(oldOversample)];
      for (int i=0; i<MAP_COUNT; i++){
        for (int j=0; j<4; j++){
          cv1UpSample[j][i].setOversample(oversample);
          cv2UpSample[j][i].setOversample(oversample);
          if (i==0){
            inUpSample[j].setOversample(oversample);
            driveUpSample[j].setOversample(oversample);
            levelUpSample[j].setOversample(oversample);
            outDownSample[j].setOversample(oversample);
          }
        }
      }
    }
    
    // get channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c>channels)
        channels = c;
    }
    int simdCnt = (channels+3)/4;

    for (int o=0; o<oversample; o++){
      for (int s=0, c=0; s<simdCnt; s++, c+=4){

        // Get input
        if (s==0 || inputs[IN_INPUT].isPolyphonic()) {
          in[s] = o ? float_4::zero() : (inputs[IN_INPUT].getPolyVoltageSimd<float_4>(c) + inOffset)/10.f;
          if (oversample>1){
            if (o==0) in[s] *= oversample;
            in[s] = inUpSample[s].process(in[s]);
          }
        } else in[s] = in[0];

        // Get maps
        for (int m=0; m<MAP_COUNT; m++){
          // Get CV1
          if (s==0 || inputs[CV1_INPUT+m].isPolyphonic()){
            if (inputs[CV1_INPUT+m].isConnected()){
              cv1[s][m] = o ? float_4::zero() : inputs[CV1_INPUT+m].getPolyVoltageSimd<float_4>(c)/10.f * params[CV1_PARAM+m].getValue();
              if (oversample>1){
                if (o==0) cv1[s][m] *= oversample;
                cv1[s][m] = cv1UpSample[s][m].process(cv1[s][m]);
              }
            }
            else cv1[s][m] = float_4::zero();
          }
          else cv1[s][m] = cv1[0][m];
          // Get CV2
          if (s==0 || inputs[CV2_INPUT+m].isPolyphonic()){
            if (inputs[CV2_INPUT+m].isConnected()){
              cv2[s][m] = o ? float_4::zero() : inputs[CV2_INPUT+m].getPolyVoltageSimd<float_4>(c)/10.f * params[CV2_PARAM+m].getValue();
              if (oversample>1){
                if (o==0) cv2[s][m] *= oversample;
                cv2[s][m] = cv2UpSample[s][m].process(cv2[s][m]);
              }
            }
            else cv2[s][m] = float_4::zero();
          }
          else cv2[s][m] = cv2[0][m];
          // compute final map element
          map[s][m] = params[MAP_PARAM+m].getValue() + cv1[s][m] + cv2[s][m];
        }

        // Apply map
        out[s] = float_4::zero();
        for (int i=0, j=1; j<MAP_COUNT; i++, j++)
          out[s] = simd::ifelse(simd::ifelse(in[s] > rangeMin[i], in[s] <= rangeMax[i], float_4::zero()), (in[s] - mapDefault[i])/0.25f * (map[s][j] - map[s][i]) + map[s][i], out[s]);

        // Convert to bipolar +/- 5V
        out[s] = out[s]*10.f - 5.f;
        
        // Get and apply drive
        if (s==0 || inputs[DRIVE_INPUT].isPolyphonic()) {
          if (inputs[DRIVE_INPUT].isConnected()){
            drive[s] = o ? float_4::zero() : clamp(inputs[DRIVE_INPUT].getPolyVoltageSimd<float_4>(c) + params[DRIVE_PARAM].getValue(), 1.f, 10.f);
            if (oversample>1){
              if (o==0) drive[s] *= oversample;
              drive[s] = driveUpSample[s].process(drive[s]);
            }
          }
          else drive[s] = params[DRIVE_PARAM].getValue();
        }
        else drive[s] = drive[0];
        out[s] *= drive[s];
        
        // Apply clipping
        if (clip==1) out[s] = clamp(out[s],-10.f,10.f);
        if (clip==2) out[s] = softClip(out[s]);
        
        // Get and apply level
        if (s==0 || inputs[LEVEL_INPUT].isPolyphonic()) {
          if (inputs[LEVEL_INPUT].isConnected()){
            level[s] = o ? float_4::zero() : clamp(inputs[LEVEL_INPUT].getPolyVoltageSimd<float_4>(c)/10.f) * params[LEVEL_PARAM].getValue();
            if (oversample>1){
              if (o==0) level[s] *= oversample;
              level[s] = levelUpSample[s].process(level[s]);
            }
          }
          else level[s] = params[LEVEL_PARAM].getValue();
        }
        else level[s] = level[0];
        out[s] *= level[s];
        
        // Downsample
        if (oversample>1) out[s] = outDownSample[s].process(out[s]);
      }
    }
    
    // write output with correct polarity and set channel count
    for (int s=0, c=0; s<simdCnt; s++, c+=4)
      outputs[OUT_OUTPUT].setVoltageSimd(out[s]+outOffset, c);
    outputs[OUT_OUTPUT].setChannels(channels);
  }

};

struct ReformationWidget : VenomWidget {

  struct PortSwitch : GlowingSvgSwitchLockable {
    PortSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  ReformationWidget(Reformation* module) {
    setModule(module);
    setVenomPanel("Reformation");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(61.f,58.f), module, Reformation::DRIVE_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(104.f,58.f), module, Reformation::LEVEL_PARAM));
    addParam(createLockableParamCentered<PortSwitch>(Vec(23.78f,47.78f), module, Reformation::IN_PARAM));
    addParam(createLockableParamCentered<PortSwitch>(Vec(141.22f,47.78f), module, Reformation::OUT_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(23.78f,66.78f), module, Reformation::CLIP_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(141.22f,66.78f), module, Reformation::OVER_PARAM));
    float x = 22.5f;
    for (int i=0; i<MAP_COUNT; i++, x+=30.f){
      addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(x,133.5f), module, Reformation::MAP_PARAM+i, Reformation::MAP_LIGHT+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(x, 192.f), module, Reformation::CV1_PARAM+i));
      addInput(createInputCentered<PolyPJ301MPort>(Vec(x,224.5f), module, Reformation::CV1_INPUT+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(x, 257.f), module, Reformation::CV2_PARAM+i));
      addInput(createInputCentered<PolyPJ301MPort>(Vec(x,289.5f), module, Reformation::CV2_INPUT+i));
    }
    addInput(createInputCentered<PolyPJ301MPort>(Vec(33.f,340.5f), module, Reformation::IN_INPUT));
    addInput(createInputCentered<PolyPJ301MPort>(Vec(66.f,340.5f), module, Reformation::DRIVE_INPUT));
    addInput(createInputCentered<PolyPJ301MPort>(Vec(99.f,340.5f), module, Reformation::LEVEL_INPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(132.f,340.5f), module, Reformation::OUT_OUTPUT));
  }

};

Model* modelReformation = createModel<Reformation, ReformationWidget>("Reformation");
