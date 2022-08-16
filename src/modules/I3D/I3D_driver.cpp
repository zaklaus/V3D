#include "I3D_driver.h"
#include "I3D_frame.h"
#include "I3D_dummy.h"
#include "I3D_camera.h"
#include "I3D_sector.h"

I3D_frame* I3D_driver::createFrame(I3D_FRAME_TYPE type) {
    switch (type) {
        case FRAME_NULL:
            return new I3D_frame(this);
        case FRAME_DUMMY: 
            return new I3D_dummy(this);
        case FRAME_CAMERA:
            return new I3D_camera(this);
        case FRAME_SECTOR: 
            return new I3D_sector(this);
        default:
            return nullptr;
    }
}

uint32_t I3D_driver::getRenderTime() {
    return 0;
}