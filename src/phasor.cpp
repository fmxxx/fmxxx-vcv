#include <array>

#include "dsp/phase.hpp"
#include "plugin.hpp"

using namespace rack;
using simd::float_4;

struct Phasor : Module {
  enum ParamId {
    FREQ_PARAM,
    PM_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PM_AMP_INPUT,
    PM_INPUT,
    RESET_INPUT,
    VOCT_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    PHASE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Phasor() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(FREQ_PARAM, -6.f, 6.f, 0.f, "Frequency", " Hz", 2.f, dsp::FREQ_C4);
    configParam(PM_AMT_PARAM, 0.f, 1.f, 1.f, "PM Amount", "%", 0.f, 100.f);
    configInput(PM_AMP_INPUT, "PM VCA");
    configInput(PM_INPUT, "PM");
    configInput(RESET_INPUT, "Reset");
    configInput(VOCT_INPUT, "V/Oct");
    configOutput(PHASE_OUTPUT, "Phase");
  }

  std::array<float_4, 4> phases;  // Stores phases for each channel in blocks of 4

  bool isActive() {
    return outputs[PHASE_OUTPUT].isConnected();
  }

  float_4 voct_to_hz(float hz, float_4 v) {
    return hz * simd::pow(2.f, v);
  }

  void process(const ProcessArgs& args) override {
    // Set channels
    int channels = inputs[VOCT_INPUT].getChannels();
    outputs[PHASE_OUTPUT].setChannels(channels);

    if (this->isActive()) {
      // Global
      float freqBase = dsp::FREQ_C4 * std::powf(2.f, params[FREQ_PARAM].getValue());

      float pmAmt = std::powf(params[PM_AMT_PARAM].getValue(), 2);

      // Per Channel
      for (int c = 0; c < channels; c += 4) {
        int block = c / 4;
        // Phasor
        float_4 hz = voct_to_hz(freqBase, inputs[VOCT_INPUT].getPolyVoltageSimd<float_4>(c));

        float_4 p = phasor(phases[block], hz, args.sampleRate);
        phases[block] = p;

        // Phase modulation
        float_4 pmSrc = inputs[PM_INPUT].getPolyVoltageSimd<float_4>(c);
        p = phaseMod(p, pmSrc, pmAmt);
        p = wrap(p);

        outputs[PHASE_OUTPUT].setVoltageSimd(simd::clamp(p * 10.f, -12.f, 12.f), c);
      }
    }
  };
};

struct PhasorWidget : ModuleWidget {
  PhasorWidget(Phasor* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/phasor.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 19.05)), module, Phasor::FREQ_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.02)), module, Phasor::PM_AMT_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 64.77)), module, Phasor::PM_AMP_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Phasor::PM_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Phasor::RESET_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Phasor::VOCT_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phasor::PHASE_OUTPUT));
  }
};

Model* modelPhasor = createModel<Phasor, PhasorWidget>("phasor");