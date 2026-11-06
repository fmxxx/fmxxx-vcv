#include "plugin.hpp"

struct Phase_dist : Module {
  enum ParamId {
    PD_PARAM,
    PD_TYPE_PARAM,
    PD_ATT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PD_INPUT,
    PD_TYPE_INPUT,
    PHASE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    PHASE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Phase_dist() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(PD_PARAM, 0.f, 1.f, 0.f, "");
    configParam(PD_TYPE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(PD_ATT_PARAM, 0.f, 1.f, 0.f, "");
    configInput(PD_INPUT, "");
    configInput(PD_TYPE_INPUT, "");
    configInput(PHASE_INPUT, "");
    configOutput(PHASE_OUTPUT, "");
  }

  void process(const ProcessArgs& args) override {
  }
};

struct Phase_distWidget : ModuleWidget {
  Phase_distWidget(Phase_dist* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/phase_dist.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 35.56)), module, Phase_dist::PD_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 49.53)), module, Phase_dist::PD_TYPE_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(7.634, 66.03)), module, Phase_dist::PD_ATT_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 78.74)), module, Phase_dist::PD_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 91.44)), module, Phase_dist::PD_TYPE_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 104.14)), module, Phase_dist::PHASE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phase_dist::PHASE_OUTPUT));

    // mm2px(Vec(15.24, 15.24))
    addChild(createWidget<Widget>(mm2px(Vec(0.0, 10.16))));
  }
};

Model* modelPhase_dist = createModel<Phase_dist, Phase_distWidget>("phase_dist");