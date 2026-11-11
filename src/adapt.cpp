#include "plugin.hpp"

using simd::float_4;

struct Adapt : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    C_INPUT,
    D_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    A_OUTPUT,
    B_OUTPUT,
    C_OUTPUT,
    D_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Adapt() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(A_INPUT, "Unipolar (0V 10V)");
    configInput(B_INPUT, "Unipolar (0V 10V)");
    configInput(C_INPUT, "Bipolar (-5V +5)");
    configInput(D_INPUT, "Bipolar (-5V +5)");
    configOutput(A_OUTPUT, "Bipolar (-5V +5)");
    configOutput(B_OUTPUT, "Bipolar (-5V +5)");
    configOutput(C_OUTPUT, "Unipolar (0V 10V)");
    configOutput(D_OUTPUT, "Unipolar (0V 10V)");
  }

  void process(const ProcessArgs& args) override {
    // One loop for each I/O pair
    // UNI to BI
    if (outputs[A_OUTPUT].isConnected()) {
      int channelsA = inputs[A_INPUT].getChannels();
      outputs[A_OUTPUT].setChannels(channelsA);

      for (int c = 0; c < channelsA; c += 4) {
        float_4 v = inputs[A_INPUT].getPolyVoltageSimd<float_4>(c) - 5.f;
        outputs[A_OUTPUT].setVoltageSimd<float_4>(simd::clamp(v, -12.f, 12.f), c);
      }
    }

    if (outputs[B_OUTPUT].isConnected()) {
      int channelsB = inputs[B_INPUT].getChannels();
      outputs[B_OUTPUT].setChannels(channelsB);

      for (int c = 0; c < channelsB; c += 4) {
        float_4 v = inputs[B_INPUT].getPolyVoltageSimd<float_4>(c) - 5.f;
        outputs[B_OUTPUT].setVoltageSimd<float_4>(simd::clamp(v, -12.f, 12.f), c);
      }
    }

    // BI to UNI
    if (outputs[C_OUTPUT].isConnected()) {
      int channelsC = inputs[C_INPUT].getChannels();
      outputs[C_OUTPUT].setChannels(channelsC);

      for (int c = 0; c < channelsC; c += 4) {
        float_4 v = inputs[C_INPUT].getPolyVoltageSimd<float_4>(c) + 5.f;
        outputs[C_OUTPUT].setVoltageSimd<float_4>(simd::clamp(v, -12.f, 12.f), c);
      }
    }

    if (outputs[D_OUTPUT].isConnected()) {
      int channelsD = inputs[D_INPUT].getChannels();
      outputs[D_OUTPUT].setChannels(channelsD);

      for (int c = 0; c < channelsD; c += 4) {
        float_4 v = inputs[D_INPUT].getPolyVoltageSimd<float_4>(c) + 5.f;
        outputs[D_OUTPUT].setVoltageSimd<float_4>(simd::clamp(v, -12.f, 12.f), c);
      }
    }
  }
};

struct AdaptWidget : ModuleWidget {
  AdaptWidget(Adapt* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/adapt.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 20.32)), module, Adapt::A_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 48.26)), module, Adapt::B_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 76.2)), module, Adapt::C_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 104.14)), module, Adapt::D_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 33.02)), module, Adapt::A_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 60.96)), module, Adapt::B_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 88.9)), module, Adapt::C_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Adapt::D_OUTPUT));
  }
};

Model* modelAdapt = createModel<Adapt, AdaptWidget>("adapt");