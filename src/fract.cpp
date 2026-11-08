#include "plugin.hpp"


struct Fract : Module {
	enum ParamId {
		NUM_PARAM,
		DENOM_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_1_INPUT,
		VOCT_2_INPUT,
		NUM_INPUT,
		DENOM_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Fract() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(NUM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DENOM_PARAM, 0.f, 1.f, 0.f, "");
		configInput(VOCT_1_INPUT, "");
		configInput(VOCT_2_INPUT, "");
		configInput(NUM_INPUT, "");
		configInput(DENOM_INPUT, "");
		configOutput(VOCT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct FractWidget : ModuleWidget {
	FractWidget(Fract* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/fract.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 19.05)), module, Fract::NUM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.02)), module, Fract::DENOM_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 64.77)), module, Fract::VOCT_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Fract::VOCT_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Fract::NUM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Fract::DENOM_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Fract::VOCT_OUTPUT));
	}
};


Model* modelFract = createModel<Fract, FractWidget>("fract");