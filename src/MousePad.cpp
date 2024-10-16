// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

#define SHIFT_PRESS 1
#define CTRL_PRESS 2
#define ALT_PRESS 4

struct MousePad : VenomModule {

  enum ParamId {
    SHIFT_PARAM,
    CTRL_PARAM,
    ALT_PARAM,
    TOG_PARAM,
    XSCALE_PARAM,
    YSCALE_PARAM,
    XORIGIN_PARAM,
    YORIGIN_PARAM,
    XABS_PARAM,
    YABS_PARAM,
    XRETURN_PARAM,
    YRETURN_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    INPUTS_LEN
  };
  enum OutputId {
    GATE_OUTPUT,
    X_OUTPUT,
    Y_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    SHIFT_LIGHT,
    CTRL_LIGHT,
    ALT_LIGHT,
    TOG_LIGHT,
    XABS_LIGHT,
    YABS_LIGHT,
    XRETURN_LIGHT,
    YRETURN_LIGHT,
    LIGHTS_LEN
  };

//  math::Vec mousePos{};
  float x=0.f;
  float y=0.f;
  float gate=0.f;
  
  MousePad() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SHIFT_PARAM, 0.f, 1.f, 0.f, "Shift activation", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(TOG_PARAM, 0.f, 1.f, 0.f, "Toggle activation", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(CTRL_PARAM, 0.f, 1.f, 0.f, "Control activation", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(ALT_PARAM, 0.f, 1.f, 0.f, "Alt activation", {"Off", "On"});

    configParam(XSCALE_PARAM, -2.f, 2.f, 1.f, "X scale");
    configParam(YSCALE_PARAM, -2.f, 2.f, 1.f, "Y scale");
    configParam(XORIGIN_PARAM, 0.f, 1.f, 0.5f, "X origin", "%", 0, 100, 0);
    configParam(YORIGIN_PARAM, 0.f, 1.f, 0.5f, "Y origin", "%", 0, 100, 0);

    configSwitch<FixedSwitchQuantity>(XABS_PARAM, 0.f, 1.f, 0.f, "X absolute position", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(YABS_PARAM, 0.f, 1.f, 0.f, "Y absolute position", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(XRETURN_PARAM, 0.f, 1.f, 1.f, "X return on release", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(YRETURN_PARAM, 0.f, 1.f, 1.f, "Y return on release", {"Off", "On"});

    configOutput(GATE_OUTPUT, "Gate");
    configOutput(X_OUTPUT, "X");
    configOutput(Y_OUTPUT, "Y");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    outputs[GATE_OUTPUT].setVoltage(gate);
    outputs[X_OUTPUT].setVoltage(x);
    outputs[Y_OUTPUT].setVoltage(y);
  }

};

struct MousePadWidget : VenomWidget {
  
  MousePad* mod = NULL;
  bool activateState = false;
  bool active = false;
  float xRelativeOrigin = 0.f;
  float yRelativeOrigin = 0.f;
  float xSave = 0.f;
  float ySave = 0.f;
  
  MousePadWidget(MousePad* module) {
    setModule(module);
    setVenomPanel("MousePad");
    mod = module;

    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(12.f,59.f), module, MousePad::SHIFT_PARAM, MousePad::SHIFT_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(33.f,59.f), module, MousePad::TOG_PARAM, MousePad::TOG_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(12.f,89.f), module, MousePad::CTRL_PARAM, MousePad::CTRL_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(33.f,89.f), module, MousePad::ALT_PARAM, MousePad::ALT_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.5f, 134.f), module, MousePad::XSCALE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(33.5f, 134.f), module, MousePad::YSCALE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.5f, 164.f), module, MousePad::XORIGIN_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(33.5f, 164.f), module, MousePad::YORIGIN_PARAM));

    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(12.f,194.f), module, MousePad::XABS_PARAM, MousePad::XABS_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(33.f,194.f), module, MousePad::YABS_PARAM, MousePad::YABS_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(12.f,224.f), module, MousePad::XRETURN_PARAM, MousePad::XRETURN_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(33.f,224.f), module, MousePad::YRETURN_PARAM, MousePad::YRETURN_LIGHT));

    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,271.5f), module, MousePad::GATE_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,305.5f), module, MousePad::X_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,339.5f), module, MousePad::Y_OUTPUT));
  }
  
  void step() override {
    VenomWidget::step();
    if (mod){
      mod->lights[MousePad::SHIFT_LIGHT].setBrightness(mod->params[MousePad::SHIFT_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::TOG_LIGHT].setBrightness(mod->params[MousePad::TOG_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::CTRL_LIGHT].setBrightness(mod->params[MousePad::CTRL_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::ALT_LIGHT].setBrightness(mod->params[MousePad::ALT_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::XABS_LIGHT].setBrightness(mod->params[MousePad::XABS_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::YABS_LIGHT].setBrightness(mod->params[MousePad::YABS_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::XRETURN_LIGHT].setBrightness(mod->params[MousePad::XRETURN_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[MousePad::YRETURN_LIGHT].setBrightness(mod->params[MousePad::YRETURN_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);

      int activateMask = (mod->params[MousePad::SHIFT_PARAM].getValue() ? SHIFT_PRESS : 0)
                       + (mod->params[MousePad::CTRL_PARAM].getValue() ? CTRL_PRESS : 0)
                       + (mod->params[MousePad::ALT_PARAM].getValue() ? ALT_PRESS : 0);
      if (!activateMask){
        active = false;
        activateState = false;
        mod->x = 0.f;
        mod->y = 0.f;
        mod->gate = 0.f;
        xSave = 0.f;
        ySave = 0.f;
        return;
      }
      int keys = (glfwGetKey(APP->window->win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(APP->window->win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ? SHIFT_PRESS : 0)
               + (glfwGetKey(APP->window->win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(APP->window->win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ? CTRL_PRESS : 0)
               + (glfwGetKey(APP->window->win, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(APP->window->win, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ? ALT_PRESS : 0);
      bool activate = (keys == activateMask);
      float xIn = clamp(APP->scene->getMousePos().x/APP->scene->box.size.x,0.f,1.f);
      float yIn = 1-clamp(APP->scene->getMousePos().y/APP->scene->box.size.y,0.f,1.f);
      if (mod->params[MousePad::TOG_PARAM].getValue()) { // toggle mode
        if (activate && !activateState){
          active = !active;
          if (active) {
            xRelativeOrigin = xIn;
            yRelativeOrigin = yIn;
          }
        }
      } else { // gate mode
        active = activate;
        if (active && !activateState){
          xRelativeOrigin = xIn;
          yRelativeOrigin = yIn;
        }
      }
      activateState = activate;
      if (!active){
        mod->gate = 0.f;
        if (mod->params[MousePad::XRETURN_PARAM].getValue()){
          mod->x = 0.f;
          xSave = 0.f;
        } else xSave = mod->x;
        if (mod->params[MousePad::YRETURN_PARAM].getValue()){
          mod->y = 0.f;
          ySave = 0.f;
        } else ySave = mod->y;
        return;
      }
      mod->gate = 10.f;
      xIn -= mod->params[MousePad::XABS_PARAM].getValue() ? mod->params[MousePad::XORIGIN_PARAM].getValue() : xRelativeOrigin;
      yIn -= mod->params[MousePad::YABS_PARAM].getValue() ? mod->params[MousePad::YORIGIN_PARAM].getValue() : yRelativeOrigin;
      mod->x = xIn * mod->params[MousePad::XSCALE_PARAM].getValue() * 10.f + (mod->params[MousePad::XABS_PARAM].getValue() ? 0.f : xSave);
      mod->y = yIn * mod->params[MousePad::YSCALE_PARAM].getValue() * 10.f + (mod->params[MousePad::YABS_PARAM].getValue() ? 0.f : ySave);
    }
  }
  
};

Model* modelMousePad = createModel<MousePad, MousePadWidget>("MousePad");
