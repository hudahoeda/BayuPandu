#include "VarioFilter.h"

namespace {
// Velocity is unknown at seed time; start wide so the first few samples set it.
constexpr float kInitialVelocityVariance = 4.0f;  // (2 m/s)^2
}

VarioFilter::VarioFilter(float measurementNoise, float accelerationNoise)
    : r_(measurementNoise * measurementNoise),
      qa_(accelerationNoise * accelerationNoise) {}

void VarioFilter::reset() {
    initialized_ = false;
    h_ = v_ = 0.0f;
    p00_ = p01_ = p11_ = 0.0f;
}

void VarioFilter::seed(float altitude) {
    h_ = altitude;
    v_ = 0.0f;
    p00_ = r_;
    p01_ = 0.0f;
    p11_ = kInitialVelocityVariance;
    initialized_ = true;
}

void VarioFilter::update(float altitude, float dt) {
    if (!initialized_ || dt <= 0.0f || dt > kMaxGapS) {
        seed(altitude);
        return;
    }

    // Predict: h += v*dt, with P = F P F' + Q for a white-acceleration model.
    h_ += v_ * dt;
    float dt2 = dt * dt;
    float p00 = p00_ + dt * (2.0f * p01_ + dt * p11_) + qa_ * dt2 * dt2 / 4.0f;
    float p01 = p01_ + dt * p11_ + qa_ * dt2 * dt / 2.0f;
    float p11 = p11_ + qa_ * dt2;

    // Update with the altitude reading (H = [1 0]).
    float s = p00 + r_;
    float k0 = p00 / s;
    float k1 = p01 / s;
    float innovation = altitude - h_;
    h_ += k0 * innovation;
    v_ += k1 * innovation;

    p00_ = (1.0f - k0) * p00;
    p01_ = (1.0f - k0) * p01;
    p11_ = p11 - k1 * p01;
}
