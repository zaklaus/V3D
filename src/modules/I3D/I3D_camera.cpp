#include "I3D_camera.h"

I3D_camera::I3D_camera() : 
    I3D_frame(),
    _fov(glm::pi<float>() * 0.5f),
    _range(glm::vec2(0.1f, 1000.f)),
    _camFlags(FRMFLAGS_PROJ_DIRTY) 
{
    _type = FRAME_CAMERA;
}

void I3D_camera::setFOV(float fov) {
    _fov = fov;
    _camFlags |= FRMFLAGS_PROJ_DIRTY;
}

void I3D_camera::setRange(const glm::vec2& range) {
    _range = range;
    _camFlags |= FRMFLAGS_PROJ_DIRTY;
}

void I3D_camera::updateCameraMatrices(float aspectRatio) {
    if(!(_camFlags & FRMFLAGS_PROJ_DIRTY)) return;
    _proj = glm::perspectiveLH(_fov, aspectRatio, _range.x, _range.y);
    _camFlags &= ~FRMFLAGS_PROJ_DIRTY;
}