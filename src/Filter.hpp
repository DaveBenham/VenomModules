// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include "rack.hpp"
#define OVERSAMPLE_CUTOFF 0.25f

class OversampleFilter {
  public:
    void setOversample(int oversample) {
      float cutoff = OVERSAMPLE_CUTOFF / oversample;
      f[0].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, .51763809f, 1);
      f[1].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.70710678f, 1);
      f[2].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 1.9318517f, 1);
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

class OversampleFilter_4 {
  public:
    void setOversample(int oversample) {
      float cutoff = OVERSAMPLE_CUTOFF / oversample;
      f[0].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, .51763809f, 1);
      f[1].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.70710678f, 1);
      f[2].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 1.9318517f, 1);
    }

    rack::simd::float_4 process(rack::simd::float_4 x) {
      x = f[0].process(x);
      x = f[1].process(x);
      x = f[2].process(x);
      return x;
    }

  private:
    rack::dsp::TBiquadFilter<rack::simd::float_4> f[3];
};

class DCBlockFilter_4 {
  public:
    void init(float sampleRate){
      f[0].setCutoffFreq(2.f/sampleRate);
      f[1].setCutoffFreq(2.f/sampleRate);
      f[2].setCutoffFreq(2.f/sampleRate);
      f[3].setCutoffFreq(2.f/sampleRate);
    }
    void reset(){
      f[0].reset();
      f[1].reset();
      f[2].reset();
      f[3].reset();
    }
    rack::simd::float_4 process(rack::simd::float_4 val){
      f[0].process(val);
      f[1].process(f[0].highpass());
      f[2].process(f[1].highpass());
      f[3].process(f[2].highpass());
      return f[3].highpass();
    }

  private:
    rack::dsp::TRCFilter <rack::simd::float_4>f[4];
};
