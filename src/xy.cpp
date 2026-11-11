#include "plugin.hpp"


struct Xy : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		X1_INPUT,
		Y1_INPUT,
		X2_INPUT,
		Y2_INPUT,
		X3_INPUT,
		Y3_INPUT,
		X4_INPUT,
		Y4_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		X1_OUTPUT,
		Y1_OUTPUT,
		X2_OUTPUT,
		Y2_OUTPUT,
		X3_OUTPUT,
		Y3_OUTPUT,
		X4_OUTPUT,
		Y4_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Xy() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(X1_INPUT, "");
		configInput(Y1_INPUT, "");
		configInput(X2_INPUT, "");
		configInput(Y2_INPUT, "");
		configInput(X3_INPUT, "");
		configInput(Y3_INPUT, "");
		configInput(X4_INPUT, "");
		configInput(Y4_INPUT, "");
		configOutput(X1_OUTPUT, "");
		configOutput(Y1_OUTPUT, "");
		configOutput(X2_OUTPUT, "");
		configOutput(Y2_OUTPUT, "");
		configOutput(X3_OUTPUT, "");
		configOutput(Y3_OUTPUT, "");
		configOutput(X4_OUTPUT, "");
		configOutput(Y4_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct XyWidget : ModuleWidget {
	XyWidget(Xy* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/xy.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.89, 82.55)), module, Xy::X1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.59, 82.55)), module, Xy::Y1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.89, 93.98)), module, Xy::X2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.59, 93.98)), module, Xy::Y2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.89, 105.41)), module, Xy::X3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.59, 105.41)), module, Xy::Y3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.89, 116.84)), module, Xy::X4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21.59, 116.84)), module, Xy::Y4_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.37, 82.55)), module, Xy::X1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.07, 82.55)), module, Xy::Y1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.37, 93.98)), module, Xy::X2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.07, 93.98)), module, Xy::Y2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.37, 105.41)), module, Xy::X3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.07, 105.41)), module, Xy::Y3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.37, 116.84)), module, Xy::X4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.07, 116.84)), module, Xy::Y4_OUTPUT));

		// mm2px(Vec(55.88, 55.88))
		addChild(createWidget<Widget>(mm2px(Vec(2.54, 10.16))));
	}
};


Model* modelXy = createModel<Xy, XyWidget>("xy");