// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "BenjolinModule.hpp"

struct BenjolinGatesExpanderWidget : BenjolinExpanderWidget {
  int venomDelCnt = 0;

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  struct PolaritySwitch : GlowingSvgSwitchLockable {
    PolaritySwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  struct GatePort : MonoPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      static_cast<BenjolinGatesExpander*>(this->module)->gateOutputMenu(menu,portId);
      MonoPort::appendContextMenu(menu);
    }
  };
  
  GatePort* createGateOutputCentered(math::Vec pos, engine::Module* module, int portId) {
    GatePort* o = createOutputCentered<GatePort>(pos, module, portId);
    o->portId = portId;
    return o;
  }

  struct GatesLabelsWidget : widget::Widget {
    BenjolinGatesExpander* mod=NULL;
    std::string fontPath;
  
    GatesLabelsWidget() {
      fontPath = asset::system("res/fonts/DejaVuSans.ttf");
    }
  
    void draw(const DrawArgs& args) override {
      int theme = mod ? mod->currentTheme : 0;
      if (!theme) theme = settings::preferDarkPanels ? getDefaultDarkTheme()+1 : getDefaultTheme()+1;
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (!font)
        return;
      nvgFontFaceId(args.vg, font->handle);
      nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
      switch (theme) {
        case 1: // ivory
          nvgFillColor(args.vg, nvgRGB(0xed, 0xe7, 0xdc));
          break;
        case 2: // coal
          nvgFillColor(args.vg, nvgRGB(0x25, 0x25, 0x25));
          break;
        case 3: // earth
          nvgFillColor(args.vg, nvgRGB(0x42, 0x39, 0x32));
          break;
        default: // danger
          nvgFillColor(args.vg, nvgRGB(0xf2, 0xf2, 0xf2));
          break;
      }
      nvgFontSize(args.vg, 8.0);
      std::string text;
      for (int i=0; i<8; i++) {
        text = mod ? mod->outputInfos[i]->name : std::to_string(i+1);
        nvgText(args.vg, 22.5f, 77.f+i*35.f, text.c_str(), NULL);
      }
    }
  };

  BenjolinGatesExpanderWidget(BenjolinGatesExpander* module) {
    setModule(module);
    setVenomPanel("BenjolinGatesExpander");

    GatesLabelsWidget* labels = createWidget<GatesLabelsWidget>(Vec(0.f,0.f));
    labels->mod = module;
    labels->box.size = box.size;
    addChild(labels);

    addParam(createLockableParamCentered<ModeSwitch>(Vec(11.780,57.415), module, BenjolinModule::GATES_MODE_PARAM));
    addParam(createLockableParamCentered<PolaritySwitch>(Vec(33.221,57.415), module, BenjolinModule::GATES_POLARITY_PARAM));
    float delta=0.f;
    for(int i=0; i<8; i++){
      addOutput(createGateOutputCentered(Vec(22.5f,92.5f+delta), module, BenjolinModule::GATE_OUTPUT+i));
      addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(36.f, 84.f+delta), module, BenjolinModule::GATE_LIGHT+i));
      delta+=35.f;
    }  
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(6.f, 33.f), module, 0));
  }
  
};

Model* modelBenjolinGatesExpander = createModel<BenjolinGatesExpander, BenjolinGatesExpanderWidget>("BenjolinGatesExpander");
