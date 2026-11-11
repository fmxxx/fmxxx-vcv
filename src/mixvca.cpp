#include "plugin.hpp"

using simd::float_4;

struct Mixvca : Module {
  enum ParamId {
    MODE_PARAM,
    CH1_PARAM,
    CH2_PARAM,
    CH3_PARAM,
    CH4_PARAM,
    CH5_PARAM,
    CH6_PARAM,
    CH7_PARAM,
    CH8_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CH1_INPUT,
    CH2_INPUT,
    CH3_INPUT,
    CH4_INPUT,
    CH5_INPUT,
    CH6_INPUT,
    CH7_INPUT,
    CH8_INPUT,
    CH1_VCA_INPUT,
    CH2_VCA_INPUT,
    CH3_VCA_INPUT,
    CH4_VCA_INPUT,
    CH5_VCA_INPUT,
    CH6_VCA_INPUT,
    CH7_VCA_INPUT,
    CH8_VCA_INPUT,

    INPUTS_LEN
  };
  enum OutputId {
    CH1_OUTPUT,
    CH2_OUTPUT,
    CH3_OUTPUT,
    CH4_OUTPUT,
    CH5_OUTPUT,
    CH6_OUTPUT,
    CH7_OUTPUT,
    CH8_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Mixvca() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(MODE_PARAM, 0.f, 1.f, 1.f, "Mode", {"Control", "Audio"});
    configParam(CH1_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH2_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH3_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH4_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH5_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH6_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH7_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configParam(CH8_PARAM, -1.f, 1.f, 0.f, "", "%", 0.f, 100.f);
    configInput(CH1_INPUT, "Channel 1");
    configInput(CH1_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH2_INPUT, "");
    configInput(CH2_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH3_INPUT, "");
    configInput(CH3_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH4_INPUT, "");
    configInput(CH4_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH5_INPUT, "");
    configInput(CH5_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH6_INPUT, "");
    configInput(CH6_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH7_INPUT, "");
    configInput(CH7_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH8_INPUT, "");
    configInput(CH8_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configOutput(CH1_OUTPUT, "");
    configOutput(CH2_OUTPUT, "");
    configOutput(CH3_OUTPUT, "");
    configOutput(CH4_OUTPUT, "");
    configOutput(CH5_OUTPUT, "");
    configOutput(CH6_OUTPUT, "");
    configOutput(CH7_OUTPUT, "");
    configOutput(CH8_OUTPUT, "");
  }

  void process(const ProcessArgs& args) override {
    for (int i = 0; i < 8; i++) {
      const int inId = CH1_INPUT + i;
      const int outId = CH1_OUTPUT + i;
      const int pId = CH1_PARAM + i;
      const int vcaId = CH1_VCA_INPUT + i;

      if (!outputs[outId].isConnected()) continue;

      const int channels = inputs[inId].getChannels();
      outputs[outId].setChannels(channels);

      const float_4 chLevelParam = params[pId].getValue();

      for (int c = 0; c < channels; c += 4) {
        float_4 chLevelMod = inputs[vcaId].getNormalPolyVoltageSimd<float_4>(10.f, c);
        chLevelMod = simd::clamp(chLevelMod, -10.f, 10.f) * 0.1f;

        const float_4 chLevel = chLevelParam * chLevelMod;
        const float_4 chIn = inputs[inId].getNormalPolyVoltageSimd<float_4>(0.f, c);
        const float_4 chOut = chIn * chLevel;

        outputs[outId].setVoltageSimd<float_4>(chOut, c);
      }
    }
  }
};

struct MixvcaWidget : ModuleWidget {
  MixvcaWidget(Mixvca* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/mixvca.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<CKSS>(mm2px(Vec(22.86, 15.24)), module, Mixvca::MODE_PARAM));

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(27.94, 27.94)), module, Mixvca::CH1_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 40.64)), module, Mixvca::CH2_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 53.34)), module, Mixvca::CH3_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 66.04)), module, Mixvca::CH4_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 78.74)), module, Mixvca::CH5_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 91.44)), module, Mixvca::CH6_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 104.14)), module, Mixvca::CH7_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 116.84)), module, Mixvca::CH8_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 27.94)), module, Mixvca::CH1_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.78, 27.94)), module, Mixvca::CH1_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 40.64)), module, Mixvca::CH2_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 40.64)), module, Mixvca::CH2_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 53.34)), module, Mixvca::CH3_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 53.34)), module, Mixvca::CH3_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 66.04)), module, Mixvca::CH4_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 66.04)), module, Mixvca::CH4_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 78.74)), module, Mixvca::CH5_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 78.74)), module, Mixvca::CH5_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 91.44)), module, Mixvca::CH6_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 91.44)), module, Mixvca::CH6_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 104.14)), module, Mixvca::CH7_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 104.14)), module, Mixvca::CH7_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.68, 116.84)), module, Mixvca::CH8_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(17.84, 116.84)), module, Mixvca::CH8_VCA_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.1, 27.94)), module, Mixvca::CH1_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 40.64)), module, Mixvca::CH2_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 53.34)), module, Mixvca::CH3_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 66.04)), module, Mixvca::CH4_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 78.74)), module, Mixvca::CH5_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 91.44)), module, Mixvca::CH6_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 104.14)), module, Mixvca::CH7_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.16, 116.84)), module, Mixvca::CH8_OUTPUT));
  }
};

Model* modelMixvca = createModel<Mixvca, MixvcaWidget>("mixvca");