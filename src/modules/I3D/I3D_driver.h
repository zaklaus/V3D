#pragma once

class I3D_frame;
enum I3D_FRAME_TYPE;

class I3D_driver {
public:
    I3D_frame* createFrame(I3D_FRAME_TYPE type);
};