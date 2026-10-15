#pragma once

// Two-state Kalman filter over barometric altitude: estimates altitude and
// vertical speed together under a constant-velocity model, with unknown
// vertical acceleration treated as process noise.
//
// Tuning, in plain terms:
//   measurementNoise   how much the altitude reading jitters (m, 1 sigma).
//                      An MS5611 at full oversampling sits around 0.1-0.3 m.
//   accelerationNoise  how hard the air is allowed to push the pilot (m/s^2,
//                      1 sigma). Higher reacts faster to a thermal edge but
//                      lets more sensor noise through to the climb figure.
class VarioFilter {
public:
    explicit VarioFilter(float measurementNoise = 0.25f, float accelerationNoise = 1.0f);

    // Feeds one altitude sample taken dt seconds after the previous one.
    // The first sample (or the first after reset) only seeds the altitude.
    // A dt that is not positive, or longer than kMaxGapS, restarts the filter
    // from this sample, since a stale velocity is worse than none.
    void update(float altitude, float dt);

    void reset();

    float altitude() const { return h_; }
    float verticalSpeed() const { return v_; }
    bool isInitialized() const { return initialized_; }

    static constexpr float kMaxGapS = 1.0f;

private:
    void seed(float altitude);

    float r_;   // measurement variance (m^2)
    float qa_;  // acceleration variance (m^2/s^4)

    float h_ = 0.0f;  // altitude estimate (m)
    float v_ = 0.0f;  // vertical speed estimate (m/s)
    // Estimate covariance [[p00, p01], [p01, p11]].
    float p00_ = 0.0f, p01_ = 0.0f, p11_ = 0.0f;
    bool initialized_ = false;
};
