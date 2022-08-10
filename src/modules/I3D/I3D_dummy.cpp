#include "I3D_dummy.h"

I3D_dummy::I3D_dummy() {
    I3D_frame::I3D_frame();
    _type = FRAME_DUMMY;
    _bbox.Invalidate();
}

void I3D_dummy::duplicate(I3D_frame* src) {
    if(src->getFrameType() == FRAME_DUMMY) {
        I3D_dummy* dummy = I3DCAST_DUMMY(src);
        _bbox = dummy->_bbox;
    }

    return I3D_frame::duplicate(src);   
}

void I3D_dummy::setBBox(const I3D_bbox& bbox) {
    _bbox = bbox;
}

const I3D_bbox& I3D_dummy::getBBox() const {
    return _bbox;
}