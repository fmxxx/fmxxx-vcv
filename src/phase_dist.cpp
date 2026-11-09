#include "dsp/phase.hpp"
#include "plugin.hpp"

using namespace rack;
using simd::float_4;
using PDistFn = float_4 (*)(const float_4, const float_4);

struct Phase_dist : Module {
  enum ParamId {
    PD_PARAM,
    PD_TYPE_PARAM,
    PD_CV_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PD_INPUT,
    PD_TYPE_INPUT,
    PHASE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    PHASE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Phase_dist() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(PD_PARAM, 0.f, 1.f, 0.5f, "Phase Distortion", "%", 0.f, 200.f, -100.f);

    configSwitch(PD_TYPE_PARAM, 0.f, 10.f, 0.f, "Distortion Type",
                 {"Horizontal Skew", "Bend", "Pinch", "RampTri", "RampRev", "Mirror", "Mute", "Formant", "Power2", "Aliased Quantize", "Bypass"});
    getParamQuantity(PD_TYPE_PARAM)->snapEnabled = true;

    configParam(PD_CV_AMT_PARAM, -1.f, 1.f, 0.f, "Phase Distortion CV", "%", 0.f, 100.f);

    configInput(PD_INPUT, "Phase Distortion Mod (Offset, -5V +5V)");
    configInput(PD_TYPE_INPUT, "PD Type (Mono, Overrides Knob, 0V +10V)");
    configInput(PHASE_INPUT, "Phase (0V, 10V)");

    configOutput(PHASE_OUTPUT, "Phase (0V, 10V)");

    updateControlRate();
  }

  int controlRateCounter = 0;
  const int CONTROL_RATE_INTERVAL = 32;

  // Knob variables, updated at control rate
  float pdParam = 0.f;
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

  bool isActive() {
    return outputs[PHASE_OUTPUT].isConnected();
  }

  void updateControlRate() {
    // Get knobs
    pdParam = params[PD_PARAM].getValue();
    pdCVAmtParam = params[PD_CV_AMT_PARAM].getValue();

    // Get control rate CVs
    float typeValue = inputs[PD_TYPE_INPUT].isConnected()
                          ? inputs[PD_TYPE_INPUT].getVoltage(0)
                          : params[PD_TYPE_PARAM].getValue();

    int typeIndex = (int)std::round(typeValue);  // 1V per algorithm
    typeIndex = clamp(typeIndex, 0, (int)(sizeof(pdTable) / sizeof(pdTable[0])) - 1);

    currentPD = pdTable[typeIndex];
  }

  void process(const ProcessArgs& args) override {
    if (this->isActive()) {
      // Set channels
      int channels = inputs[PHASE_INPUT].getChannels();
      channels = std::max(inputs[PD_INPUT].getChannels(), channels);
      channels = std::max(1, channels);
      outputs[PHASE_OUTPUT].setChannels(channels);

      // Control Rate
      if (controlRateCounter <= 0) {
        controlRateCounter = CONTROL_RATE_INTERVAL;
        updateControlRate();
      }
      controlRateCounter--;

      // Per channel, per sample
      for (int c = 0; c < channels; c += 4) {
        float_4 p = inputs[PHASE_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f;

        float_4 pdMod = inputs[PD_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f * pdCVAmtParam;
        float_4 dist = pdParam + pdMod;

        // Phase distortion
        p = currentPD(p, dist);

        outputs[PHASE_OUTPUT].setVoltageSimd(simd::clamp(p * 10.f, -12.f, 12.f), c);
      }
    }
  }
};

struct Phase_distWidget : ModuleWidget {
  Phase_distWidget(Phase_dist* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/phase_dist.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 35.56)), module, Phase_dist::PD_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 49.53)), module, Phase_dist::PD_TYPE_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(7.634, 64.76)), module, Phase_dist::PD_CV_AMT_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Phase_dist::PD_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Phase_dist::PD_TYPE_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Phase_dist::PHASE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phase_dist::PHASE_OUTPUT));

    // mm2px(Vec(15.24, 15.24))
    addChild(createWidget<Widget>(mm2px(Vec(0.0, 10.16))));
  }
};

Model* modelPhase_dist = createModel<Phase_dist, Phase_distWidget>("phase_dist");