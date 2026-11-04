#include <array>

#include "dsp/phase.hpp"
#include "plugin.hpp"

using namespace rack;
using simd::float_4;

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
    FM_AMT_ATT_PARAM,
    DIST2_ATT_PARAM,
    DETUNE_ATT_PARAM,
    DIST1_ATT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    WAVE_INPUT,
    FM_AMT_INPUT,
    DETUNE_INPUT,
    DIST1_INPUT,
    DIST2_INPUT,
    VOCT_INPUT,
    RESET_INPUT,
    FM_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    L_OUTPUT,
    R_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Wt() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(SMOOTH_PARAM, 0.f, 1.f, 1.f, "Interpolate Waves");
    configParam(VOICES_PARAM, 1.f, 7.f, 1.f, "Unison Voices");
    configParam(RECT50_PARAM, 0.f, 1.f, 0.f, "");
    configParam(RECT51_PARAM, 0.f, 1.f, 0.f, "");

    configParam(TUNE_PARAM, 32.703f, 16744.f, 261.63f, "Frequency");
    configParam(WAVE_PARAM, 0.f, 1.f, 0.f, "Wavetable Position");
    configParam(FM_PARAM, 0.f, 4.f, 0.f, "FM Amount");

    configParam(DETUNE_PARAM, 0.f, 1.f, 0.f, "Semitones");
    configParam(DIST1_PARAM, 0.f, 1.f, 0.f, "Phase Distortion 1");
    configParam(DIST2_PARAM, 0.f, 1.f, 0.f, "Phase Distortion 2");

    // ------ Attenuverters ------ //
    configParam(WAVE_ATT_PARAM, -1.f, 1.f, 0.f, "");
    configParam(FM_AMT_ATT_PARAM, -1.f, 1.f, 0.f, "");
    configParam(DIST2_ATT_PARAM, -1.f, 1.f, 0.f, "");
    configParam(DETUNE_ATT_PARAM, -1.f, 1.f, 0.f, "");
    configParam(DIST1_ATT_PARAM, -1.f, 1.f, 0.f, "");

    // ------ Mod ins ------ //
    configInput(WAVE_INPUT, "");
    configInput(FM_AMT_INPUT, "");
    configInput(DETUNE_INPUT, "");
    configInput(DIST1_INPUT, "");
    configInput(DIST2_INPUT, "");

    // ------ Attenuverters ------ //
    configInput(VOCT_INPUT, "");
    configInput(RESET_INPUT, "");
    configInput(FM_INPUT, "");

    // ------ Outputs ------ //
    configOutput(L_OUTPUT, "Left output");
    configOutput(R_OUTPUT, "Right Output");
  }

  std::array<float_4, 4> phases;  // Stores phases for each channel in blocks of 4

  bool isActive() {
    return (outputs[L_OUTPUT].isConnected() || outputs[R_OUTPUT].isConnected());
  }

  float_4 voct_to_hz(float hz, float_4 v) {
    return hz * simd::pow(2.0f, v);
  }

  void process(const ProcessArgs& args) override {
    // Set channels
    int channels = inputs[VOCT_INPUT].getChannels();
    channels = std::max(inputs[RESET_INPUT].getChannels(), channels);

    if (this->isActive()) {
      // Global
      float baseHz = params[TUNE_PARAM].getValue();
      float dist1 = params[DIST1_PARAM].getValue();
      float dist2 = params[DIST2_PARAM].getValue();
      float fmAmt = params[FM_PARAM].getValue();

      // Per Channel
      for (int c = 0; c < channels; c += 4) {
        int block = c / 4;
        // Phasor
        float_4 hz = voct_to_hz(baseHz, inputs[VOCT_INPUT].getVoltage());
        float_4 p = phasor(phases[block], hz, args.sampleRate);
        phases[block] = p;

        // Phase distortion
        p = rampToTriangle(p, dist1);
        p = skew(p, dist2);

        // Phase modulation
        float_4 fmSrc = inputs[FM_INPUT].getVoltageSimd<float_4>(c);
        p = phaseMod(p, fmSrc, fmAmt);

        // Wavetable stuff

        // TESTING
        outputs[L_OUTPUT]
            .setVoltageSimd(simd::clamp(p, -12.f, 12.f), c);
      }
    }
  };
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
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.415, 90.42)), module, Wt::FM_AMT_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(54.61, 90.17)), module, Wt::DIST2_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 90.42)), module, Wt::DETUNE_ATT_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(42.545, 90.42)), module, Wt::DIST1_ATT_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 104.14)), module, Wt::WAVE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.415, 104.14)), module, Wt::FM_AMT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 104.14)), module, Wt::DETUNE_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.545, 104.14)), module, Wt::DIST1_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.61, 104.14)), module, Wt::DIST2_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.35, 116.84)), module, Wt::VOCT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 116.84)), module, Wt::RESET_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.415, 116.84)), module, Wt::FM_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.545, 116.84)), module, Wt::L_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.61, 116.84)), module, Wt::R_OUTPUT));

    // mm2px(Vec(50.8, 30.48))
    addChild(createWidget<Widget>(mm2px(Vec(5.08, 10.16))));
  }
};

Model* modelWt = createModel<Wt, WtWidget>("wt");