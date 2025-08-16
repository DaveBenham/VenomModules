#include <array>

template <typename T>
T tanh_rational5(T x) {
  T x2 = x * x;
  return simd::ifelse(
    simd::fabs(x) >= static_cast<T>(3.f),
    simd::sgn(x),
    x * (static_cast<T>(27.f) + x2) / (static_cast<T>(27.f) + static_cast<T>(9.f) * x2)
  );
}

template <typename T>
T softClip(T x, float drive = 0) {
  x /= (9.5f - drive * 9.f);
  T x2 = x * x;
  x = simd::ifelse(
    simd::fabs(x) >= static_cast<T>(3.f),
    simd::sgn(x),
    x * (static_cast<T>(27.f) + x2) / (static_cast<T>(27.f) + static_cast<T>(9.f) * x2)
  );
  return x * static_cast<T>(10.f);
}

// fast sine calculation. modified from the Reaktor 6 core library.
// takes a [0, 1] range and folds it to a triangle on a [0, 0.5] range.
inline float sin_01(float t) {
  if (t > 1.0f) {
    t = 1.0f;
  }
  else if (t > 0.5) {
    t = 1.0f - t;
  }
  else if (t < 0.0f) {
    t = 0.0f;
  }
  t = 2.0f * t - 0.5f;
  float t2 = t * t;
  t = (((-0.540347 * t2 + 2.53566) * t2 - 5.16651) * t2 + 3.14159) * t;
  return t;
}

// Normalized Tunable Sigmoid Function: see https://dhemery.github.io/DHE-Modules/technical/sigmoid/#function
// phasor x and curve y always between -1 and 1 inclusive, shape k between -1 and 1 exclusive. Linear curve (y=x) if k=0
template <typename T1, typename T2>
T1 normSigmoid(T1 x, T2 k) {  // linear phasor x to curve
  return (x - k*x)/(k - 2.f*k*fabs(x) + 1.f);
}

template <typename T1, typename T2>
T1 invNormSigmoid(T1 y, T2 k) { // curve y to linear phasor
  return (y + k*y)/(2.f*k*fabs(y) - k + 1.f);
}