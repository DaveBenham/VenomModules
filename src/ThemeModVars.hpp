// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

  int currentTheme = 0;
  int defaultTheme = getDefaultTheme();
  int prevTheme = -1;

  std::string currentThemeStr(){
    return modThemes[currentTheme==0 ? defaultTheme+1 : currentTheme];
  }
