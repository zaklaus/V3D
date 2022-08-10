#pragma once
#include "I3D.h"

class I3D_frame;
class I3D_driver {
public:
    I3D_frame* createFrame(I3D_FRAME_TYPE type);
};