#include "dsp/wavetable.hpp"
#include "plugin.hpp"

using simd::float_4;

struct Phase_wt : Module {
  enum ParamId {
    WAVE_PARAM,
    DO_MORPH_PARAM,
    PD_PARAM,
    LFO_VCO_PARAM,
    WAVESIZE_PARAM,
    UNI_BI_PARAM,
    PD_ATT_PARAM,
    WAVE_ATT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PD_MOD_INPUT,
    WAVE_MOD_INPUT,
    PHASE_INPUT,
    PD_TYPE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    AUDIO_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Phase_wt() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(WAVE_PARAM, 0.f, 1.f, 0.f, "Wavetable Position", "%", 0.f, 100.f);
    configParam(PD_PARAM, -1.f, 1.f, 0.f, "Phase Distortion Amount", "%", 0.f, 100.f);
    configParam(WAVESIZE_PARAM, 4.f, 12.f, 8.f, "Wave Length", " samples", 2.f);

    configSwitch(DO_MORPH_PARAM, 0.f, 1.f, 1.f, "Morph", {"OFF", "ON"});
    configSwitch(LFO_VCO_PARAM, 0.f, 1.f, 1.f, "Mode", {"LFO (No anti-aliasing)", "VCO (Anti-aliased)"});
    configSwitch(UNI_BI_PARAM, 0.f, 1.f, 1.f, "Polarity", {"Unipolar", "Bipolar"});

    configParam(PD_ATT_PARAM, -1.f, 1.f, 0.f, "Phase Distortion CV", "%", 0.f, 100.f);
    configParam(WAVE_ATT_PARAM, -1.f, 1.f, 0.f, "Wavetable Position CV", "%", 0.f, 100.f);

    applySnap();

    configInput(PD_MOD_INPUT, "Phase Distortion Mod (Offset, -5V +5V)");
    configInput(WAVE_MOD_INPUT, "Wavetable Position Mod (Offset, -5V +5V)");
    configInput(PHASE_INPUT, "Wavetable Phase [0V, 10V)");
    configInput(PD_TYPE_INPUT, "PD Type (Mono, Overrides Knob, 0V +10V)");
    configOutput(AUDIO_OUTPUT, "Audio Output");
  }

  void applySnap() {
    getParamQuantity(WAVESIZE_PARAM)->snapEnabled = true;
  }

  bool isActive() {
    return outputs[AUDIO_OUTPUT].isConnected();
  }

  void process(const ProcessArgs& args) override {
    if (this->isActive()) {
      int channels = inputs[PHASE_INPUT].getChannels();
      channels = std::max(1, channels);
      outputs[AUDIO_OUTPUT].setChannels(channels);

      float offset = params[UNI_BI_PARAM].getValue() >= 1.f ? 0 : 5.f;
      float wtPosBase = params[WAVE_PARAM].getValue();

      for (int c = 0; c < channels; c += 4) {
        // Phase
        float_4 p = inputs[PHASE_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f;
        // If bypass internal phaser if phase input is connected

        // Wave
        float_4 w = simd::sin(p * 2.f * M_PI) * 5.f;

        outputs[AUDIO_OUTPUT].setVoltageSimd(simd::clamp(w + offset, -12.f, 12.f), c);
      }
    }
  }
};

struct Phase_wtWidget : ModuleWidget {
  Phase_wtWidget(Phase_wt* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/phase_wt.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 35.56)), module, Phase_wt::WAVE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 49.53)), module, Phase_wt::PD_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 63.5)), module, Phase_wt::WAVESIZE_PARAM));

    addParam(createParamCentered<CKSS>(mm2px(Vec(22.86, 35.56)), module, Phase_wt::DO_MORPH_PARAM));
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.86, 49.53)), module, Phase_wt::LFO_VCO_PARAM));
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.86, 63.5)), module, Phase_wt::UNI_BI_PARAM));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(22.86, 78.74)), module, Phase_wt::PD_ATT_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(7.634, 78.74)), module, Phase_wt::WAVE_ATT_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 91.44)), module, Phase_wt::WAVE_MOD_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.86, 91.44)), module, Phase_wt::PD_MOD_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.86, 104.14)), module, Phase_wt::PD_TYPE_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phase_wt::PHASE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(22.86, 116.84)), module, Phase_wt::AUDIO_OUTPUT));

    // mm2px(Vec(25.4, 15.24))
    addChild(createWidget<Widget>(mm2px(Vec(2.54, 10.16))));
  }
};

Model* modelPhase_wt = createModel<Phase_wt, Phase_wtWidget>("phase_wt");