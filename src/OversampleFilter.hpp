#pragma once
#include "rack.hpp"
#define CUTOFF 0.25f

class OversampleFilter {
  public:
    void setOversample(int oversample) {
      float cutoff = CUTOFF / oversample;
      f[0].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, .51763809, 1);
      f[1].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.70710678, 1);
      f[2].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 1.9318517, 1);
    }

    float process(float x) {
      x = f[0].process(x);
      x = f[1].process(x);
      x = f[2].process(x);
      return x;
    }

  private:
    rack::dsp::TBiquadFilter<float> f[3];
};
