// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

    json_t* themeVal = json_object_get(rootJ, "currentTheme");
    if (themeVal)
      currentTheme = json_integer_value(themeVal);
