// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#pragma once
#include "rack.hpp"

namespace Venom {

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

class HighBlockFilter {
  public:
    void setHighBlock(float cutoff, int sampleRate, int oversample) {
      cutoff /= (sampleRate * oversample);
      if (cutoff >= 0.5f)
        cutoff = 0.49f;
      f[0].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.5062f, 1);
      f[1].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.5612f, 1);
      f[2].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 0.7071f, 1);
      f[3].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 1.1013f, 1);
      f[4].setParameters(rack::dsp::TBiquadFilter<float>::LOWPASS, cutoff, 3.1970f, 1);
    }
    float process(float x) {
      for (int i=0; i<5; i++)
        x = f[i].process(x);
      return x;
    }
  private:
    rack::dsp::TBiquadFilter<float> f[5]{};
};


class HighBlockFilter_4 {
  public:
    void setHighBlock(float cutoff, int sampleRate, int oversample) {
      cutoff /= (sampleRate * oversample);
      if (cutoff >= 0.5f)
        cutoff = 0.49f;
      f[0].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.5062f, 1);
      f[1].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.5612f, 1);
      f[2].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 0.7071f, 1);
      f[3].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 1.1013f, 1);
      f[4].setParameters(rack::dsp::TBiquadFilter<rack::simd::float_4>::LOWPASS, cutoff, 3.1970f, 1);
    }
    rack::simd::float_4 process(rack::simd::float_4 x) {
      for (int i=0; i<5; i++)
        x = f[i].process(x);
      return x;
    }
  private:
    rack::dsp::TBiquadFilter<rack::simd::float_4> f[5]{};
};

class DCBlockFilter {
  public:
  
    void init(int sampleRate, int oversample = 1){
      r = 1. - 100. / static_cast<double>(sampleRate) / static_cast<double>(oversample);
    }
  
    float val() {
      return rtn;
    }
    
    float process(float x) {
      double y = static_cast<double>(x) - prevX + r * prevY;
      prevX = static_cast<double>(x);
      prevY = y;
      rtn = static_cast<float>(y);
      return rtn;
    }
  
  private:
    double prevX = 0.;
    double prevY = 0.;
    float rtn = 0.;
    double r = 1. - 100. / static_cast<double>(APP->engine->getSampleRate ());
};

class DCBlockFilter_4 {
  public:
  
    void init(int sampleRate, int oversample = 1){
      r = 1. - 100. / static_cast<double>(sampleRate) / static_cast<double>(oversample);
    }
  
    rack::simd::float_4 val() {
      return rtn;
    }
    
    rack::simd::float_4 process( rack::simd::float_4 x, int over = 1 ) {
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
    double r = 1. - 100. / static_cast<double>(APP->engine->getSampleRate ());
};

}