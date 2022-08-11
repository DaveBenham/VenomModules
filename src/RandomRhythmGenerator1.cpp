#include "plugin.hpp"
#include "util.hpp"

#define SLIDER_COUNT 8
#define MAX_PATTERN_LENGTH 16
#define MAX_UNIT64 18446744073709551615ul

static const std::string SLIDER_LABELS[SLIDER_COUNT] = {
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
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		SEED_INPUT,	
		RNG_OVERRIDE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(GATE_OUTPUT, SLIDER_COUNT),
		SEED_OUTPUT,
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
	bool newSeedBtnHigh;
	int gateHigh [SLIDER_COUNT];

	RandomRhythmGenerator1() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(CLOCK_INPUT,"Clock");
		configInput(SEED_INPUT,"Seed Input");
		configInput(RNG_OVERRIDE_INPUT,"RNG Override");
		configOutput(SEED_OUTPUT,"Seed Output");

		configButton(NEW_SEED_BUTTON_PARAM, "New Seed");
		configParam(PATERN_LENGTH_PARAM, 1, MAX_PATTERN_LENGTH, 4, "Pattern Length");

		for(int si = 0; si < SLIDER_COUNT; si++){
			configParam(DENSITY_PARAM + si, 0.f, 10.f, 0.f, SLIDER_LABELS[si] + " density", " V");
			configOutput(GATE_OUTPUT + si, SLIDER_LABELS[si] + " gate");
		}

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
		bool newSeedEvent = buttonTrigger(newSeedBtnHigh,params[NEW_SEED_BUTTON_PARAM].getValue());
		bool newCycle = false;
		bool endOfCycle = false;

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
					}
				}

				int gateLength = GATE_LENGTH[si];
				bool gateCheck = currentPulse % gateLength == 0;
				if(gateCheck){
					float rndFloat = ((rng() >> 32) * 2.32830629e-10f) * 10.f;
					rndFloat = inputs[RNG_OVERRIDE_INPUT].getNormalVoltage(rndFloat, si);
					float threshold = params[DENSITY_PARAM + si].getValue();

					if(rndFloat < threshold){
						outputs[GATE_OUTPUT + si].setVoltage(10.f);
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
		y += dy;
		addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::SEED_OUTPUT));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::NEW_SEED_BUTTON_PARAM));
		y += dy;
		addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x,y), module, RandomRhythmGenerator1::PATERN_LENGTH_PARAM));
		
		x = xStart + dx * 2;
		
		for(int si = 0; si < SLIDER_COUNT; si++){
			y = yStart;
			y += dy * 2;
			addParam(createParamCentered<VCVSlider>(Vec(x,y), module, RandomRhythmGenerator1::DENSITY_PARAM + si));
			y += dy * 2;
			addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_OUTPUT + si));
			x += dx;
		}

		x = xStart + dx * 1.75f;
		y = yStart + dy * 6;

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