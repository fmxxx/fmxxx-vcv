#pragma once
#include <rack.hpp>

using namespace rack;
using simd::float_4;

struct Wavetable {
  std::vector<float> samples;
  size_t waveSize = 0;   // Samples in a single wave
  std::string filename;  // Name of the loaded wavetable

  bool doInterpolate;
};