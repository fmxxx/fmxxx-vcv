#pragma once
#include <rack.hpp>

using namespace rack;
using simd::float_4;

// Classic phase modulation (usually called FM)
float_4 phaseMod(float_4 phase, float_4 mod, float_4 modAmt) {
  return phase + mod * modAmt;
}

// Takes incoming ramp phase [0, 1) and slants it into a triangle
// dist is the point in the wave where it goes from up to down
float_4 rampToTriangle(float_4 phase, float_4 dist) {
  // Remap dist range: 0.0 -> 1.0 and 1.0 -> 0.5
  dist = 1.0 - (0.5 * dist);

  // 1.0 -> saw, 0.5 -> tri
  float_4 up_branch = phase / dist;

  float_4 denom = simd::fmax(1.0f - dist, 0.00001f);
  float_4 down_branch = (1.0f - phase) / denom;

  phase = simd::ifelse(phase < dist, up_branch, down_branch);

  return phase;
}

//  Creates a PW type effect in any waveform
//  Adapted from ValleyAudio`s "bend" phase distortion
//  Created by Dale Johnson on 02/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
float_4 skew(float_4 phase, float_4 dist) {
  const float_4 x = (dist * 0.5) + 0.5;

  float_4 denom;

  float_4 true_branch = x;
  float_4 false_branch = 1.0f - x;
  denom = simd::ifelse(phase < x, true_branch, false_branch);

  const float_4 m = 0.5f / denom;
  const float_4 c = 0.5f - (m * x);

  float_4 true_branch2 = (m * phase);
  float_4 false_branch2 = (m * phase) + c;

  return simd::ifelse(phase < x, true_branch2, false_branch2);
}

float_4 mirror(float_4 phase, float_4 dist) {
  return phase;
}

inline float_4 phasor(float_4 phase, float_4 freq, float_4 sampleRate) {
  phase += freq / sampleRate;
  phase = simd::ifelse(phase >= 1.0f, 0.0f, phase);

  return phase;
}