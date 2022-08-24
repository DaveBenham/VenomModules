#include "plugin.hpp"
#include "util.hpp"

#define SLIDER_COUNT 8
#define MAX_PATTERN_LENGTH 16
#define MAX_UNIT64 18446744073709551615ul

static const std::vector<std::string> SLIDER_LABELS = {
	"1/4",
	"1/8",
	"1/16",
	"1/32",
	"1 T",
	"1/2 T",
	"1/4 T",
	"1/8 T",
};

static const int GATE_LENGTH [SLIDER_COUNT] = {
	24,
	12,
	6,
	3,
	16,
	8,
	4,
	2,
};

struct RandomRhythmGenerator1 : Module {
	enum ParamId {
		ENUMS(DENSITY_PARAM, SLIDER_COUNT),
		NEW_SEED_BUTTON_PARAM,
		PATERN_LENGTH_PARAM,
		ENUMS(RATE_PARAM, SLIDER_COUNT),
		RESET_BUTTON_PARAM,
		ENUMS(MUTE_CHANNEL_PARAM, SLIDER_COUNT),
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		SEED_INPUT,	
		RNG_OVERRIDE_INPUT,
		RESET_TRIGGER_INPUT,
		ENUMS(DENSITY_CHANNEL_INPUT, SLIDER_COUNT),
		DENSITY_CHANNEL_POLY_INPUT,
		NEW_SEED_TRIGGER_PARAM,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(GATE_OUTPUT, SLIDER_COUNT),
		SEED_OUTPUT,
		GATE_POLY_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(PATERN_STEP_LIGHT, MAX_PATTERN_LENGTH),
		LIGHTS_LEN
	};

	//Persistant State

	float internalSeed;

	//Non Persistant State

	int currentPulse;
	int currentCycle;
	rack::random::Xoroshiro128Plus rng;
	bool clockHigh;
	bool resetTrigHigh;
	bool resetBtnHigh;
	bool newSeedBtnHigh;
	bool newSeedTrigHigh;
	int gateHigh [SLIDER_COUNT];

	RandomRhythmGenerator1() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(CLOCK_INPUT,"Clock");
		configInput(NEW_SEED_TRIGGER_PARAM,"New Seed");
		configInput(RESET_TRIGGER_INPUT,"Reset");
		configInput(SEED_INPUT,"Seed Input");
		configInput(RNG_OVERRIDE_INPUT,"RNG Override");
		configOutput(SEED_OUTPUT,"Seed Output");

		configButton(NEW_SEED_BUTTON_PARAM, "New Seed");
		configButton(RESET_BUTTON_PARAM,"Reset");
		configParam(PATERN_LENGTH_PARAM, 1, MAX_PATTERN_LENGTH, 4, "Pattern Length");

		for(int si = 0; si < SLIDER_COUNT; si++){
			std::string si_s = std::to_string(si+1);
			configSwitch(RATE_PARAM + si, 0, 7, si, "Rate " + si_s, SLIDER_LABELS);
			configParam(DENSITY_PARAM + si, 0.f, 10.f, 0.f, "Density " + si_s, " V");
			configOutput(GATE_OUTPUT + si, "Gate " + si_s);
			configInput(DENSITY_CHANNEL_INPUT + si, "Density CV" + si_s);
			configSwitch(MUTE_CHANNEL_PARAM + si, 0, 1 , 0, "Mute " + si_s, {"Off","On"});
		}
		configInput(DENSITY_CHANNEL_POLY_INPUT,"Density Poly CV");

		initalize();
	}

	void onReset(const ResetEvent& e) override {
		Module::onReset(e);
		initalize();
	}

	void initalize(){
		currentPulse = 0;
		currentCycle = 0;
		rng = {};
		clockHigh = false;
		resetTrigHigh = false;
		resetBtnHigh = false;
		newSeedBtnHigh = false;
		newSeedTrigHigh = false;
		memset(gateHigh, 0, sizeof(gateHigh));
		internalSeed = rack::random::uniform() * 10.f;
		reseedRng();
	}

	json_t *dataToJson() override{
		json_t *jobj = json_object();

		json_object_set_new(jobj, "internalSeed", json_real(internalSeed));

		return jobj;
	}

	void dataFromJson(json_t *jobj) override {		

		internalSeed = json_real_value(json_object_get(jobj, "internalSeed"));
	}

	void process(const ProcessArgs& args) override {		

		//Clock Logic
		bool clockEvent = schmittTrigger(clockHigh,inputs[CLOCK_INPUT].getVoltage());
		bool resetEvent = false;
		if(schmittTrigger(resetTrigHigh,inputs[RESET_TRIGGER_INPUT].getVoltage())) resetEvent = true;
		if(buttonTrigger(resetBtnHigh,params[RESET_BUTTON_PARAM].getValue())) resetEvent = true;

		bool newSeedEvent = false;
		if(schmittTrigger(newSeedBtnHigh,inputs[NEW_SEED_TRIGGER_PARAM].getVoltage())) newSeedEvent = true;
		if(buttonTrigger(newSeedBtnHigh,params[NEW_SEED_BUTTON_PARAM].getValue())) newSeedEvent = true;

		bool newCycle = false;
		bool endOfCycle = false;

		outputs[GATE_POLY_OUTPUT].setChannels(8);

		if(resetEvent){
			currentCycle = 0;
			currentPulse = 0;
			endOfCycle = true;
			newSeedEvent = true;
		}

		if(newSeedEvent){
			internalSeed = rack::random::uniform() * 10.f;
		}

		if(clockEvent){
			currentPulse++;
			if(currentPulse % 24 == 0){
				newCycle = true;
			}

			for(int si = 0; si < SLIDER_COUNT; si++){
				if(gateHigh[si] > 0){
					DEBUG("gateHigh[%i] = %i",si,gateHigh[si]);
					gateHigh[si]--;
					if(gateHigh[si] == 0){
						//Gate Low
						outputs[GATE_OUTPUT + si].setVoltage(0.f);
						outputs[GATE_POLY_OUTPUT].setVoltage(0.f, si);
					}
				}

				int rate = static_cast<int>(params[RATE_PARAM + si].getValue());

				int gateLength = GATE_LENGTH[rate];
				bool gateCheck = currentPulse % gateLength == 0;
				if(gateCheck){
					float rndFloat = ((rng() >> 32) * 2.32830629e-10f) * 10.f;
					rndFloat = inputs[RNG_OVERRIDE_INPUT].getNormalVoltage(rndFloat, si);
					float threshold = params[DENSITY_PARAM + si].getValue();
					threshold = inputs[DENSITY_CHANNEL_INPUT + si].getNormalVoltage(threshold);
					threshold = inputs[DENSITY_CHANNEL_POLY_INPUT].getNormalVoltage(threshold, si);

					if(rndFloat < threshold && params[MUTE_CHANNEL_PARAM + si].getValue() == 0){
						outputs[GATE_OUTPUT + si].setVoltage(10.f);
						outputs[GATE_POLY_OUTPUT].setVoltage(10.f, si);
						gateHigh[si] = gateLength / 2;
					}
				}	
			}
		}

		if(newCycle){
			currentCycle++;
			int maxCycle = params[PATERN_LENGTH_PARAM].getValue();
			if(currentCycle >= maxCycle){
				currentCycle = 0;
				currentPulse = 0;
				endOfCycle = true;
			}
		}

		if(endOfCycle){
			reseedRng();
		}

		//Update Cycle Lights
		for(int ci = 0; ci < MAX_PATTERN_LENGTH; ci++){
			lights[PATERN_STEP_LIGHT + ci].setBrightness(currentCycle == ci ? 1.f : 0.f);
		}

		outputs[SEED_OUTPUT].setVoltage(internalSeed);
	}

	void reseedRng(){
		float seed = inputs[SEED_INPUT].getNormalVoltage(internalSeed);
		float seed1 = seed / 10.f;
		float seed2 = std::fmod(seed,1.f);
		uint64_t s1 = seed1 * MAX_UNIT64;
		uint64_t s2 = seed2 * MAX_UNIT64;
		rng.seed(s1, s2);
	}
};


struct RandomRhythmGenerator1Widget : ModuleWidget {

	struct RateSwitch : app::SvgSwitch {
		RateSwitch() {
			shadow->opacity = 0.0;
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_0.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_1.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_2.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_3.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_4.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_5.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_6.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_7.svg")));
		}
	};

	struct MuteSwitch : app::SvgSwitch {
		MuteSwitch() {
			shadow->opacity = 0.0;
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/mute_0.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/mute_1.svg")));
		}
	};

	RandomRhythmGenerator1Widget(RandomRhythmGenerator1* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Blank26hp.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		float dx = RACK_GRID_WIDTH * 2;
		float dy = RACK_GRID_WIDTH * 2;

		float yStart = RACK_GRID_WIDTH*2;
		float xStart = RACK_GRID_WIDTH;

		float x = xStart;
		float y = yStart;

		addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RandomRhythmGenerator1::CLOCK_INPUT));
		y += dy;
		addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RandomRhythmGenerator1::RNG_OVERRIDE_INPUT));
		y += dy;
		addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RandomRhythmGenerator1::SEED_INPUT));
		addOutput(createOutputCentered<PJ3410Port>(Vec(x + dx,y), module, RandomRhythmGenerator1::SEED_OUTPUT));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::NEW_SEED_BUTTON_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::NEW_SEED_TRIGGER_PARAM));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::RESET_BUTTON_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::RESET_TRIGGER_INPUT));
		y += dy;
		addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x,y), module, RandomRhythmGenerator1::PATERN_LENGTH_PARAM));

		x += dx;
		y += dy;

		x += dx * 2;
		
		for(int si = 0; si < SLIDER_COUNT; si++){
			y = yStart;
			addParam(createParamCentered<RateSwitch>(Vec(x,y), module, RandomRhythmGenerator1::RATE_PARAM + si));
			y += dy * 2;
			addParam(createParamCentered<VCVSlider>(Vec(x,y), module, RandomRhythmGenerator1::DENSITY_PARAM + si));
			y += dy * 2;
			addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RandomRhythmGenerator1::DENSITY_CHANNEL_INPUT + si));
			y += dy;
			addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_OUTPUT + si));
			y += dy;
			addParam(createParamCentered<MuteSwitch>(Vec(x,y), module, RandomRhythmGenerator1::MUTE_CHANNEL_PARAM + si));			

			x += dx;
		}

		y = yStart + dy * 4;
		addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RandomRhythmGenerator1::DENSITY_CHANNEL_POLY_INPUT));
		y += dy;
		addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_POLY_OUTPUT));

		x = xStart + dx * 2.75f;
		y = yStart + dy * 8;

		for(int li = 0; li < MAX_PATTERN_LENGTH; li++){
			addChild(createLightCentered<MediumLight<BlueLight>>(Vec(x,y), module, RandomRhythmGenerator1::PATERN_STEP_LIGHT + li));
			x += dx * 0.5f;
		}

	}

	// void appendContextMenu(Menu* menu) override {
	// 	RandomRhythmGenerator1* module = dynamic_cast<RandomRhythmGenerator1*>(this->module);

	// 	menu->addChild(new MenuEntry); //Blank Row
	// 	menu->addChild(createMenuLabel("RandomRhythmGenerator1"));
	// }
};


Model* modelRandomRhythmGenerator1 = createModel<RandomRhythmGenerator1, RandomRhythmGenerator1Widget>("RandomRhythmGenerator1");