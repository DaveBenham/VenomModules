  void step() override {
    MODULE_NAME* module = (MODULE_NAME*)this->module;
    if (module){
      if (module->defaultTheme != getDefaultTheme()){
        module->defaultTheme = getDefaultTheme();
        if(module->currentTheme == 0)
          module->prevTheme = -1;
      }
      if (module->prevTheme != module->currentTheme){
        module->prevTheme = module->currentTheme;
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, faceplatePath(moduleName, module->currentThemeStr()))));
      }
    }
    Widget::step();
  }
