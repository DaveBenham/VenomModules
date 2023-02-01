// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

  menu->addChild(new MenuSeparator);

  menu->addChild(createIndexSubmenuItem(
    "Venom Default Theme",
    themes,
    [=]() {
      return getDefaultTheme();
    },
    [=](int theme) {
      setDefaultTheme(theme);
    }
  ));

  menu->addChild(createIndexSubmenuItem(
    "Theme",
    modThemes,
    [=]() {
      return module->currentTheme;
    },
    [=](int theme) {
      module->currentTheme = theme;
    }
  ));
