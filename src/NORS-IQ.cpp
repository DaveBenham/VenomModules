// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include <float.h>
#include <string.h>

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f
#define INTVL_CNT 13
#define RATIO_UNIT HZ_UNIT

struct NORS_IQ : VenomModule {
  
  enum ParamId {
    INTVL_UNIT_PARAM,
    POI_PARAM,
    EDPO_PARAM,
    LENGTH_PARAM,
    ROOT_PARAM,
    ROOT_UNIT_PARAM,
    ROUND_PARAM,
    EQUI_PARAM,
    ENUMS(INTVL_PARAM,INTVL_CNT),
    EQUAL_DIVS_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(INTVL_INPUT,INTVL_CNT),
    POI_INPUT,
    EDPO_INPUT,
    LENGTH_INPUT,
    ROOT_INPUT,
    IN_INPUT,
    TRIG_INPUT,
    POLY_INTVL_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT_OUTPUT,
    TRIG_OUTPUT,
    SCALE_OUTPUT,
    POCT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    EQUI_LIGHT,
    EQUAL_DIVS_LIGHT,
    LIGHTS_LEN
  };
  
  enum UnitId {
    VOLT_UNIT,
    CENT_UNIT,
    HZ_UNIT
  };
  
  enum RoundId {
    ROUND_DOWN,
    ROUND_NEAR,
    ROUND_UP
  };

  dsp::SchmittTrigger trigIn[16];
  dsp::PulseGenerator trigOut[16];
  int channelNote[16]{};
  int channelOct[16]{};
  int oldTrigChannels = 0;
  int oldChannels = 0;
  float oldOut[16]{};

  bool equalDivs = true;
  float poi = 1;
  int edpo = 12;
  int len = 0;
  float root = 0;
  int intvl[INTVL_CNT]{};
  float step[INTVL_CNT]{};
  
  std::string intvlStr(float val, bool display) {
    constexpr size_t sz = 32;
    char buf[sz];
    switch (static_cast<int>(params[INTVL_UNIT_PARAM].getValue())) {
      case RATIO_UNIT:
        snprintf(buf, sz, display ? "%g:1" : "%g", pow(2.f, val));
        break;
      case CENT_UNIT:
        snprintf(buf, sz, display ? "%g \u00A2" : "%g", val*1200.f);
        break;
      case VOLT_UNIT:
      default:
        snprintf(buf, sz, display ? "%g V" : "%g", val);
        break;
    }
    return std::string(buf, sz);
  }  

  struct POIQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      return dynamic_cast<NORS_IQ*>(module)->intvlStr(getValue(), false);
    }
    void setDisplayValue(float v) override {
      if (module->params[INTVL_UNIT_PARAM].getValue() == RATIO_UNIT)
        setValue(log2(v));
      else
        setValue( module->params[INTVL_UNIT_PARAM].getValue() == CENT_UNIT ? v/1200.f : v);
    }
  };
  
  struct IntervalQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      NORS_IQ* mod = dynamic_cast<NORS_IQ*>(module);
      if (mod->equalDivs)
        return std::to_string(static_cast<int>(std::round(getValue()*99+1)));
      return mod->intvlStr(getValue()*2.f, false);
    }
    void setDisplayValue(float v) override {
      NORS_IQ* mod = dynamic_cast<NORS_IQ*>(module);
      if (mod->equalDivs)
        setValue((v-1.f)/99.f);
      else if (mod->params[INTVL_UNIT_PARAM].getValue() == RATIO_UNIT)
        setValue(log2(v)/2.f);
      else
        setValue( (mod->params[INTVL_UNIT_PARAM].getValue() == CENT_UNIT ? v/1200.f : v) / 2.f );
    }
  };
  
  std::string rootStr(float val, bool display) {
    constexpr size_t sz = 32;
    char buf[sz];
    switch (static_cast<int>(params[ROOT_UNIT_PARAM].getValue())) {
      case HZ_UNIT:
        snprintf(buf, sz, display ? "%g Hz" : "%g", pow(2.f, val + log2(dsp::FREQ_C4)));
        break;
      case CENT_UNIT:
        snprintf(buf, sz, display ? "%g \u00A2" : "%g", val*1200.f);
        break;
      case VOLT_UNIT:
      default:
        snprintf(buf, sz, display ? "%g V" : "%g", val);
    }
    return std::string(buf, sz);
  };

  struct RootQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      return dynamic_cast<NORS_IQ*>(module)->rootStr(getValue(), false);
    }
    void setDisplayValue(float v) override {
      if (module->params[ROOT_UNIT_PARAM].getValue() == HZ_UNIT)
        setValue(log2(v) - log2(dsp::FREQ_C4));
      else
        setValue( module->params[ROOT_UNIT_PARAM].getValue() == CENT_UNIT ? v/1200.f : v);
    }
  };
  
  void setIntervalUnit() {
    std::string unit;
    switch (static_cast<int>(params[INTVL_UNIT_PARAM].getValue())) {
      case RATIO_UNIT:
        unit = ":1";
        break;
      case CENT_UNIT:
        unit = " \u00A2";
        break;
      case VOLT_UNIT:
        unit = " V";
        break;
    }  
    paramQuantities[POI_PARAM]->unit = unit;
    if (equalDivs)
      unit = "";
    for (int i=0; i<INTVL_CNT; i++)
      paramQuantities[INTVL_PARAM+i]->unit = unit;
  }
  
  void setRootUnit() {
    ParamQuantity* pq = paramQuantities[ROOT_PARAM];
    switch (static_cast<int>(params[ROOT_UNIT_PARAM].getValue())) {
      case HZ_UNIT:
        pq->unit = " Hz";
        break;
      case CENT_UNIT:
        pq->unit = " \u00A2";
        break;
      case VOLT_UNIT:
        pq->unit = " V";
        break;
    }  
  }

  NORS_IQ() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(INTVL_UNIT_PARAM, 0, 2, 1, "Interval unit", {"Volts", "Cents", "Ratio"});
    configSwitch<FixedSwitchQuantity>(EQUAL_DIVS_PARAM, 0.f, 1.f, 1.f, "Equal Interval Divisions", {"Off", "On"});
    configParam<POIQuantity>(POI_PARAM, 0.f, 4.f, 1.f, "Pseudo-octave interval", " V");
    configParam(EDPO_PARAM, 1.f, 100.f, 12.f, "Equal divisions per pseudo-octave");
    configParam(LENGTH_PARAM, 1.f, INTVL_CNT, 12.f, "Scale length");
    configParam<RootQuantity>(ROOT_PARAM, -4.f, 4.f, 0.f, "Scale root", " V");
    configSwitch<FixedSwitchQuantity>(ROOT_UNIT_PARAM, 0, 2, 2, "Scale root unit", {"Volts", "Cents", "Hertz"});
    configSwitch<FixedSwitchQuantity>(ROUND_PARAM, 0, 2, 1, "Round algorithm", {"Down", "Nearest", "Up"});
    configSwitch<FixedSwitchQuantity>(EQUI_PARAM, 0.f, 1.f, 0.f, "Equi-likely", {"Off", "On"});
    for (int i=0; i<INTVL_CNT; i++) {
      std::string name = "Interval " + std::to_string(i+1);
      configParam<IntervalQuantity>(INTVL_PARAM+i, 0, 1, 0, name);
      configInput(INTVL_INPUT+i, name + " CV");
    }
    configInput(POLY_INTVL_INPUT, "Polyphonic intervals");
    configInput(POI_INPUT, "Pseudo-octave interval CV");
    configInput(EDPO_INPUT, "Equal divisions per pseudo-octave CV");
    configInput(LENGTH_INPUT, "Scale length CV");
    configInput(ROOT_INPUT, "Scale root CV");
    configInput(IN_INPUT, "V/Oct");
    configInput(TRIG_INPUT, "Trigger");
    configOutput(OUT_OUTPUT, "V/Oct");
    configOutput(TRIG_OUTPUT, "Trigger");
    configOutput(SCALE_OUTPUT, "Scale");
    configOutput(POCT_OUTPUT, "Pseudo-octave");
    configBypass(TRIG_INPUT, TRIG_OUTPUT);
    configBypass(IN_INPUT, OUT_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if ((params[EQUAL_DIVS_PARAM].getValue() != 0.f) != equalDivs) {
      equalDivs = !equalDivs;
      setIntervalUnit();
    }
    poi = clamp(params[POI_PARAM].getValue() + inputs[POI_INPUT].getVoltage(), 0.f, 4.f);
    edpo = clamp(params[EDPO_PARAM].getValue() + std::round(inputs[EDPO_INPUT].getVoltage()*10.f), 1.f, 100.f);
    float minIntvl = poi / edpo;
    root = clamp(params[ROOT_PARAM].getValue() + inputs[ROOT_INPUT].getVoltage(), -4.f, 4.f);
    len = clamp(params[LENGTH_PARAM].getValue() + std::round(inputs[LENGTH_INPUT].getVoltage()*2.f), 1.f, 13.f);
    float scale = 0.f;
    int round = params[ROUND_PARAM].getValue();
    bool equi = params[EQUI_PARAM].getValue();
    for (int i=0; i<len; i++){
      if (equalDivs) {
        intvl[i] = clamp(std::round(params[INTVL_PARAM+i].getValue()*99+1) + std::round((inputs[INTVL_INPUT+i].getVoltage()+inputs[POLY_INTVL_INPUT].getVoltage(i))*10.f), 1.f, 100.f);
        step[i] = minIntvl * intvl[i];
      }
      else {
        step[i] = clamp(params[INTVL_PARAM+i].getValue()*2 + inputs[INTVL_INPUT+i].getVoltage() + inputs[POLY_INTVL_INPUT].getVoltage(i), 0.f, 2.f);
      }
      scale += step[i];
      outputs[SCALE_OUTPUT].setVoltage(scale+root, i+1);
    }
    outputs[SCALE_OUTPUT].setVoltage(root, 0);
    outputs[SCALE_OUTPUT].setChannels(len+1);
    int trigChannels = inputs[TRIG_INPUT].getChannels();
    if (trigChannels < oldTrigChannels) {
      for (int c=trigChannels; c<oldTrigChannels; c++)
        trigIn[c].process(0.f);
      oldTrigChannels = trigChannels;
    }
    int channels = std::max({1, trigChannels, inputs[IN_INPUT].getChannels()});
    if (channels < oldChannels) {
      for (int c=channels; c<oldChannels; c++)
        trigOut[c].reset();
    }
    oldChannels = channels;
    for (int c=0; c<channels; c++) {
      if (trigIn[c].process(inputs[TRIG_INPUT].getPolyVoltage(c), 0.1f, 1.f) || !inputs[TRIG_INPUT].isConnected()) {
        float in = inputs[IN_INPUT].getPolyVoltage(c);
        int oct = (in - root) / scale;
        float out = scale * oct + root;
        if (in < out) {
          out -= scale;
          oct--;
        }
        float test = out;
        float testStep = scale / len;
        channelNote[c] = 0;
        for (int i=0; i<len; i++) {
          if (!equi)
            testStep = step[i];
          if (round == ROUND_NEAR && in < test + testStep/2)
            break;
          if (round == ROUND_DOWN && in < test + testStep)
            break;
          if (round == ROUND_UP && in <= test)
            break;
          test += testStep;
          out = equi ? out + step[i] : test;
          channelNote[c]++;
        }
        if (channelNote[c]==len) {
          channelNote[c]=0;
          oct++;
        }
        channelOct[c] = oct;
        if (!isNear(oldOut[c],out)) {
          oldOut[c] = out;
          if (!inputs[TRIG_INPUT].isConnected())
            trigOut[c].trigger();
        }
        outputs[OUT_OUTPUT].setVoltage(out, c);
        outputs[POCT_OUTPUT].setVoltage(oct, c);
      }
      trigOut[c].process(args.sampleTime);
      outputs[TRIG_OUTPUT].setVoltage( inputs[TRIG_INPUT].isConnected() ? inputs[TRIG_INPUT].getPolyVoltage(c) : (trigOut[c].remaining > 0.f ? 10.f : 0.f), c);
    }
    outputs[OUT_OUTPUT].setChannels(channels);
    outputs[POCT_OUTPUT].setChannels(channels);
    outputs[TRIG_OUTPUT].setChannels(channels);
  }

};


struct NORS_IQDisplay : LedDisplay {
  NORS_IQ* module = NULL;
  ModuleWidget* moduleWidget = NULL;
  std::string fontPath;
  
  NORS_IQDisplay() {
    fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  }

  void drawLine(const DrawArgs& args, float x0, float y0, float x1, float y1) {
    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, x0, y0);
    nvgLineTo(args.vg, x1, y1);
    nvgStroke(args.vg);
  }
  
  void drawNote(const DrawArgs& args, float x0, float y0, int oct, std::shared_ptr<Font> font) {
    if (oct < 0) {
      nvgFillColor(args.vg, SCHEME_RED);
      oct=-oct;
    }
    else
      nvgFillColor(args.vg, SCHEME_YELLOW);

    float xDelta = 2.5f;
    float yDelta = 3.8f;
    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, x0-xDelta, y0+yDelta);
    nvgLineTo(args.vg, x0-xDelta, y0-yDelta);
    nvgLineTo(args.vg, x0+xDelta, y0-yDelta);
    nvgLineTo(args.vg, x0+xDelta, y0+yDelta);
    nvgClosePath(args.vg);

    char chr;
    if (oct < 10)
      chr = '0' + oct;
    else if (oct < 36)
      chr = 'A' + oct - 10;
    else if (oct < 62)
      chr = 'a' + oct - 36;
    else
      chr = ' ';
    std::string txt = string::f("%c", chr);
    nvgFill(args.vg);
    nvgFontSize(args.vg, 9);
    nvgFillColor(args.vg, SCHEME_BLACK);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
    nvgText(args.vg, x0, y0, txt.c_str(), NULL);
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer != 1)
      return;

    // draw lines
    nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xd7, 0x14, 0x40));
    drawLine(args, 2.f,21.f, 390.233f,21.f);
    drawLine(args, 2.f,64.f, 390.233f,64.f);
    drawLine(args, 177.2f,2.6f, 177.2f,21.f);
    drawLine(args, 213.f,2.6f, 213.f,21.f);
    float x = 30.3f;
    for (int i=0; i<INTVL_CNT-1; i++) {
      drawLine(args, x,21.f, x,82.247);
      x += 30.f;
    }
    
    // draw text
    int dummyIntvl[5] {3,2,2,3,2};
    std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
    if (!font)
      return;
    nvgFontSize(args.vg, 13);
    nvgFontFaceId(args.vg, font->handle);
    nvgFillColor(args.vg, SCHEME_YELLOW);
    std::string txt;

    if (!module || (module && module->equalDivs)) {
      txt = module ? module->intvlStr(module->poi, true) + string::f(" / %d", module->edpo) : "1200 \u00A2 / 12";
      nvgTextAlign(args.vg, NVG_ALIGN_RIGHT + NVG_ALIGN_TOP);
      nvgText(args.vg, 171.f, 6.f, txt.c_str(), NULL);
    }

    txt = module ? string::f("%d", module->len) : "5";
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
    nvgText(args.vg, 195.1f, 6.f, txt.c_str(), NULL);

    txt = module ? module->rootStr(module->root, true) : "261.626 Hz";
    nvgTextAlign(args.vg, NVG_ALIGN_LEFT + NVG_ALIGN_TOP);
    nvgText(args.vg, 221.f, 6.f, txt.c_str(), NULL);
    
    x = 15.3f;
    if (!module || (module && module->equalDivs)) {
      nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_BOTTOM);
      for (int i=0; i<(module ? module->len : 5); i++) {
        txt = string::f("%d", module ? module->intvl[i] : dummyIntvl[i]);
        nvgText(args.vg, x, 81.f, txt.c_str(), NULL);
        x += 30.f;
      }
    } else {
      std::string txt2;
      nvgFontSize(args.vg, 8);
      nvgTextLetterSpacing(args.vg, -1);
      for (int i=0; i<module->len; i++) {
        txt = module->intvlStr(module->step[i], true);
        txt2 = txt.substr(0,txt.length()-2);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_TOP);
        nvgText(args.vg, x, 66.f, txt2.c_str(), NULL);
        txt2 = txt.substr(txt.length()-2,2);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_BOTTOM);
        nvgText(args.vg, x, 82.f, txt2.c_str(), NULL);
        x += 30.f;
      }
    }
    
    // draw quantized notes
    float x0 = 6.3f;
    float y0 = 55.4f;
    float xDelta = 6.f;
    float yDelta = 8.6f;
    float width = 30;
    if (module) {
      for (int c=0; c<module->oldChannels; c++) {
        int cx = c/4;
        int cy = c - cx*4;
        drawNote(args, x0 + module->channelNote[c]*width + cx*xDelta, y0 - cy*yDelta, module->channelOct[c], font);
      }
    } else drawNote(args, x0, y0, 0, font);
  }

};

struct IntvlUnitSwitch : CKSSThreeLockable {
  void onChange( const ChangeEvent& e) override {
    if (module)
      static_cast<NORS_IQ*>(module)->setIntervalUnit();
    CKSSThreeLockable::onChange(e);
  }
};

struct RootUnitSwitch : CKSSThreeLockable {
  void onChange( const ChangeEvent& e) override {
    if (module)
      static_cast<NORS_IQ*>(module)->setRootUnit();
    CKSSThreeLockable::onChange(e);
  }
};

struct NORS_IQWidget : VenomWidget {

  NORS_IQWidget(NORS_IQ* module) {
    setModule(module);
    setVenomPanel("NORS_IQ");

    addParam(createLockableParam<IntvlUnitSwitch>(Vec(31.743, 68.0), module, NORS_IQ::INTVL_UNIT_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(77.871, 82.0), module, NORS_IQ::EQUAL_DIVS_PARAM, NORS_IQ::EQUAL_DIVS_LIGHT));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(125.0, 82.0), module, NORS_IQ::POI_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(164.1, 82.0), module, NORS_IQ::EDPO_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(201.2, 82.0), module, NORS_IQ::LENGTH_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(239.3, 82.0), module, NORS_IQ::ROOT_PARAM));
    addParam(createLockableParam<RootUnitSwitch>(Vec(264.881, 68.0), module, NORS_IQ::ROOT_UNIT_PARAM));
    addParam(createLockableParam<CKSSThreeLockable>(Vec(317.264, 68.0), module, NORS_IQ::ROUND_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(374.188, 82.0), module, NORS_IQ::EQUI_PARAM, NORS_IQ::EQUI_LIGHT));

    float x = 22.5f, y = 206.0f;
    for (int i=0; i<INTVL_CNT; i++) {
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(x, y), module, NORS_IQ::INTVL_PARAM+i));
      addInput(createInputCentered<MonoPort>(Vec(x, y+32.5f), module, NORS_IQ::INTVL_INPUT+i));
      x+=30.f;
    }

    addInput(createInputCentered<PolyPort>(Vec(35.603, 312.834), module, NORS_IQ::POLY_INTVL_INPUT));

    addInput(createInputCentered<MonoPort>(Vec(125.0, 312.834), module, NORS_IQ::POI_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(164.1, 312.834), module, NORS_IQ::EDPO_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(201.2,312.834), module, NORS_IQ::LENGTH_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(239.3,312.834), module, NORS_IQ::ROOT_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(299.279, 289.616), module, NORS_IQ::IN_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(333.867, 289.616), module, NORS_IQ::TRIG_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(368.455, 289.616), module, NORS_IQ::TRIG_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(299.279, 336.052), module, NORS_IQ::OUT_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(333.867, 336.052), module, NORS_IQ::POCT_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(368.455, 336.052), module, NORS_IQ::SCALE_OUTPUT));
    
    NORS_IQDisplay* display = createWidget<NORS_IQDisplay>(Vec(6.83,102.629));
    display->box.size = Vec(392.233, 84.847);
    display->module = module;
    display->moduleWidget = this;
    addChild(display);
  }

  void step() override {
    VenomWidget::step();
    NORS_IQ* mod = dynamic_cast<NORS_IQ*>(this->module);
    if(mod) {
      mod->lights[NORS_IQ::EQUI_LIGHT].setBrightness(mod->params[NORS_IQ::EQUI_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[NORS_IQ::EQUAL_DIVS_LIGHT].setBrightness(mod->params[NORS_IQ::EQUAL_DIVS_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }

};

Model* modelNORS_IQ = createModel<NORS_IQ, NORS_IQWidget>("NORS_IQ");
