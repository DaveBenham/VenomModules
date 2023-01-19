#pragma once

#include "plugin.hpp"

static const std::vector<std::string> modThemes = {
  "Venom Default",
  "Ivory",
  "Coal",
  "Earth",
  "Danger"
};

static const std::vector<std::string> themes = {
  "Ivory",
  "Coal",
  "Earth",
  "Danger"
};

inline std::string faceplatePath(std::string mod, std::string theme = "Ivory") {
  return "res/"+theme+"/"+mod+"_"+theme+".svg";
}
