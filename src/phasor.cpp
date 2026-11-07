#include <array>

#include "dsp/phase.hpp"
#include "plugin.hpp"

using namespace rack;
using simd::float_4;
using simd::int32_4;

struct Phasor : Module {
  enum ParamId {
    FREQ_PARAM,
    PM_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PM_VCA_INPUT,
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
    configParam(FREQ_PARAM, -5.f, 5.f, 0.f, "Frequency", " Hz", 2.f, dsp::FREQ_C4);
    configParam(PM_AMT_PARAM, 0.f, 1.f, 0.f, "Phase Modulation", "%", 0.f, 100.f);

    configInput(PM_VCA_INPUT, "PM Amplitude (Scale, 0V +10V)");
    configInput(PM_INPUT, "Phase Modulation (Offset, -5V +5V)");
    configInput(RESET_INPUT, "Reset Trigger (≥ 1V)");
    configInput(VOCT_INPUT, "V/Oct");
    configOutput(PHASE_OUTPUT, "Phase [0V, 10V)");
  }

  std::array<float_4, 4> phases = {float_4(0.f), float_4(0.f),
                                   float_4(0.f), float_4(0.f)};
  std::array<int32_4, 4> resetState = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

  bool isActive() {
    return outputs[PHASE_OUTPUT].isConnected();
  }

  float_4 voct_to_hz(float hz, float_4 v) {
    return hz * simd::pow(2.f, v);
  }

  float_4 resetPhase(int block, float_4 newVal, float_4 p) {
    // Create a boolean mask from the input threshold
    int32_4 newState = (newVal >= 1.0f);

    // Rising Edge Logic: New is TRUE and Old is FALSE (~old)
    int32_4 edgeMask = newState & (~resetState[block]);
    resetState[block] = newState;

    // If edgeMask is true, reset , else return phase
    return simd::ifelse(edgeMask, float_4(0.0f), p);
  }

  // Reset from right click menu
  void onReset() override {
    for (auto& p : phases) p = float_4(0.f);
    for (auto& r : resetState) r = int32_4(-1);  // 0xffffffff
    Module::onReset();
  }

  void process(const ProcessArgs& args) override {
    // Set channels
    int channels = inputs[VOCT_INPUT].getChannels();
    outputs[PHASE_OUTPUT].setChannels(channels);

    if (this->isActive()) {
      // Global, per sample
      float freqBase = dsp::FREQ_C4 * std::powf(2.f, params[FREQ_PARAM].getValue());
      float pmAmt = std::powf(params[PM_AMT_PARAM].getValue(), 2);

      // Per Channel, per sample
      for (int c = 0; c < channels; c += 4) {
        int block = c / 4;
        // Phasor
        float_4 hz = voct_to_hz(freqBase, inputs[VOCT_INPUT].getPolyVoltageSimd<float_4>(c));

        float_4 p = phasor(phases[block], hz, args.sampleRate);
        phases[block] = resetPhase(block, inputs[RESET_INPUT].getPolyVoltageSimd<float_4>(c), p);

        // Phase modulation
        float_4 pmSrc = inputs[PM_INPUT].getPolyVoltageSimd<float_4>(c);
        float_4 pmVCA = inputs[PM_VCA_INPUT].getNormalPolyVoltageSimd<float_4>(c, 10.f) * 0.1f;

        p = phaseMod(p, pmSrc * pmAmt * pmVCA);
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

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 19.05)), module, Phasor::FREQ_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.02)), module, Phasor::PM_AMT_PARAM));
    // addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 48.26)), module, Phasor::SKEW_AMT_PARAM)); // Placeholder

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 64.77)), module, Phasor::PM_VCA_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Phasor::PM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Phasor::RESET_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Phasor::VOCT_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phasor::PHASE_OUTPUT));
  }
};

Model* modelPhasor = createModel<Phasor, PhasorWidget>("phasor");