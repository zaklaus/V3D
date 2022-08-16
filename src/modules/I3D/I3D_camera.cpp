#include "I3D_camera.h"

I3D_camera::I3D_camera(I3D_driver* driver) : 
    I3D_frame(driver),
    _fov(glm::pi<float>() * 0.5f),
    _range(glm::vec2(0.1f, 1000.f)),
    _camFlags(CAMFLAGS_PROJ_DIRTY) 
{
    _type = FRAME_CAMERA;
}

void I3D_camera::duplicate(I3D_frame* src) {
    if(src->getFrameType() == FRAME_CAMERA) {
        const I3D_camera* cam = I3DCAST_CAMERA(src);
        _fov = cam->_fov;
        _range = cam->_range;
        _camFlags |= CAMFLAGS_PROJ_DIRTY;
    }

    return I3D_frame::duplicate(src);
}

void I3D_camera::setFOV(float fov) {
    _fov = fov;
    _camFlags |= CAMFLAGS_PROJ_DIRTY;
}

void I3D_camera::setRange(const glm::vec2& range) {
    _range = range;
    _camFlags |= CAMFLAGS_PROJ_DIRTY;
}

void I3D_camera::updateCameraMatrices(float aspectRatio) {
    if(!(_camFlags & CAMFLAGS_PROJ_DIRTY)) return;
    _proj = glm::perspectiveLH(_fov, aspectRatio, _range.x, _range.y);
    _camFlags &= ~CAMFLAGS_PROJ_DIRTY;
}