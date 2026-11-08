#include "plugin.hpp"

using simd::float_4;

struct Fract : Module {
  enum ParamId {
    NUM_PARAM,
    DENOM_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    VOCT_1_INPUT,
    VOCT_2_INPUT,
    NUM_INPUT,
    DENOM_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    VOCT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  Fract() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(NUM_PARAM, 1.f, 100.f, 1.f, "Numerator");
    configParam(DENOM_PARAM, 1.f, 100.f, 1.f, "Denominator");

    applySnap();

    configInput(VOCT_1_INPUT, "V/Oct 1");
    configInput(VOCT_2_INPUT, "V/Oct 2");
    configInput(NUM_INPUT, "Numerator Mod (Offset, 0V +10V)");
    configInput(DENOM_INPUT, "Denominator Mod (Offset, 0V +10V)");

    configOutput(VOCT_OUTPUT, "V/Oct");

    updateControlRate();
  }

  int controlRateCounter = 0;
  const int CONTROL_RATE_INTERVAL = 32;

  bool snapNum = true;
  bool snapDenom = true;

  // Knob variables, updated at control rate
  float num = 1.f;
  float denom = 1.f;

  void applySnap() {
    getParamQuantity(NUM_PARAM)->snapEnabled = snapNum;
    getParamQuantity(NUM_PARAM)->smoothEnabled = !snapNum;
    getParamQuantity(DENOM_PARAM)->snapEnabled = snapDenom;
    getParamQuantity(DENOM_PARAM)->smoothEnabled = !snapDenom;
  }

  bool isActive() {
    return outputs[VOCT_OUTPUT].isConnected();
  }

  void updateControlRate() {
    // Get knobs
    num = params[NUM_PARAM].getValue();
    denom = params[DENOM_PARAM].getValue();
  }

  void process(const ProcessArgs& args) override {
    if (this->isActive()) {
      int channels = inputs[VOCT_1_INPUT].getChannels();
      channels = std::max(inputs[VOCT_2_INPUT].getChannels(), channels);
      channels = std::max(inputs[NUM_INPUT].getChannels(), channels);
      channels = std::max(inputs[DENOM_INPUT].getChannels(), channels);
      outputs[VOCT_OUTPUT].setChannels(channels);

      // Control Rate
      if (controlRateCounter <= 0) {
        controlRateCounter = CONTROL_RATE_INTERVAL;
        updateControlRate();
      }
      controlRateCounter--;

      // Per Channel, per sample
      for (int c = 0; c < channels; c += 4) {
        // Ratio
        // 10V = 100x
        float_4 numMod = inputs[NUM_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c) * 10.f;

        float_4 denomMod = inputs[DENOM_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c) * 10.f;

        float_4 ratio = simd::clamp(num + numMod, 1.f, 100.f) /
                        simd::clamp(denom + denomMod, 1.f, 100.f);

        // Add them all
        float_4 v =
            inputs[VOCT_1_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c) +
            inputs[VOCT_2_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c) + simd::log2(ratio);

        outputs[VOCT_OUTPUT].setVoltageSimd(simd::clamp(v, -12.f, 12.f), c);
      }
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "snapNum", json_boolean(snapNum));
    json_object_set_new(rootJ, "snapDenom", json_boolean(snapDenom));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* snapNumJ = json_object_get(rootJ, "snapNum");
    if (snapNumJ) snapNum = json_boolean_value(snapNumJ);

    json_t* snapDenomJ = json_object_get(rootJ, "snapDenom");
    if (snapDenomJ) snapDenom = json_boolean_value(snapDenomJ);

    applySnap();
  }
};

struct FractWidget : ModuleWidget {
  FractWidget(Fract* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/fract.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 19.05)), module, Fract::NUM_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.02)), module, Fract::DENOM_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 64.77)), module, Fract::VOCT_1_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Fract::VOCT_2_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Fract::NUM_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Fract::DENOM_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Fract::VOCT_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Fract* module = getModule<Fract>();
    if (!module) return;

    menu->addChild(new MenuSeparator);

    menu->addChild(createBoolMenuItem("Snap numerator", "", [=]() { return module->snapNum; }, [=](int mode) {
        module->snapNum = mode;
        module->applySnap(); }));

    menu->addChild(createBoolMenuItem("Snap denominator", "", [=]() { return module->snapDenom; }, [=](int mode) {
        module->snapDenom = mode;
        module->applySnap(); }));
  }
};

Model* modelFract = createModel<Fract, FractWidget>("fract");