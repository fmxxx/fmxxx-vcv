#include "plugin.hpp"

struct Wt : Module {
  enum ParamId {
    SMOOTH_PARAM,
    VOICES_PARAM,
    RECT50_PARAM,
    RECT51_PARAM,
    TUNE_PARAM,
    WAVE_PARAM,
    FM_PARAM,
    DETUNE_PARAM,
    DIST1_PARAM,
    DIST2_PARAM,
    WAVE_ATT_PARAM,
    FM_ATT_PARAM,
    DIST2_ATT_PARAM,
    DETUNE_ATT_PARAM,
    DIST1_ATT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    WAVE_INPUT,
    FM_INPUT,
    DETUNE_INPUT,
    DIST1_INPUT,
    DIST2_INPUT,
    VOCT_INPUT,
    RESET_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT_L_OUTPUT,
    OUT_R_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Wt() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(SMOOTH_PARAM, 0.f, 1.f, 0.f, "");
    configParam(VOICES_PARAM, 0.f, 1.f, 0.f, "");
    configParam(RECT50_PARAM, 0.f, 1.f, 0.f, "");
    configParam(RECT51_PARAM, 0.f, 1.f, 0.f, "");
    configParam(TUNE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(WAVE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(FM_PARAM, 0.f, 1.f, 0.f, "");
    configParam(DETUNE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(DIST1_PARAM, 0.f, 1.f, 0.f, "");
    configParam(DIST2_PARAM, 0.f, 1.f, 0.f, "");
    configParam(WAVE_ATT_PARAM, 0.f, 1.f, 0.f, "");
    configParam(FM_ATT_PARAM, 0.f, 1.f, 0.f, "");
    configParam(DIST2_ATT_PARAM, 0.f, 1.f, 0.f, "");
    configParam(DETUNE_ATT_PARAM, 0.f, 1.f, 0.f, "");
    configParam(DIST1_ATT_PARAM, 0.f, 1.f, 0.f, "");
    configInput(WAVE_INPUT, "");
    configInput(FM_INPUT, "");
    configInput(DETUNE_INPUT, "");
    configInput(DIST1_INPUT, "");
    configInput(DIST2_INPUT, "");
    configInput(VOCT_INPUT, "");
    configInput(RESET_INPUT, "");
    configOutput(OUT_L_OUTPUT, "");
    configOutput(OUT_R_OUTPUT, "");
  }

  void process(const ProcessArgs& args) override {
  }
};

struct WtWidget : ModuleWidget {
  WtWidget(Wt* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/wt.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParam<RoundBlackKnob>(mm2px(Vec(10.16, 10.16)), module, Wt::SMOOTH_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(35.56, 10.16)), module, Wt::VOICES_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(10.16, 35.56)), module, Wt::RECT50_PARAM));
    addParam(createParam<RoundBlackKnob>(mm2px(Vec(35.56, 35.56)), module, Wt::RECT51_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 53.34)), module, Wt::TUNE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 53.34)), module, Wt::WAVE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(50.8, 53.34)), module, Wt::FM_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 67.31)), module, Wt::DETUNE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 67.31)), module, Wt::DIST1_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(50.8, 67.31)), module, Wt::DIST2_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.35, 90.17)), module, Wt::WAVE_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.415, 90.42)), module, Wt::FM_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(54.61, 90.17)), module, Wt::DIST2_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 90.42)), module, Wt::DETUNE_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.545, 90.42)), module, Wt::DIST1_ATT_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 104.14)), module, Wt::WAVE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.415, 104.14)), module, Wt::FM_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 104.14)), module, Wt::DETUNE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.545, 104.14)), module, Wt::DIST1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.61, 104.14)), module, Wt::DIST2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 116.84)), module, Wt::VOCT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.415, 116.84)), module, Wt::RESET_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.545, 116.84)), module, Wt::OUT_L_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.61, 116.84)), module, Wt::OUT_R_OUTPUT));

    // mm2px(Vec(50.8, 30.48))
    addChild(createWidget<Widget>(mm2px(Vec(5.08, 10.16))));
  }
};

Model* modelWt = createModel<Wt, WtWidget>("wt");