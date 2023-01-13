#pragma once

#include "plugin.hpp"

inline bool schmittTrigger(bool & state, float input){
  if(!state && input >= 2.0f){
    state = true;
    return true;
  }else if(state && input <= 0.1f){
    state = false;
    return false;
  }
  return false;
}

inline bool buttonTrigger(bool & state, float input){
  if(!state && input >= 1.0f){
    state = true;
    return true;
  }else if(state && input <= 0.f){
    state = false;
    return false;
  }
  return false;
}

inline std::string faceplatePath(std::string name) {
  return "res/Danger/"+name+"_Danger.svg";
}
