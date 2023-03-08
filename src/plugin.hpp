// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelBernoulliSwitch;
extern Model* modelCloneMerge;
extern Model* modelHQ;
extern Model* modelPolyClone;
extern Model* modelRecurse;
extern Model* modelRecurseStereo;
extern Model* modelRhythmExplorer;
extern Model* modelVCO;
extern Model* modelWinComp;

////////////////////////////////

int getDefaultTheme();
void setDefaultTheme(int theme);

template <typename TBase>
struct RotarySwitch : TBase {
	RotarySwitch() {
		this->snap = true;
		this->smooth = false;
	}

	// handle the manually entered values
	void onChange(const event::Change &e) override {
		SvgKnob::onChange(e);
		this->getParamQuantity()->setValue(roundf(this->getParamQuantity()->getValue()));
	}
};

struct CKSSNarrow : app::SvgSwitch {
  CKSSNarrow() {
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/components/SwitchNarrow_0.svg")));
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/components/SwitchNarrow_2.svg")));
  }
};

struct DigitalDisplay : Widget {
  Module* module;
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
    if (layer == 1 && (!module || (module && !module->isBypassed()))) {
      prepareFont(args);

      // Foreground text
      nvgFillColor(args.vg, fgColor);
      nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
    }
    Widget::drawLayer(args, layer);
  }
  
  void step() override {
  }
};

struct DigitalDisplay18 : DigitalDisplay {
  DigitalDisplay18() {
    fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
    textPos = Vec(22, 20);
    bgText = "18";
    fontSize = 16;
    box.size = mm2px(Vec(8.197, 8.197)); // 31px square
  }
};

struct DigitalDisplay188 : DigitalDisplay {
  DigitalDisplay188() {
    fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
    textPos = Vec(33.5, 20);
    bgText = "188";
    fontSize = 16;
    box.size = mm2px(Vec(12, 8.197));
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
struct BluLight : TBase {
  BluLight() {
    this->addBaseColor(SCHEME_BLUE);
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

template <typename TBase = GrayModuleLightWidget>
struct RedBlueLight : TBase {
  RedBlueLight() {
    this->addBaseColor(SCHEME_RED);
    this->addBaseColor(SCHEME_BLUE);
  }
};

template <typename TBase = GrayModuleLightWidget>
struct YellowRedLight : TBase {
  YellowRedLight() {
    this->addBaseColor(SCHEME_YELLOW);
    this->addBaseColor(SCHEME_RED);
  }
};

struct RoundHugeBlackKnobSnap : RoundHugeBlackKnob {
  RoundHugeBlackKnobSnap() {
    snap = true;
  }
};

struct RoundBigBlackKnobSnap : RoundBigBlackKnob {
  RoundBigBlackKnobSnap() {
    snap = true;
  }
};

struct RoundLargeBlackKnobSnap : RoundLargeBlackKnob {
  RoundLargeBlackKnobSnap() {
    snap = true;
  }
};

struct RoundBlackKnobSnap : RoundBlackKnob {
  RoundBlackKnobSnap() {
    snap = true;
  }
};

struct RoundSmallBlackKnobSnap : RoundSmallBlackKnob {
  RoundSmallBlackKnobSnap() {
    snap = true;
  }
};
