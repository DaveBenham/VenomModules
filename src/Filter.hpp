// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include "rack.hpp"

class OversampleFilter {
  public:
    int stages = 3;
    void setOversample(int oversample, int stageCnt = 3) {
      stages = stageCnt;
      float cutoff = 1.f / oversample;
      switch (stages) {
        case 3:
          cutoff *= 0.25f;
          f[0].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, .51763809f, 1);
          f[1].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.70710678f, 1);
          f[2].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 1.9318517f, 1);
          break;
        case 4:
          cutoff *= 0.4f;
          f[0].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.5098f, 1);
          f[1].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.6013f, 1);
          f[2].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.9000f, 1);
          f[3].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 2.5268f, 1);
          break;
        default: // 5
          cutoff *= 0.4;
          f[0].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.5062f, 1);
          f[1].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.5612f, 1);
          f[2].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.7071f, 1);
          f[3].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 1.1013f, 1);
          f[4].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 3.1970f, 1);
      }
    }

    float process(float x) {
      for (int i=0; i<stages; i++)
        x = f[i].process(x);
      return x;
    }

  private:
    rack::dsp::TBiquadFilter<float> f[5]{};
};

class OversampleFilter_4 {
  public:
    int stages = 3;
    void setOversample(int oversample, int stageCnt = 3) {
      stages = stageCnt;
      float cutoff = 1.f / oversample;
      switch (stages) {
        case 3:
          cutoff *= 0.25f;
          f[0].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.51763809f, 1);
          f[1].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.70710678f, 1);
          f[2].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 1.9318517f, 1);
          break;
        case 4:
          cutoff *= 0.4f;
          f[0].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.5098f, 1);
          f[1].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.6013f, 1);
          f[2].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.9000f, 1);
          f[3].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 2.5268f, 1);
          break;
        default: // 5
          cutoff *= 0.4;
          f[0].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.5062f, 1);
          f[1].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.5612f, 1);
          f[2].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.7071f, 1);
          f[3].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 1.1013f, 1);
          f[4].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 3.1970f, 1);
      }
    }

    rack::simd::float_4 process(rack::simd::float_4 x) {
      for (int i=0; i<stages; i++)
        x = f[i].process(x);
      return x;
    }

  private:
    rack::dsp::TBiquadFilter<rack::simd::float_4> f[5]{};
};

/*
class DCBlockFilter_4 {
// This version is mysteriously not working on some machines
// Be sure to uncomment `, oversample` in process calls if reverting to this version
  public:
    rack::simd::float_4 val() {
      return prevY;
    }
    
    rack::simd::float_4 process( rack::simd::float_4 x, int over = 1 ) {
      float r = 1.f - 250.f / rack::settings::sampleRate / static_cast<float>(over);
      rack::simd::float_4 y = x - prevX + static_cast<rack::simd::float_4>(r) * prevY;
      prevX = x;
      prevY = y;
      return y;
    }
  
  private:
    rack::simd::float_4 prevX = rack::simd::float_4::zero();
    rack::simd::float_4 prevY = rack::simd::float_4::zero();
};
*/

/*
class DCBlockFilter_4 {
// This version is mysteriously not working on some machines
// But I really like the behavior when it does.
//  - Minimal bass attenuation
//  - Consistent at all sample rates and oversampling rates
//  - Excellent DC offset filtering
// Be sure to uncomment `, oversample` in process calls if reverting to this version
  public:
    rack::simd::float_4 val() {
      return rtn;
    }
    
    rack::simd::float_4 process( rack::simd::float_4 x, int over = 1 ) {
      double r = 1. - 100. / static_cast<double>(rack::settings::sampleRate) / static_cast<double>(over);
      for (int i=0; i<4; i++){
        double y = static_cast<double>(x[i]) - prevX[i] + r * prevY[i];
        prevX[i] = static_cast<double>(x[i]);
        prevY[i] = y;
        rtn[i] = static_cast<float>(y);
      }
      return rtn;
    }
  
  private:
    double prevX[4]{};
    double prevY[4]{};
    rack::simd::float_4 rtn = rack::simd::float_4::zero();
};
*/

class DCBlockFilter_4 {
// Naive version that doesn't adjust for sample rate or oversampling, so cutoff is not consistent
// Also attenuates bass tones too much as sample rate and/or oversampling rate rises
  public:
    rack::simd::float_4 val = rack::simd::float_4::zero();
    
    rack::simd::float_4 process( rack::simd::float_4 x ) {
      val = x - prevX + static_cast<rack::simd::float_4>(0.999f) * val;
      prevX = x;
      return val;
    }
  
  private:
    rack::simd::float_4 prevX = rack::simd::float_4::zero();
};