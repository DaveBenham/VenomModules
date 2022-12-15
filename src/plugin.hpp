#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelBernoulliSwitch;
extern Model* modelRandomRhythmGenerator1;
extern Model* modelRecurse;
extern Model* modelVCO;
extern Model* modelWinComp;

////////////////////////////////

struct CKSSNarrow : app::SvgSwitch {
  CKSSNarrow() {
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/components/SwitchNarrow_0.svg")));
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/components/SwitchNarrow_2.svg")));
  }
};

struct DigitalDisplay : Widget {
  std::string fontPath;
  std::string bgText;
  std::string text;
  float fontSize;
  NVGcolor bgColor = nvgRGB(0x46,0x46, 0x46);
  NVGcolor fgColor = SCHEME_YELLOW;
  Vec textPos;

  void prepareFont(const DrawArgs& args) {
    // Get font
    std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
    if (!font)
      return;
    nvgFontFaceId(args.vg, font->handle);
    nvgFontSize(args.vg, fontSize);
    nvgTextLetterSpacing(args.vg, 0.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
  }

  void draw(const DrawArgs& args) override {
    // Background
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2);
    nvgFillColor(args.vg, nvgRGB(0x19, 0x19, 0x19));
    nvgFill(args.vg);

    prepareFont(args);

    // Background text
    nvgFillColor(args.vg, bgColor);
    nvgText(args.vg, textPos.x, textPos.y, bgText.c_str(), NULL);
  }

  void drawLayer(const DrawArgs& args, int layer) override {
    if (layer == 1) {
      prepareFont(args);

      // Foreground text
      nvgFillColor(args.vg, fgColor);
      nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
    }
    Widget::drawLayer(args, layer);
  }
};


struct ChannelDisplay : DigitalDisplay {
  ChannelDisplay() {
    fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
    textPos = Vec(22, 20);
    bgText = "18";
    fontSize = 16;
  }
};

template <typename TBase = GrayModuleLightWidget>
struct YlwLight : TBase {
  YlwLight() {
    this->addBaseColor(SCHEME_YELLOW);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct GrnLight : TBase {
  GrnLight() {
    this->addBaseColor(SCHEME_GREEN);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct RdLight : TBase {
  RdLight() {
    this->addBaseColor(SCHEME_RED);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct YellowBlueLight : TBase {
  YellowBlueLight() {
    this->addBaseColor(SCHEME_YELLOW);
    this->addBaseColor(SCHEME_BLUE);
  }
};


///////
// Knobs

struct RoundSmallBlackKnobSnap : RoundSmallBlackKnob {
  RoundSmallBlackKnobSnap() {
    snap = true;
  }
};

struct kHzKnob : RoundKnob {
  kHzKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzKnob.svg")));
    shadow->box.pos = Vec(0.0, 2.5);
  }
};

struct kHzKnobSmall : RoundKnob {
  kHzKnobSmall() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzKnobSmall.svg")));
    shadow->box.pos = Vec(0.0, 2.5);
  }
};

struct kHzKnobTiny : RoundKnob {
  kHzKnobTiny() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzKnobTiny.svg")));
    shadow->box.pos = Vec(0.0, 2.5);
  }
};

struct kHzKnobSnap : kHzKnob {
  kHzKnobSnap() {
    snap = true;
  }
};

struct kHzKnobSmallSnap : kHzKnobSmall {
  kHzKnobSmallSnap() {
    snap = true;
  }
};

// Buttons

struct kHzButton : SvgSwitch {
  kHzButton() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzButton_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzButton_1.svg")));
  }
};

// Ports

struct kHzPort : SvgPort {
  kHzPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzPort.svg")));
    shadow->box.pos = Vec(0.0, 1.5);
  }
};

// Misc

struct kHzScrew : SvgScrew {
  kHzScrew() {
    sw->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/kHzScrew.svg")));
  }
};
