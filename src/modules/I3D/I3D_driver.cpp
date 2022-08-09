#include "I3D_driver.h"
#include "I3D_frame.h"

I3D_frame* I3D_driver::createFrame(I3D_FRAME_TYPE type) {
    switch (type) {
        case FRAME_NULL:
            return new I3D_frame();
        default:
            return nullptr;
    }
}