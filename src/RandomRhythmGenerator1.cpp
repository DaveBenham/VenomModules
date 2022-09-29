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
		BAR_COUNT_PARAM,
		ENUMS(RATE_PARAM, SLIDER_COUNT),
		RESET_BUTTON_PARAM,
		ENUMS(MUTE_CHANNEL_PARAM, SLIDER_COUNT),
		RUN_GATE_PARAM,
		BAR_LENGTH_PARAM,
		LINEAR_GATE_PARAM,
		OFFBEAT_GATE_PARAM,
		ENUMS(MODE_CHANNEL_PARAM, SLIDER_COUNT),
		XOR_MODE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		SEED_INPUT,	
		RNG_OVERRIDE_INPUT,
		RESET_TRIGGER_INPUT,
		ENUMS(DENSITY_CHANNEL_INPUT, SLIDER_COUNT),
		DENSITY_CHANNEL_POLY_INPUT,
		NEW_SEED_TRIGGER_INPUT,
		RUN_GATE_INPUT,
		LINEAR_GATE_INPUT,
		OFFBEAT_GATE_INPUT,
		XOR_MODE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(GATE_OUTPUT, SLIDER_COUNT),
		SEED_OUTPUT,
		GATE_POLY_OUTPUT,
		START_OF_MEASURE_OUTPUT,
		GATE_OR_OUTPUT,
		GATE_XOR_OUTPUT,
		GATE_XOR_ODD_OUTPUT,
		GATE_XOR_ONE_OUTPUT,
		START_OF_CYCLE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(PATERN_STEP_LIGHT, MAX_PATTERN_LENGTH),
		RUN_GATE_LIGHT,
		LINEAR_GATE_LIGHT,
		OFFBEAT_GATE_LIGHT,
		XOR_MODE_LIGHT,
		LIGHTS_LEN
	};

	//Persistant State

	float internalSeed;
	bool runGateActive;
	bool linearModeActive;
	bool offbeatModeActive;
	bool xorModeActive;

	//Non Persistant State

	int currentPulse;
	int currentCycle;
	rack::random::Xoroshiro128Plus rng;
	bool clockHigh;
	bool resetTrigHigh;
	bool resetBtnHigh;
	bool newSeedBtnHigh;
	bool newSeedTrigHigh;
	bool runGateBtnHigh;
	bool runGateTrigHigh;
	bool linearModeBtnHigh;
	bool linearModeTrigHigh;
	bool offbeatModeBtnHigh;
	bool offbeatModeTrigHigh;
	bool xorModeBtnHigh;
	bool xorModeTrigHigh;
	int gateHigh [SLIDER_COUNT];

	RandomRhythmGenerator1() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(CLOCK_INPUT,"Clock");
		configInput(NEW_SEED_TRIGGER_INPUT,"New Seed");
		configInput(RESET_TRIGGER_INPUT,"Reset");
		configInput(SEED_INPUT,"Seed Input");
		configInput(RNG_OVERRIDE_INPUT,"RNG Override");
		configOutput(SEED_OUTPUT,"Seed Output");

		configButton(NEW_SEED_BUTTON_PARAM, "New Seed");
		configButton(RESET_BUTTON_PARAM,"Reset");

		configButton(RUN_GATE_PARAM, "Run Gate");
		configInput(RUN_GATE_INPUT,"Run Gate");

		configButton(LINEAR_GATE_PARAM, "Linear Gate");
		configInput(LINEAR_GATE_INPUT,"Linear Gate");

		configButton(OFFBEAT_GATE_PARAM, "Offbeat Mode");
		configInput(OFFBEAT_GATE_INPUT,"Offbeat Mode");

		configButton(XOR_MODE_PARAM, "XOR Mode");
		configInput(XOR_MODE_INPUT,"XOR Mode");
		configOutput(GATE_XOR_OUTPUT,"XOR");

		configOutput(GATE_OR_OUTPUT,"OR");
		configOutput(GATE_XOR_ODD_OUTPUT,"XOR Odd Parity");
		configOutput(GATE_XOR_ONE_OUTPUT,"XOR 1 Hot");

		configParam(BAR_COUNT_PARAM, 1, MAX_PATTERN_LENGTH, 4, "Bar Count");
		configParam(BAR_LENGTH_PARAM, 1, MAX_PATTERN_LENGTH, 4, "Bar Length");
		configOutput(START_OF_MEASURE_OUTPUT,"Start of Measure");
		configOutput(START_OF_CYCLE_OUTPUT,"Start of Cycle");

		for(int si = 0; si < SLIDER_COUNT; si++){
			std::string si_s = std::to_string(si+1);
			configSwitch(RATE_PARAM + si, 0, 7, si, "Rate " + si_s, SLIDER_LABELS);
			configParam(DENSITY_PARAM + si, 0.f, 10.f, 0.f, "Density " + si_s, " V");
			configOutput(GATE_OUTPUT + si, "Gate " + si_s);
			configInput(DENSITY_CHANNEL_INPUT + si, "Density CV" + si_s);
			configSwitch(MUTE_CHANNEL_PARAM + si, 0, 1 , 0, "Mute " + si_s, {"Off","On"});
			configSwitch(MODE_CHANNEL_PARAM + si, 0, 2 , 2, "Mode " + si_s, {"None","Linear","Offbeat"});
		}
		configInput(DENSITY_CHANNEL_POLY_INPUT,"Density Poly CV");
		configOuptut(GATE_POLY_OUTPUT, "Gate Poly");

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
		runGateActive = true;
		linearModeActive = false;
		offbeatModeActive = false;
		xorModeActive = false;
		clockHigh = false;
		resetTrigHigh = false;
		resetBtnHigh = false;
		newSeedBtnHigh = false;
		newSeedTrigHigh = false;
		runGateBtnHigh = false;
		runGateTrigHigh = false;
		linearModeBtnHigh = false;
		linearModeTrigHigh = false;
		offbeatModeBtnHigh = false;
		offbeatModeTrigHigh = false;
		xorModeBtnHigh = false;
		xorModeTrigHigh = false;
		memset(gateHigh, 0, sizeof(gateHigh));
		internalSeed = rack::random::uniform() * 10.f;
		reseedRng();
		updateLights();
	}

	json_t *dataToJson() override{
		json_t *jobj = json_object();

		json_object_set_new(jobj, "internalSeed", json_real(internalSeed));
		json_object_set_new(jobj, "runGateActive", json_bool(runGateActive));
		json_object_set_new(jobj, "linearModeActive", json_bool(linearModeActive));
		json_object_set_new(jobj, "offbeatModeActive", json_bool(offbeatModeActive));
		json_object_set_new(jobj, "xorModeActive", json_bool(xorModeActive));
		

		return jobj;
	}

	void dataFromJson(json_t *jobj) override {		

		internalSeed = json_real_value(json_object_get(jobj, "internalSeed"));
		runGateActive = json_is_true(json_object_get(jobj, "runGateActive"));
		linearModeActive = json_is_true(json_object_get(jobj, "linearModeActive"));
		offbeatModeActive = json_is_true(json_object_get(jobj, "offbeatModeActive"));
		xorModeActive = json_is_true(json_object_get(jobj, "xorModeActive"));

		reseedRng();
		updateLights();
	}

	void updateLights(){
		lights[RUN_GATE_LIGHT].setBrightness(runGateActive ? 1 : 0);
		lights[LINEAR_GATE_LIGHT].setBrightness(linearModeActive ? 1 : 0);
		lights[OFFBEAT_GATE_LIGHT].setBrightness(offbeatModeActive ? 1 : 0);
		lights[XOR_MODE_LIGHT].setBrightness(xorModeActive ? 1 : 0);
	}

	void process(const ProcessArgs& args) override {		

		//Clock Logic
		bool clockEvent = schmittTrigger(clockHigh,inputs[CLOCK_INPUT].getVoltage());
		bool resetEvent = false;
		if(schmittTrigger(resetTrigHigh,inputs[RESET_TRIGGER_INPUT].getVoltage())) resetEvent = true;
		if(buttonTrigger(resetBtnHigh,params[RESET_BUTTON_PARAM].getValue())) resetEvent = true;

		bool newSeedEvent = false;
		if(schmittTrigger(newSeedTrigHigh,inputs[NEW_SEED_TRIGGER_INPUT].getVoltage())) newSeedEvent = true;
		if(buttonTrigger(newSeedBtnHigh,params[NEW_SEED_BUTTON_PARAM].getValue())) newSeedEvent = true;

		schmittTrigger(runGateTrigHigh,inputs[RUN_GATE_INPUT].getNormalVoltage(10));
		if(buttonTrigger(runGateBtnHigh,params[RUN_GATE_PARAM].getValue())){
			runGateActive = !runGateActive;
			lights[RUN_GATE_LIGHT].setBrightness(runGateActive ? 1 : 0);
		}

		schmittTrigger(linearModeTrigHigh,inputs[LINEAR_GATE_INPUT].getNormalVoltage(10));
		if(buttonTrigger(linearModeBtnHigh,params[LINEAR_GATE_PARAM].getValue())){
			linearModeActive = !linearModeActive;
			lights[LINEAR_GATE_LIGHT].setBrightness(linearModeActive ? 1 : 0);
		}
		bool linearModeOn = linearModeActive == linearModeTrigHigh;

		schmittTrigger(offbeatModeTrigHigh,inputs[OFFBEAT_GATE_INPUT].getNormalVoltage(10));
		if(buttonTrigger(offbeatModeBtnHigh,params[OFFBEAT_GATE_PARAM].getValue())){
			offbeatModeActive = !offbeatModeActive;
			lights[OFFBEAT_GATE_LIGHT].setBrightness(offbeatModeActive ? 1 : 0);
		}
		bool offbeatModeOn = offbeatModeActive == offbeatModeTrigHigh;

		schmittTrigger(xorModeTrigHigh,inputs[XOR_MODE_INPUT].getNormalVoltage(10));
		if(buttonTrigger(xorModeBtnHigh,params[XOR_MODE_PARAM].getValue())){
			xorModeActive = !xorModeActive;
			lights[XOR_MODE_LIGHT].setBrightness(xorModeActive ? 1 : 0);
		}
		bool xorModeOn = xorModeActive == xorModeTrigHigh;

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

		//Supress clock events if runGate does not match theRunGatTrig
		if(clockEvent && runGateActive == runGateTrigHigh){
			
			// //Bar Logic
			// {
			// 	int barLength = params[BAR_LENGTH_PARAM].getValue() * 12;
			// 	int barPart = currentPulse / barLength;
			// 	outputs[BAR_OUTPUT].setVoltage(barPart % 2 == 0 ? 10.f : 0.f);
			// }

			currentPulse++;
			if(currentPulse % 24 == 0){
				newCycle = true;
			}

			bool linearModeShadow = false;
			bool offbeatModeShadow = false;
			int outGateCount = 0;
			for(int si = 0; si < SLIDER_COUNT; si++){
				if(gateHigh[si] > 0){
					// DEBUG("gateHigh[%i] = %i",si,gateHigh[si]);
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
					// uint64_t k = rng();
					// uint32_t k2 = (k >> 32);
					// float k3 = (k2 * 2.32830629e-10f);
					float rndFloat = ((rng() >> 32) * 2.32830629e-10f) * 10.f;
					rndFloat = inputs[RNG_OVERRIDE_INPUT].getNormalVoltage(rndFloat, si);
					float threshold = params[DENSITY_PARAM + si].getValue();
					threshold = inputs[DENSITY_CHANNEL_INPUT + si].getNormalVoltage(threshold);
					threshold = inputs[DENSITY_CHANNEL_POLY_INPUT].getNormalVoltage(threshold, si);

					if(si == 0) DEBUG("rndFloat:%f threshold:%f",rndFloat,threshold);
					//if(si == 0) DEBUG("rndFloat:%f threshold:%f k:%llu k2:%u k3:%f",rndFloat,threshold,k,k2,k3);

					bool modePermits = true;

					int channelMode = (int)params[MODE_CHANNEL_PARAM + si].getValue();

					if(channelMode >= 2 && offbeatModeOn && offbeatModeShadow) modePermits = false;
					if(channelMode >= 1 && linearModeOn && linearModeShadow) modePermits = false;

					if(modePermits){
						offbeatModeShadow = true;
						if(rndFloat < threshold){
							linearModeShadow = true;
							if(params[MUTE_CHANNEL_PARAM + si].getValue() == 0){
								outputs[GATE_OUTPUT + si].setVoltage(10.f);
								outputs[GATE_POLY_OUTPUT].setVoltage(10.f, si);
								gateHigh[si] = gateLength / 2;
								outGateCount ++;
							}
						}
					}
				}	
			}

			//Or Output
			outputs[GATE_OR_OUTPUT].setVoltage(outGateCount > 0 ? 10.f : 0.f);

			//Xor Output
			outputs[GATE_XOR_OUTPUT].setVoltage((xorModeOn ? (outGateCount == 1) : (outGateCount % 2 == 1)) ? 10.f : 0.f);
			outputs[GATE_XOR_ODD_OUTPUT].setVoltage((outGateCount % 2 == 1) ? 10.f : 0.f);
			outputs[GATE_XOR_ONE_OUTPUT].setVoltage((outGateCount == 1) ? 10.f : 0.f);
		}

		if(newCycle){
			currentCycle++;
			int maxMeasure = params[BAR_COUNT_PARAM].getValue();
			int barLength = params[BAR_LENGTH_PARAM].getValue();
			int maxCycle = maxMeasure * barLength;
			if(currentCycle >= maxCycle){
				currentCycle = 0;
				currentPulse = 0;
				endOfCycle = true;
			}
			outputs[START_OF_CYCLE_OUTPUT].setVoltage(currentCycle == 0 ? 10.f : 0.f);
			outputs[START_OF_MEASURE_OUTPUT].setVoltage(currentCycle % barLength == 0 ? 10.f : 0.f);
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

	struct ModeSwitch : app::SvgSwitch {
		ModeSwitch() {
			shadow->opacity = 0.0;
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_0.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_1.svg")));
			addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_2.svg")));
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
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::NEW_SEED_TRIGGER_INPUT));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::RESET_BUTTON_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::RESET_TRIGGER_INPUT));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::RUN_GATE_PARAM));
		addChild(createLightCentered<VCVBezelLight<GreenLight>>(Vec(x,y), module, RandomRhythmGenerator1::RUN_GATE_LIGHT));
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::RUN_GATE_INPUT));
		y += dy;
		addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x,y), module, RandomRhythmGenerator1::BAR_COUNT_PARAM));
		addOutput(createOutputCentered<PJ3410Port>(Vec(x + dx,y), module, RandomRhythmGenerator1::START_OF_CYCLE_OUTPUT));
		y += dy;
		addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x,y), module, RandomRhythmGenerator1::BAR_LENGTH_PARAM));
		addOutput(createOutputCentered<PJ3410Port>(Vec(x + dx,y), module, RandomRhythmGenerator1::START_OF_MEASURE_OUTPUT));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::LINEAR_GATE_PARAM));
		addChild(createLightCentered<VCVBezelLight<GreenLight>>(Vec(x,y), module, RandomRhythmGenerator1::LINEAR_GATE_LIGHT));
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::LINEAR_GATE_INPUT));
		y += dy;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::OFFBEAT_GATE_PARAM));
		addChild(createLightCentered<VCVBezelLight<GreenLight>>(Vec(x,y), module, RandomRhythmGenerator1::OFFBEAT_GATE_LIGHT));
		addInput(createInputCentered<PJ301MPort>(Vec(x + dx,y), module, RandomRhythmGenerator1::OFFBEAT_GATE_INPUT));
		

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
			y += dy;
			addParam(createParamCentered<ModeSwitch>(Vec(x,y), module, RandomRhythmGenerator1::MODE_CHANNEL_PARAM + si));

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

		x = xStart + dx * 3;
		y = yStart + dy * 10;

		addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_OR_OUTPUT));
		x += dx;
		addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_XOR_ONE_OUTPUT));
		x += dx;
		addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_XOR_ODD_OUTPUT));
		x += dx;
		x += dx;
		addOutput(createOutputCentered<PJ3410Port>(Vec(x,y), module, RandomRhythmGenerator1::GATE_XOR_OUTPUT));
		x += dx;
		addParam(createParamCentered<VCVButton>(Vec(x,y), module, RandomRhythmGenerator1::XOR_MODE_PARAM));
		addChild(createLightCentered<VCVBezelLight<GreenLight>>(Vec(x,y), module, RandomRhythmGenerator1::XOR_MODE_LIGHT));
		x += dx;
		addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RandomRhythmGenerator1::XOR_MODE_INPUT));

	}

	// void appendContextMenu(Menu* menu) override {
	// 	RandomRhythmGenerator1* module = dynamic_cast<RandomRhythmGenerator1*>(this->module);

	// 	menu->addChild(new MenuEntry); //Blank Row
	// 	menu->addChild(createMenuLabel("RandomRhythmGenerator1"));
	// }
};


Model* modelRandomRhythmGenerator1 = createModel<RandomRhythmGenerator1, RandomRhythmGenerator1Widget>("RandomRhythmGenerator1");