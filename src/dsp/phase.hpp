#pragma once
#include <rack.hpp>

using namespace rack;
using simd::float_4;

// These expact phase to be in the range [0, 1)

inline float_4 phasor(float_4 phase, float_4 freq, float_4 sampleRate) {
  phase += freq / sampleRate;

  phase = simd::ifelse(phase >= 1.0f, phase - 1.0f, phase);

  // Prevent negative frequencies or edge cases from breaking it
  phase = simd::ifelse(phase < 0.0f, phase + 1.0f, phase);

  return phase;
}

// Classic phase modulation (usually called FM)
inline float_4 phaseMod(const float_4 phase, const float_4 mod) {
  return phase + mod;
}

// Wraps it back into [0, 1) range
inline float_4 wrap(float_4 phase) {
  phase -= simd::floor(phase);
  return phase;
}

// ------ PHASE DISTORTION ALGORITHMS ------ //

// 0 Distort towards left
// 0.5 Unchanged
// 1 Distort towards right
// Some modules might have identical behavior from 0.5 to 0 and from 0.5 to 1

// Takes incoming ramp up phase and slants it into a triangle
// Same behavior from 0.5 to 0 and 0.5 to 1
inline float_4 rampToTriangle(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(1.0f - simd::fabs(dist - 0.5f), 0.f, 1.f);

  const float_4 cond = phase < d;
  const float_4 num = simd::ifelse(cond, phase, 1.0f - phase);
  const float_4 den = simd::fmax(simd::ifelse(cond, d, 1.0f - d), 0.00001f);

  return num / den;
}

// Takes incoming ramp up phase and slants it into a triangle then into ramp down
// Same behavior from 0.5 to 0 and 0.5 to 1
inline float_4 rampReverse(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(1.0f - simd::fabs(dist * 2.f - 1.0f), 0.f, 1.f);

  const float_4 cond = phase < d;
  const float_4 num = simd::ifelse(cond, phase, 1.0f - phase);
  const float_4 den = simd::fmax(simd::ifelse(cond, d, 1.0f - d), 0.00001f);

  return num / den;
}

//  Creates a PW type effect in any waveform
//  Adapted from ValleyAudio`s "bend" phase distortion
//  Created by Dale Johnson on 02/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
inline float_4 skew(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.f, 1.f);

  const float_4 false_branch = 1.0f - d;
  float_4 denom = simd::ifelse(phase < d, d, false_branch);

  const float_4 m = 0.5f / denom;
  const float_4 c = 0.5f - (m * d);

  const float_4 true_branch2 = (m * phase);
  const float_4 false_branch2 = (m * phase) + c;

  return simd::ifelse(phase < d, true_branch2, false_branch2);
}

inline float_4 mirror(float_4 phase, float_4 dist) {
  return phase;
}
