#include "I3D_driver.h"
#include "I3D_frame.h"
#include "I3D_camera.h"
#include "I3D_sector.h"

I3D_frame* I3D_driver::createFrame(I3D_FRAME_TYPE type) {
    switch (type) {
        case FRAME_NULL:
            return new I3D_frame();
        case FRAME_CAMERA:
            return new I3D_camera();
        case FRAME_SECTOR: 
            return new I3D_sector();
        default:
            return nullptr;
    }
}