#include "dsp/phase.hpp"
#include "dsp/wavetable.hpp"
#include "plugin.hpp"

using namespace rack;
using simd::float_4;
using PDistFn = float_4 (*)(const float_4, const float_4);

struct Phase_wt : Module {
  enum ParamId {
    WTPOS_PARAM,
    PD_PARAM,
    WAVESIZE_PARAM,
    PD_TYPE_PARAM,
    DO_MORPH_PARAM,
    LFOVCO_PARAM,
    WTPOS_CV_AMT_PARAM,
    PD_CV_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    WTPOS_INPUT,
    PD_INPUT,
    PD_TYPE_INPUT,
    PHASE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    PHASE_OUTPUT,
    AUDIO_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Phase_wt() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(WTPOS_PARAM, 0.f, 1.f, 0.f, "Wavetable Position", "%", 0.f, 100.f);
    configParam(PD_PARAM, 0.f, 1.f, 0.5f, "Phase Distortion", "%", 0.f, 200.f, -100.f);
    configParam(WAVESIZE_PARAM, 4.f, 12.f, 8.f, "Wave Size", " samples", 2.f);

    configSwitch(PD_TYPE_PARAM, 0.f, 10.f, 0.f, "Distortion Type",
                 {"Horizontal Skew", "Bend", "Pinch", "RampTri", "RampRev", "Mirror", "Mute", "Formant", "Power2", "Aliased Quantize", "Bypass"});
    configSwitch(DO_MORPH_PARAM, 0.f, 1.f, 1.f, "Morph", {"OFF", "ON"});
    configSwitch(LFOVCO_PARAM, 0.f, 1.f, 1.f, "Mode", {"LFO (No anti-aliasing)", "VCO (Anti-aliased)"});

    configParam(PD_CV_AMT_PARAM, -1.f, 1.f, 0.f, "Phase Distortion CV", "%", 0.f, 100.f);
    configParam(WTPOS_CV_AMT_PARAM, -1.f, 1.f, 0.f, "Wavetable Position CV", "%", 0.f, 100.f);

    applySnap();

    configInput(PD_INPUT, "Phase Distortion Mod (Offset, -5V +5V)");
    configInput(WTPOS_INPUT, "Wavetable Position Mod (Offset, -5V +5V)");
    configInput(PHASE_INPUT, "Wavetable Phase (0V, 10V)");
    configInput(PD_TYPE_INPUT, "PD Type (Mono, Overrides Knob, 0V +10V)");

    configOutput(PHASE_OUTPUT, "Processed Phase Output (0V +10V)");
    configOutput(AUDIO_OUTPUT, "Audio Output (-5V +5V");

    updateControlRate();
  }

  int controlRateCounter = 0;
  const int CONTROL_RATE_INTERVAL = 32;

  // Knob variables, updated at control rate
  float wtPosParam = 0.f;
  int waveSizeSamples = 256;
  bool doMorph = true;
  float wtPosCVAmtParam = 0.f;

  float pdParam = 0.f;
  bool isVCO = true;
  float pdCVAmtParam = 0.f;

  static inline const PDistFn pdTable[] = {
      hSkew,
      bend,
      pinch,
      rampToTriangle,
      rampReverse,
      mirror,
      mute,
      formant,
      powr2,
      aliasQuantize,
      passthrough,
  };

  enum class PDist { HorSkew,
                     Bend,
                     Pinch,
                     RampTri,
                     RampRev,
                     Mirror,
                     Mute,
                     Formant,
                     Power2,
                     Quantize,
                     Bypass };

  PDistFn currentPD = hSkew;

  void applySnap() {
    getParamQuantity(WAVESIZE_PARAM)->snapEnabled = true;
    getParamQuantity(PD_TYPE_PARAM)->snapEnabled = true;
  }

  bool isActive() {
    return outputs[AUDIO_OUTPUT].isConnected();
  }

  void updateControlRate() {
    // Get knobs
    wtPosParam = params[WTPOS_PARAM].getValue();
    pdParam = params[PD_PARAM].getValue();
    waveSizeSamples = std::round(std::pow(2.f, params[WAVESIZE_PARAM].getValue()));
    // PD type is handled blow, with CVs
    doMorph = params[DO_MORPH_PARAM].getValue();
    isVCO = params[LFOVCO_PARAM].getValue();
    wtPosCVAmtParam = params[WTPOS_CV_AMT_PARAM].getValue();
    pdCVAmtParam = params[PD_CV_AMT_PARAM].getValue();

    // Get control rate CVs
    float typeValue = inputs[PD_TYPE_INPUT].isConnected()
                          ? inputs[PD_TYPE_INPUT].getVoltage(0)  // Only channel 0
                          : params[PD_TYPE_PARAM].getValue();

    int typeIndex = (int)std::round(typeValue);  // 1V per algorithm
    typeIndex = clamp(typeIndex, 0, (int)(sizeof(pdTable) / sizeof(pdTable[0])) - 1);

    currentPD = pdTable[typeIndex];
  }

  void process(const ProcessArgs& args) override {
    if (this->isActive()) {
      int channels = inputs[PHASE_INPUT].getChannels();
      channels = std::max(inputs[WTPOS_INPUT].getChannels(), channels);
      channels = std::max(inputs[PD_INPUT].getChannels(), channels);
      channels = std::max(1, channels);
      outputs[AUDIO_OUTPUT].setChannels(channels);

      // Control Rate
      if (controlRateCounter <= 0) {
        controlRateCounter = CONTROL_RATE_INTERVAL;
        updateControlRate();
      }
      controlRateCounter--;

      // Per channel, per sample

      for (int c = 0; c < channels; c += 4) {
        // Phase
        float_4 p = inputs[PHASE_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f;

        float_4 pdMod = inputs[PD_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f * pdCVAmtParam;
        float_4 dist = pdParam + pdMod;

        p = currentPD(p, dist);

        // Wave
        float_4 wave = simd::sin(p * 2.f * M_PI) * 5.f;

        outputs[PHASE_OUTPUT].setVoltageSimd(simd::clamp(p * 10.f, -12.f, 12.f), c);
        outputs[AUDIO_OUTPUT].setVoltageSimd(simd::clamp(wave, -12.f, 12.f), c);
      }
    }
  }
};

struct Phase_wtWidget : ModuleWidget {
  Phase_wtWidget(Phase_wt* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/phase_wt.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 35.56)), module, Phase_wt::WTPOS_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 35.56)), module, Phase_wt::PD_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 50.8)), module, Phase_wt::WAVESIZE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.86, 50.8)), module, Phase_wt::PD_TYPE_PARAM));

    addParam(createParamCentered<CKSS>(mm2px(Vec(7.62, 66.04)), module, Phase_wt::DO_MORPH_PARAM));
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.86, 66.04)), module, Phase_wt::LFOVCO_PARAM));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(7.634, 78.73)), module, Phase_wt::WTPOS_CV_AMT_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(22.86, 78.74)), module, Phase_wt::PD_CV_AMT_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 91.44)), module, Phase_wt::WTPOS_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.86, 91.44)), module, Phase_wt::PD_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 104.14)), module, Phase_wt::PHASE_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.86, 104.14)), module, Phase_wt::PD_TYPE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phase_wt::PHASE_OUTPUT));
    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(22.86, 116.84)), module, Phase_wt::AUDIO_OUTPUT));

    // mm2px(Vec(25.4, 15.24))
    addChild(createWidget<Widget>(mm2px(Vec(2.54, 10.16))));
  }
};

Model* modelPhase_wt = createModel<Phase_wt, Phase_wtWidget>("phase_wt");