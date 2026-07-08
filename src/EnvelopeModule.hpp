#include "Venom.hpp"

namespace Venom {
  
struct EnvelopeModule : VenomModule {

  enum ParamId {
    SLOW_PARAM,
    FROM0_PARAM,
    GATE_MODE_PARAM,
    RETRIG_MODE_PARAM,
    GATE_IN_PARAM,
    RETRIG_PARAM,
    ENUMS(ACTION_PARAM,4),
    ENUMS(MODE_PARAM,4),
    ENUMS(A_PARAM,4),
    ENUMS(A_CV_PARAM,4),
    ENUMS(B_PARAM,4),
    ENUMS(B_CV_PARAM,4),
    PARAMS_LEN
  };
  enum InputId {
    GATE_INPUT,
    RETRIG_INPUT,
    ENUMS(A_CV_INPUT,4),
    ENUMS(B_CV_INPUT,4),
    INPUTS_LEN
  };
  enum OutputId {
    IDLE_OUTPUT,
    INV_OUTPUT,
    ENV_OUTPUT,
    ENUMS(GATE_OUTPUT,4),
    OUTPUTS_LEN
  };
  enum LightId {
    GATE_IN_LIGHT,
    RETRIG_LIGHT,
    IDLE_LIGHT,
    ENUMS(UP_LIGHT,4),
    ENUMS(DOWN_LIGHT,4),
    ENUMS(GATE_LIGHT,4),
    LIGHTS_LEN
  };

  enum ExpParamId {
    ACTION_EXP_PARAM,
    MODE_EXP_PARAM,
    A_EXP_PARAM,
    A_CV_EXP_PARAM,
    B_EXP_PARAM,
    B_CV_EXP_PARAM,
    EXP_PARAMS_LEN
  };
  enum ExpInputId {
    A_CV_EXP_INPUT,
    B_CV_EXP_INPUT,
    EXP_INPUTS_LEN
  };
  enum ExpOutputId {
    GATE_EXP_OUTPUT,
    EXP_OUTPUTS_LEN
  };
  enum ExpLightId {
    EXPAND_EXP_LIGHT,
    UP_EXP_LIGHT,
    DOWN_EXP_LIGHT,
    GATE_EXP_LIGHT,
    EXP_LIGHTS_LEN
  };

  bool up[4]{},
       down[4]{},
       reset = false;
  EnvelopeModule *expander = NULL;

  struct BQuantity : ParamQuantity {
    int action = 0;
    float getDisplayValue() override {
      return action && getValue() == -3.f ? 0.f : ParamQuantity::getDisplayValue();
    }
  };
  
  void onExpanderChange(const ExpanderChangeEvent &e) override {
    if (e.side) { // only care about right
      Module *right = getRightExpander().module;
      expander = (right && right->model==modelVenomEnvelopeExpander) ? static_cast<EnvelopeModule*>(right) : NULL;
    }
  }
  
};

struct EnvelopeModuleWidget : VenomWidget {
  int lightTheme = getDefaultTheme(),
      darkTheme = getDefaultDarkTheme();
  
  struct LabelWidget : FramebufferWidget {
    SvgWidget *sw;
    std::string labelNames[3] {"EnvelopeMoveLabels", "EnvelopeHoldLabels", "EnvelopeSustLabels"};
    int currentAction = -1;
    int currentTheme = -1;

    LabelWidget(Vec vec) {
      setPosition(vec);
      sw = new SvgWidget;
      addChild(sw);
    }
    
    void setLabel(int theme, int action) {
      if (theme!=currentTheme || action!=currentAction) {
        sw->setSvg(Svg::load(asset::plugin(pluginInstance,faceplatePath(labelNames[action], themes[theme]))));
        box.size = sw->box.size;
        currentAction = action;
        currentTheme = theme;
        setDirty();
      }
    }

    void  draw (const DrawArgs &args) override {
      oversample = APP->window->pixelRatio<2.0 ? 2.0 : 1.0;
      FramebufferWidget::draw(args);
    }
  };
  
  LabelWidget* labelWidget[4]{};

  struct ActionSwitch : GlowingSvgSwitchLockable {
    ActionSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Move.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Hold.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Sust.svg")));
    }
  };

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_Full.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_RTrg.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_Gate.svg")));
    }
  };

};

}