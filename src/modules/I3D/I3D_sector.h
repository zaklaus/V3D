#pragma once
#include "I3D_frame.h"

class I3D_sector : public I3D_frame {
public:
    I3D_sector() {
        _type = FRAME_SECTOR;
    }
private:
};