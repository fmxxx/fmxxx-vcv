#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

extern Model* modelTuner;
extern Model* modelPhasor;
extern Model* modelPhase_wt;
extern Model* modelPhase_dist;
extern Model* modelFract;