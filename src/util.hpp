#pragma once

#include "plugin.hpp"

static const std::vector<std::string> modThemes = {
  "Venom Default",
  "Danger",
  "Earth",
  "Coal",
  "Ivory"
};

static const std::vector<std::string> themes = {
  "Danger",
  "Earth",
  "Coal",
  "Ivory"
};

inline std::string faceplatePath(std::string mod, std::string theme = "Danger") {
  return "res/"+theme+"/"+mod+"_"+theme+".svg";
}
