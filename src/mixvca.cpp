#include <array>

#include "plugin.hpp"

using simd::float_4;

struct Mixvca : Module {
  enum ParamId {
    CH1_PARAM,
    CH2_PARAM,
    CH3_PARAM,
    CH4_PARAM,
    CH5_PARAM,
    CH6_PARAM,
    CH7_PARAM,
    CH8_PARAM,
    MAIN_PARAM,
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
    MAIN_VCA_INPUT,

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
    MAIN_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Mixvca() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CH1_PARAM, 0.f, 1.f, 0.f, "Channel 1 Vol", "%", 0.f, 100.f);
    configParam(CH2_PARAM, 0.f, 1.f, 0.f, "Channel 2 Vol", "%", 0.f, 100.f);
    configParam(CH3_PARAM, 0.f, 1.f, 0.f, "Channel 3 Vol", "%", 0.f, 100.f);
    configParam(CH4_PARAM, 0.f, 1.f, 0.f, "Channel 4 Vol", "%", 0.f, 100.f);
    configParam(CH5_PARAM, 0.f, 1.f, 0.f, "Channel 5 Vol", "%", 0.f, 100.f);
    configParam(CH6_PARAM, 0.f, 1.f, 0.f, "Channel 6 Vol", "%", 0.f, 100.f);
    configParam(CH7_PARAM, 0.f, 1.f, 0.f, "Channel 7 Vol", "%", 0.f, 100.f);
    configParam(CH8_PARAM, 0.f, 1.f, 0.f, "Channel 8 Vol", "%", 0.f, 100.f);
    configParam(MAIN_PARAM, 0.f, 1.f, 0.f, "Main output Vol", "%", 0.f, 100.f);

    configInput(CH1_INPUT, "Channel 1");
    configInput(CH2_INPUT, "Channel 2");
    configInput(CH3_INPUT, "Channel 3");
    configInput(CH4_INPUT, "Channel 4");
    configInput(CH5_INPUT, "Channel 5");
    configInput(CH6_INPUT, "Channel 6");
    configInput(CH7_INPUT, "Channel 7");
    configInput(CH8_INPUT, "Channel 8");

    configInput(CH1_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH2_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH3_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH4_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH5_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH6_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH7_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(CH8_VCA_INPUT, "Amplitude (Scale, 0V +10V)");
    configInput(MAIN_VCA_INPUT, "Amplitude (Scale, 0V +10V)");

    configOutput(CH1_OUTPUT, "Channel 1");
    configOutput(CH2_OUTPUT, "Channel 2");
    configOutput(CH3_OUTPUT, "Channel 3");
    configOutput(CH4_OUTPUT, "Channel 4");
    configOutput(CH5_OUTPUT, "Channel 5");
    configOutput(CH6_OUTPUT, "Channel 6");
    configOutput(CH7_OUTPUT, "Channel 7");
    configOutput(CH8_OUTPUT, "Channel 8");
    configOutput(MAIN_OUTPUT, "Main");
  }

  void process(const ProcessArgs& args) override {
    std::array<float_4, 4> mix = {};
    int maxChannels = 1;

    // Individual channel output
    for (int i = 0; i < 8; i++) {
      const int inId = CH1_INPUT + i;
      const int outId = CH1_OUTPUT + i;
      const int pId = CH1_PARAM + i;
      const int vcaId = CH1_VCA_INPUT + i;

      if (inputs[inId].isConnected()) {
        int channels = inputs[inId].getChannels();
        channels = std::max(inputs[vcaId].getChannels(), channels);

        outputs[outId].setChannels(channels);

        if (channels > maxChannels) {
          maxChannels = channels;
        }

        const float_4 chLevelParam = params[pId].getValue();

        for (int c = 0; c < channels; c += 4) {
          float_4 chLevelMod = inputs[vcaId].getNormalPolyVoltageSimd<float_4>(10.f, c);
          chLevelMod = simd::clamp(chLevelMod, -10.f, 10.f) * 0.1f;

          const float_4 chLevel = chLevelParam * chLevelMod;
          const float_4 chIn = inputs[inId].getNormalPolyVoltageSimd<float_4>(0.f, c);
          const float_4 chOut = chIn * chLevel;

          mix[c / 4] += chOut;
          outputs[outId].setVoltageSimd<float_4>(simd::clamp(chOut, -12.f, 12.f), c);
        }
      }
    }

    // Main output
    if (outputs[MAIN_OUTPUT].isConnected()) {
      outputs[MAIN_OUTPUT].setChannels(maxChannels);
      const float_4 mainLevelParam = params[MAIN_PARAM].getValue();

      for (int c = 0; c < maxChannels; c += 4) {
        float_4 mainLevelMod = inputs[MAIN_VCA_INPUT].getNormalPolyVoltageSimd<float_4>(10.f, c);
        mainLevelMod = simd::clamp(mainLevelMod, -10.f, 10.f) * 0.1f;

        const float_4 mainLevel = mainLevelParam * mainLevelMod;
        const float_4 mainOut = mix[c / 4] * mainLevel;

        outputs[MAIN_OUTPUT].setVoltageSimd<float_4>(simd::clamp(mainOut, -12.f, 12.f), c);
      }
    }
  }
};

struct MixvcaWidget : ModuleWidget {
  MixvcaWidget(Mixvca* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/mixvca.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(27.94, 17.78)), module, Mixvca::CH1_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 30.48)), module, Mixvca::CH2_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 43.18)), module, Mixvca::CH3_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 55.88)), module, Mixvca::CH4_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 68.58)), module, Mixvca::CH5_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 81.28)), module, Mixvca::CH6_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 93.98)), module, Mixvca::CH7_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28.0, 106.68)), module, Mixvca::CH8_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(27.94, 119.38)), module, Mixvca::MAIN_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 17.78)), module, Mixvca::CH1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.78, 17.78)), module, Mixvca::CH1_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 30.48)), module, Mixvca::CH2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 30.48)), module, Mixvca::CH2_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 43.18)), module, Mixvca::CH3_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 43.18)), module, Mixvca::CH3_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 55.88)), module, Mixvca::CH4_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 55.88)), module, Mixvca::CH4_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 68.58)), module, Mixvca::CH5_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 68.58)), module, Mixvca::CH5_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 81.28)), module, Mixvca::CH6_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 81.28)), module, Mixvca::CH6_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 93.98)), module, Mixvca::CH7_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 93.98)), module, Mixvca::CH7_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.68, 106.68)), module, Mixvca::CH8_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.84, 106.68)), module, Mixvca::CH8_VCA_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.78, 119.38)), module, Mixvca::MAIN_VCA_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.1, 17.78)), module, Mixvca::CH1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 30.48)), module, Mixvca::CH2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 43.18)), module, Mixvca::CH3_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 55.88)), module, Mixvca::CH4_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 68.58)), module, Mixvca::CH5_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 81.28)), module, Mixvca::CH6_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 93.98)), module, Mixvca::CH7_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.16, 106.68)), module, Mixvca::CH8_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.1, 119.38)), module, Mixvca::MAIN_OUTPUT));
  }
};

Model* modelMixvca = createModel<Mixvca, MixvcaWidget>("mixvca");