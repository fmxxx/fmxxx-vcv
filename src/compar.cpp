#include "plugin.hpp"

using simd::float_4;

struct Compar : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    EQUAL_OUTPUT,
    NOTEQUAL_OUTPUT,
    GREATER_OUTPUT,
    GREATEQ_OUTPUT,
    LESSER_OUTPUT,
    LESSEQ_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Compar() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(A_INPUT, "CV A");
    configInput(B_INPUT, "CV B");
    configOutput(EQUAL_OUTPUT, "A == B Boolean (0V, 10V)");
    configOutput(NOTEQUAL_OUTPUT, "A != B Boolean (0V, 10V)");
    configOutput(GREATER_OUTPUT, "A > B Boolean (0V, 10V)");
    configOutput(GREATEQ_OUTPUT, "A >= B Boolean (0V, 10V)");
    configOutput(LESSER_OUTPUT, "A < B Boolean (0V, 10V)");
    configOutput(LESSEQ_OUTPUT, "A <= B Boolean (0V, 10V)");
  }

  void process(const ProcessArgs& args) override {
    int channels = inputs[A_INPUT].getChannels();
    channels = std::max(inputs[B_INPUT].getChannels(), channels);
    channels = std::max(1, channels);

    outputs[EQUAL_OUTPUT].setChannels(channels);
    outputs[NOTEQUAL_OUTPUT].setChannels(channels);
    outputs[GREATER_OUTPUT].setChannels(channels);
    outputs[GREATEQ_OUTPUT].setChannels(channels);
    outputs[LESSER_OUTPUT].setChannels(channels);
    outputs[LESSEQ_OUTPUT].setChannels(channels);

    // Per Channel, per sample
    for (int c = 0; c < channels; c += 4) {
      float_4 a = inputs[A_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c);
      a = simd::round(a * 1000.f) / 1000.f;
      float_4 b = inputs[B_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c);
      b = simd::round(b * 1000.f) / 1000.f;

      float_4 equalV = simd::ifelse(a == b, 10.f, 0.f);
      float_4 notEqualV = simd::ifelse(a != b, 10.f, 0.f);
      float_4 greaterV = simd::ifelse(a > b, 10.f, 0.f);
      float_4 greateqV = simd::ifelse(a >= b, 10.f, 0.f);
      float_4 lesserV = simd::ifelse(a < b, 10.f, 0.f);
      float_4 lesseqV = simd::ifelse(a <= b, 10.f, 0.f);

      outputs[EQUAL_OUTPUT].setVoltageSimd(equalV, c);
      outputs[NOTEQUAL_OUTPUT].setVoltageSimd(notEqualV, c);
      outputs[GREATER_OUTPUT].setVoltageSimd(greaterV, c);
      outputs[GREATEQ_OUTPUT].setVoltageSimd(greateqV, c);
      outputs[LESSER_OUTPUT].setVoltageSimd(lesserV, c);
      outputs[LESSEQ_OUTPUT].setVoltageSimd(lesseqV, c);
    }
  }
};

struct ComparWidget : ModuleWidget {
  ComparWidget(Compar* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/compar.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 17.78)), module, Compar::A_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 30.48)), module, Compar::B_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 53.34)), module, Compar::EQUAL_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 66.04)), module, Compar::NOTEQUAL_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 78.74)), module, Compar::GREATER_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 91.44)), module, Compar::GREATEQ_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 104.14)), module, Compar::LESSER_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Compar::LESSEQ_OUTPUT));
  }
};

Model* modelCompar = createModel<Compar, ComparWidget>("compar");