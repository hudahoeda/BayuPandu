#pragma once

#include <cmath>

// Great-circle helpers on a spherical Earth; accurate to well under 1% at
// paragliding distances, which is all the display needs.
namespace geo {

constexpr double kEarthRadiusM = 6371000.0;
constexpr double kDegToRad = M_PI / 180.0;

inline double distanceM(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * kDegToRad;
    double dLon = (lon2 - lon1) * kDegToRad;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * kDegToRad) * std::cos(lat2 * kDegToRad) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    return 2.0 * kEarthRadiusM * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

// Initial bearing from point 1 to point 2, degrees clockwise from true north [0, 360).
inline double bearingDeg(double lat1, double lon1, double lat2, double lon2) {
    double phi1 = lat1 * kDegToRad, phi2 = lat2 * kDegToRad;
    double dLon = (lon2 - lon1) * kDegToRad;
    double y = std::sin(dLon) * std::cos(phi2);
    double x = std::cos(phi1) * std::sin(phi2) - std::sin(phi1) * std::cos(phi2) * std::cos(dLon);
    double deg = std::atan2(y, x) / kDegToRad;
    return deg < 0 ? deg + 360.0 : deg;
}

}  // namespace geo
