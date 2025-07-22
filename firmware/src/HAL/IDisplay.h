#ifndef IDISPLAY_H
#define IDISPLAY_H

#include "Data/Types.h"

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual bool initialize() = 0;
    virtual void clear() = 0;
    virtual void setBrightness(uint8_t percentage) = 0;
    virtual void drawUI(const FlightData& data) = 0;
};

#endif // IDISPLAY_H
