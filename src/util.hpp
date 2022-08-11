#pragma once

#include "plugin.hpp"

#define PI 3.141592f
#define TWO_PI 6.283185f
#define _CC(R,G,B) {R/255.f,G/255.f,B/255.f}
#define clamp01(x) clamp(x,0.f,1.f)
#define lerp(x, a, b) ((1.f - x) * a + x * b)

// C4 = frequency at which Rack 1v/octave CVs are zero.
#define REFERENCE_FREQUENCY 261.626f

struct CKSSHorizontal : app::SvgSwitch {
	CKSSHorizontal() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/CKSSHorizontal_0.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/CKSSHorizontal_1.svg")));
	}
};

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

template <typename TBase = rack::app::Switch>
struct ModeSwitch : TBase {
	ModeSwitch() {
		this->momentary = false;
		this->latch = true;
	}
};

struct FrequencyParamQuantity : ParamQuantity  {
	float getDisplayValue() override {
		float v = getValue();
		v = powf(2.0f, v);
		v *= REFERENCE_FREQUENCY;
		return v;
	}
	void setDisplayValue(float v) override {
		v /= REFERENCE_FREQUENCY;
		v = log2f(v);
		setValue(v);
	}
};

struct NoRnd : ParamQuantity {
	NoRnd(){
		randomizeEnabled = false;
	}
};
struct NoRndSwitch : SwitchQuantity {
	NoRndSwitch(){
		randomizeEnabled = false;
	}
};

struct CKSSThreeReverse : app::SvgSwitch {
	CKSSThreeReverse() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_2.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_1.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThree_0.svg")));
	}
};

json_t* json_bool(bool value);
#define json_bool_value(X) json_is_true(X)

json_t* json_vecArray(Vec * array, int length);
void json_vecArray_value(json_t* jArray, Vec * array, int length);

json_t* json_floatArray(float * array, int length);
void json_floatArray_value(json_t* jArray, float * array, int length);

json_t* json_boolArray(bool * array, int length);
void json_boolArray_value(json_t* jArray, bool * array, int length);

//void profile(int index);

float mod_0_max(float val, float max);
int mod_0_max(int val, int max);

inline void schmittTrigger(bool & state, float input, bool & highEvent, bool & lowEvent){
	if(!state && input >= 2.0f){
		state = true;
		highEvent = true;
	}else if(state && input <= 0.1f){
		state = false;
		lowEvent = true;
	}
}

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

inline void countClockLength(int & clockCounter, int & clockLength, bool clockHighEvent){
	if(clockHighEvent){
		clockLength = clockCounter;
		clockCounter = 0;
	}
	clockCounter++;
}

inline int rndInt(int max){
	return std::floor(rack::random::uniform() * max);
}

inline float dndAdvantage(float f){
	f = 1 - f;
	f = f * f;
	f = 1 - f;
	return f;
}

inline float dndDisAdvantage(float f){
	return f * f;
}

struct BtnAndTrig{
	bool active;
	bool buttonDown;
	bool trigHigh;
	bool toggle;

	bool process(float button, float trigger){
		bool trigUpEvent = schmittTrigger(trigHigh,trigger);
		bool btnHighEvent = buttonTrigger(buttonDown,button);
		if(trigUpEvent || btnHighEvent){
			active = toggle ? !active : true;
			return true;
		}else{
			return false;
		}
	}
};