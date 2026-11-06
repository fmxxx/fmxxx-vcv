#include "dsp/phase.hpp"
#include "plugin.hpp"

using namespace rack;
using simd::float_4;
using PDistFn = float_4 (*)(const float_4, const float_4);

struct Phase_dist : Module {
  enum ParamId {
    PD_PARAM,
    PD_TYPE_PARAM,
    PD_ATT_PARAM,
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

    configParam(PD_PARAM, -0.5f, 0.5f, 0.f, "Distort Amount", "%", 0.f, 200.f);
    configSwitch(PD_TYPE_PARAM, 0.f, 10.f, 0.f, "Distortion Type", {"Skew", "RampTri", "RampRev", "Mirror", "", "", "", "", "", "", ""});
    getParamQuantity(PD_TYPE_PARAM)->snapEnabled = true;

    configParam(PD_ATT_PARAM, -1.f, 1.f, 0.f, "Phase Distortion CV", "%", 0.f, 100.f);

    configInput(PD_INPUT, "Phase Distortion (+, Bipolar)");
    configInput(PD_TYPE_INPUT, "PD Type (Mono, overrides knob)");
    configInput(PHASE_INPUT, "Phase [0, 10) V");

    configOutput(PHASE_OUTPUT, "Phase [0, 10) V");
  }

  int controlRateCounter = 0;
  const int CONTROL_RATE_INTERVAL = 32;

  static inline const PDistFn pdTable[] = {
      skew,
      rampToTriangle,
      rampReverse,
      mirror,
  };

  enum class PDist { Skew,
                     RampTri,
                     RampRev,
                     Mirror };  // Needs to be in the same order as PDistFn

  PDistFn currentPD = skew;

  bool isActive() {
    return outputs[PHASE_OUTPUT].isConnected();
  }

  void process(const ProcessArgs& args) override {
    int channels = inputs[PHASE_INPUT].getChannels();
    outputs[PHASE_OUTPUT].setChannels(channels);

    // Get knobs
    float distKnob = params[PD_PARAM].getValue();
    float distCVAmt = params[PD_ATT_PARAM].getValue();

    if (this->isActive()) {
      // Control Rate
      if (controlRateCounter <= 0) {
        controlRateCounter = CONTROL_RATE_INTERVAL;

        // Get control rate CVs
        float typeCV = inputs[PD_TYPE_INPUT].isConnected() ? inputs[PD_TYPE_INPUT].getVoltage(0) : params[PD_TYPE_PARAM].getValue();

        int typeIndex = (int)std::round(typeCV);  // 1V per algorithm
        typeIndex = clamp(typeIndex, 0, (int)(sizeof(pdTable) / sizeof(pdTable[0])) - 1);

        currentPD = pdTable[typeIndex];
      }
      controlRateCounter--;

      // Per sample
      for (int c = 0; c < channels; c += 4) {
        float_4 p = inputs[PHASE_INPUT].getPolyVoltageSimd<float_4>(c) * 0.1f;

        float_4 distCV = inputs[PD_INPUT].getPolyVoltageSimd<float_4>(c);
        float_4 dist = distKnob + distCV * 0.1f * distCVAmt;

        // Phase distortion
        p = currentPD(p, dist + 0.5);

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
    addParam(createParamCentered<Trimpot>(mm2px(Vec(7.634, 64.76)), module, Phase_dist::PD_ATT_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Phase_dist::PD_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Phase_dist::PD_TYPE_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Phase_dist::PHASE_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Phase_dist::PHASE_OUTPUT));

    // mm2px(Vec(15.24, 15.24))
    addChild(createWidget<Widget>(mm2px(Vec(0.0, 10.16))));
  }
};

Model* modelPhase_dist = createModel<Phase_dist, Phase_distWidget>("phase_dist");