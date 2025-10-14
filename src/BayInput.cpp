// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "BayModule.hpp"

// BayInput module defined in BayModule.hpp

struct BayInputWidget : VenomWidget {
  
  BayInput* mod = NULL;

  struct BayInputLabelsWidget : widget::Widget {
    BayInput* mod=NULL;
    std::string modName;
    std::string fontPath;
  
    BayInputLabelsWidget() {
      fontPath = asset::system("res/fonts/DejaVuSans.ttf");
    }
  
    void draw(const DrawArgs& args) override {
      modName = mod ? mod->modName : "BAY INPUT";
      int theme = mod ? mod->currentTheme : 0;
      if (!theme) theme = settings::preferDarkPanels ? getDefaultDarkTheme()+1 : getDefaultTheme()+1;
      std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
      if (!font)
        return;
      nvgFontFaceId(args.vg, font->handle);
      nvgTextAlign(args.vg, NVG_ALIGN_CENTER + NVG_ALIGN_MIDDLE);
      nvgFontSize(args.vg, 11);
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
      nvgText(args.vg, 37.5f, 13.f, modName.c_str(), NULL);
      nvgFontSize(args.vg, 9.5);
      std::string text;
      for (int i=0; i<BayModule::INPUTS_LEN; i++) {
        text = mod ? mod->inputInfos[i]->name : "Port "+std::to_string(i+1);
        nvgText(args.vg, 37.5f, 32+i*42, text.c_str(), NULL);
      }
    }
  };

  BayInputWidget(BayInput* module) {
    mod = module;
    setModule(module);
    setVenomPanel("BayInput");
    BayInputLabelsWidget* text = createWidget<BayInputLabelsWidget>(Vec(0.f,0.f));
    text->mod = mod;
    text->box.size = box.size;
    addChild(text);
    for (int i=0; i<8; i++) {
      addInput(createInputCentered<PolyPort>(Vec(37.5f,48.5f+i*42), module, BayInput::POLY_INPUT + i));
    }
  }

  void appendContextMenu(Menu* menu) override {
    BayInput* thisMod = static_cast<BayInput*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Bay Input name", thisMod->modName,
      [=](Menu *menu){
        MenuTextField *editField = new MenuTextField();
        editField->box.size.x = 250;
        editField->setText(thisMod->modName);
        editField->changeHandler = [=](std::string text) {
          thisMod->modName = text;
        };
        menu->addChild(editField);
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

std::map<int64_t, BayInput*> BayModule::sources{};

Model* modelVenomBayInput = createModel<BayInput, BayInputWidget>("BayInput");
