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
inline float_4 hSkew(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.f, 1.f);

  const float_4 false_branch = 1.0f - d;
  float_4 denom = simd::ifelse(phase < d, d, false_branch);

  const float_4 m = 0.5f / denom;
  const float_4 c = 0.5f - (m * d);

  const float_4 mp = m * phase;
  const float_4 true_branch2 = mp;
  const float_4 false_branch2 = mp + c;

  return simd::ifelse(phase < d, true_branch2, false_branch2);
}

inline float_4 mirror(float_4 phase, float_4 dist) {
  return phase;
}

// Mutes the phase that falls outside a sliding window (w)
// dist >= 0.5 starting from the end
// dist < 0.5 starting from the beggining
inline float_4 mute(const float_4 phase, const float_4 dist) {
  const float_4 w = phase + dist + dist;

  return simd::ifelse((w >= 1.f) & (w < 2.f), phase, 0.f);
}

// Squeezes the waveform
// dist > 0.5: squeeze toward beginning (waveform plays fast at the start)
// dist < 0.5: squeeze toward end       (waveform plays fast at the end)
inline float_4 formant(float_4 phase, float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.f, 1.f);
  const float_4 d2 = simd::pow(2.f, simd::fabs(d - 0.5) * 8.f);

  const float_4 s = simd::ifelse(d >= 0.5f, 1.f, -1.f);

  // Reflected phase:
  // s = +1 : phase
  // s = -1 : 1 - phase
  const float_4 p2 = 0.5f + s * (phase - 0.5f);

  const float_4 z = p2 * d2;
  // s = 1: z
  // s = -1 : 1 - z
  const float_4 inner = 0.5f + s * (z - 0.5f);

  return simd::ifelse(z < 1.f, inner, 0.f);
}

// Scales the phase using powers of 2, creating an octave sync effect
// dist >= 0.5 increases up to 4 octaves
// dist <= 0.5 decreases down to 4 octaves
inline float_4 powr2(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.f, 4.f);
  const float_4 d2 = d - 0.5f;

  return wrap(phase * simd::pow(2.f, d2 * 4.f));
}

// Quantizes the phase, creating a lo-fi, bitcrushed waveform
// It doesnt quantize to bitdepth directly, making the knob smoother
// This will add nyquist aliasing to a waveform read by its output
// dist > 0.51 quantizes to large values, towards a low bitdepth
// dist < 0.49 qunatizes to smaller values, creating a high frequency "shimmer", useful for glitchy sounds on waveforms with few harmonics
inline float_4 aliasQuantize(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.0f, 1.f);
  const float_4 d2 = 2.f - d - d;
  const float_4 multiplier = simd::fmax(simd::floor(d2 * 32.f), 4.f);

  const float_4 quantized = simd::round(phase * multiplier) / multiplier;

  return simd::ifelse((dist <= 0.51f) & (dist >= 0.49f), phase, quantized);
}

// Bends the waveform using a power curve
// Remaps dist from 0, 1 to b: -10, 10
// e = (1+b^2)
// b >= 0: y = phase ^ e             [towards x = 0, y = 1]
// b < 0 : y = 1 - (1 - phase) ^ e   [towards x = 1, y = 0]
inline float_4 bend(const float_4 phase, const float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.f, 1.f);
  const float_4 b = 10.f * (d - 0.5f);
  const float_4 e = 1.f + b * b;  // e = 1 + b²

  const float_4 s = simd::ifelse(b >= 0.f, 1.f, -1.f);

  // Reflect phase around 0.5 when s = -1
  const float_4 pr = 0.5f + s * (phase - 0.5f);

  // Apply the one-sided curve
  const float_4 yr = simd::pow(pr, e);  // always phase^e or (1-x)^e

  // Reflect result back
  return 0.5f + s * (yr - 0.5f);
}

// Applies an exponential curve to the signal, compressing it near 0.5
inline float_4 pinch(float_4 phase, float_4 dist) {
  const float_4 d = simd::clamp(dist, 0.f, 1.f);
  const float_4 d2 = simd::pow(2.f, 6.f * d - 3.f);  // Map dist [0,1] -> d2 [1/8, 8]

  // Clamp phase to keep pow() bases non-negative (avoid NaN from pow(neg, frac))
  const float_4 p = simd::clamp(phase, 0.f, 1.f);

  const float_4 low = 0.5f * simd::pow(2.f * p, d2);
  const float_4 high = 1.f - 0.5f * simd::pow(2.f - 2.f * p, d2);

  return simd::ifelse(p < 0.5f, low, high);
}

inline float_4 passthrough(const float_4 phase, const float_4 dist) {
  return phase;
}
