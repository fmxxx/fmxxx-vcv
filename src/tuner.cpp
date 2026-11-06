#include "plugin.hpp"

using simd::float_4;

struct Tuner : Module {
  enum ParamId {
    SEMIS_PARAM,
    CENTS_PARAM,
    RATIO_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    VOCT_1_INPUT,
    VOCT_2_INPUT,
    RATIO_INPUT,
    CENTS_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    VOCT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  bool snapSemis = true;
  bool snapRatio = true;

  Tuner() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(SEMIS_PARAM, -48.f, 48.f, 0.f, "Semitones");
    configParam(CENTS_PARAM, -100.f, 100.f, 0.f, "Cents");
    configParam(RATIO_PARAM, 1.f, 32.f, 1.f, "Ratio");

    applySnap();

    configInput(VOCT_1_INPUT, "V/Oct 1");
    configInput(VOCT_2_INPUT, "V/Oct 2");
    configInput(RATIO_INPUT, "Ratio CV (±5V = ±32x)");
    configInput(CENTS_INPUT, "Cents CV (±5V = ±1 semi)");

    configOutput(VOCT_OUTPUT, "V/Oct");
  }

  void applySnap() {
    getParamQuantity(SEMIS_PARAM)->snapEnabled = snapSemis;
    getParamQuantity(SEMIS_PARAM)->smoothEnabled = !snapSemis;
    getParamQuantity(RATIO_PARAM)->snapEnabled = snapRatio;
    getParamQuantity(RATIO_PARAM)->smoothEnabled = !snapRatio;
  }

  bool isActive() {
    return outputs[VOCT_OUTPUT].isConnected();
  }

  void process(const ProcessArgs& args) override {
    int channels = inputs[VOCT_1_INPUT].getChannels();
    channels = std::max(inputs[VOCT_2_INPUT].getChannels(), channels);
    channels = std::max(inputs[CENTS_INPUT].getChannels(), channels);
    channels = std::max(inputs[RATIO_INPUT].getChannels(), channels);
    outputs[VOCT_OUTPUT].setChannels(channels);

    float semisVoltage = params[SEMIS_PARAM].getValue() / 12.f;
    float centsVoltage = params[CENTS_PARAM].getValue() / 1200.f;
    float ratioKnob = params[RATIO_PARAM].getValue();

    if (this->isActive()) {
      for (int c = 0; c < channels; c += 4) {
        float_4 ratio = ratioKnob + inputs[RATIO_INPUT].getPolyVoltageSimd<float_4>(c) * 6.4f;
        ratio = simd::clamp(ratio, 1.f, 32.f);

        float_4 v = inputs[VOCT_1_INPUT].getPolyVoltageSimd<float_4>(c) + inputs[VOCT_2_INPUT].getPolyVoltageSimd<float_4>(c) + inputs[CENTS_INPUT].getPolyVoltageSimd<float_4>(c) / 60.f + semisVoltage + centsVoltage + simd::log2(ratio);

        outputs[VOCT_OUTPUT].setVoltageSimd(simd::clamp(v, -12.f, 12.f), c);
      }
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "snapSemis", json_boolean(snapSemis));
    json_object_set_new(rootJ, "snapRatio", json_boolean(snapRatio));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* snapSemisJ = json_object_get(rootJ, "snapSemis");
    if (snapSemisJ) snapSemis = json_boolean_value(snapSemisJ);

    json_t* snapRatioJ = json_object_get(rootJ, "snapRatio");
    if (snapRatioJ) snapRatio = json_boolean_value(snapRatioJ);

    applySnap();
  }
};

struct TunerWidget : ModuleWidget {
  TunerWidget(Tuner* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/tuner.svg")));

    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 19.05)), module, Tuner::SEMIS_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.02)), module, Tuner::CENTS_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 46.99)), module, Tuner::RATIO_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 64.77)), module, Tuner::VOCT_1_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 77.47)), module, Tuner::VOCT_2_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 90.17)), module, Tuner::CENTS_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 102.87)), module, Tuner::RATIO_INPUT));

    addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 116.84)), module, Tuner::VOCT_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Tuner* module = getModule<Tuner>();
    if (!module) return;

    menu->addChild(new MenuSeparator);

    menu->addChild(createBoolMenuItem("Snap semitones", "", [=]() { return module->snapSemis; }, [=](int mode) {
        module->snapSemis = mode;
        module->applySnap(); }));

    menu->addChild(createBoolMenuItem("Snap ratio", "", [=]() { return module->snapRatio; }, [=](int mode) {
        module->snapRatio = mode;
        module->applySnap(); }));
  }
};

Model* modelTuner = createModel<Tuner, TunerWidget>("tuner");