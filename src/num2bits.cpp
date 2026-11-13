#include <bitset>

#include "plugin.hpp"

struct Num2bits : Module {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    X_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    P64_OUTPUT,
    P32_OUTPUT,
    P16_OUTPUT,
    P8_OUTPUT,
    P4_OUTPUT,
    P2_OUTPUT,
    P1_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Num2bits() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(X_INPUT, "Voltage to be converted (0V, 10V)");
    configOutput(P64_OUTPUT, "64s Place");
    configOutput(P32_OUTPUT, "32s Place");
    configOutput(P16_OUTPUT, "16s Place");
    configOutput(P8_OUTPUT, "8s Place");
    configOutput(P4_OUTPUT, "4s Place");
    configOutput(P2_OUTPUT, "2s Place");
    configOutput(P1_OUTPUT, "1s Place");
  }

  bool isActive() {
    return inputs[X_INPUT].isConnected();
  }

  void process(const ProcessArgs& args) override {
    if (isActive()) {
      int channels = std::max(inputs[X_INPUT].getChannels(), 1);

      outputs[P1_OUTPUT].setChannels(channels);
      outputs[P2_OUTPUT].setChannels(channels);
      outputs[P4_OUTPUT].setChannels(channels);
      outputs[P8_OUTPUT].setChannels(channels);
      outputs[P16_OUTPUT].setChannels(channels);
      outputs[P32_OUTPUT].setChannels(channels);
      outputs[P64_OUTPUT].setChannels(channels);

      // Per sample, per channel
      // No SIMD on this one
      for (int c = 0; c < channels; ++c) {
        float x = inputs[X_INPUT].getNormalPolyVoltage(0.f, c);
        x = clamp(x, 0.f, 10.f);

        float x_int_range = floor(x * 12.7);
        int8_t y = static_cast<int8_t>(x_int_range);
        std::bitset<8> bits(y);

        float p1 = bits[0] == 1 ? 10.f : 0.f;
        float p2 = bits[1] == 1 ? 10.f : 0.f;
        float p4 = bits[2] == 1 ? 10.f : 0.f;
        float p8 = bits[3] == 1 ? 10.f : 0.f;
        float p16 = bits[4] == 1 ? 10.f : 0.f;
        float p32 = bits[5] == 1 ? 10.f : 0.f;
        float p64 = bits[6] == 1 ? 10.f : 0.f;

        outputs[P1_OUTPUT].setVoltage(p1, c);
        outputs[P2_OUTPUT].setVoltage(p2, c);
        outputs[P4_OUTPUT].setVoltage(p4, c);
        outputs[P8_OUTPUT].setVoltage(p8, c);
        outputs[P16_OUTPUT].setVoltage(p16, c);
        outputs[P32_OUTPUT].setVoltage(p32, c);
        outputs[P64_OUTPUT].setVoltage(p64, c);
      }
    }
  }
};

struct Num2bitsWidget : ModuleWidget {
  Num2bitsWidget(Num2bits* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/num2bits.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 17.78)), module, Num2bits::X_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 40.64)), module, Num2bits::P64_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 53.34)), module, Num2bits::P32_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 66.04)), module, Num2bits::P16_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 78.74)), module, Num2bits::P8_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 91.44)), module, Num2bits::P4_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 104.14)), module, Num2bits::P2_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Num2bits::P1_OUTPUT));
  }
};

Model* modelNum2bits = createModel<Num2bits, Num2bitsWidget>("num2bits");