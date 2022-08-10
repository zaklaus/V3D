#pragma once
#include "I3D_frame.h"

enum I3D_CAM_FLAGS : uint32_t {
    CAMFLAGS_PROJ_DIRTY = (1 << 1)
};

class I3D_sector;
class I3D_camera : public I3D_frame {
public:
    I3D_camera();
    void duplicate(I3D_frame* src);
    
    const glm::mat4& getViewMatrix() { assert(!(_flags&FRMFLAGS_MAT_DIRTY)); return _matrix; }
    const glm::mat4& getProjMatrix() { assert(!(_camFlags&CAMFLAGS_PROJ_DIRTY)); return _proj; }

    void setFOV(float fov);
    float getFOV() const { return _fov; }

    void setRange(const glm::vec2& range);
    const glm::vec2& getRange() const { return _range; }

    void setCurrSector(I3D_sector* sect) { _sector = sect; }
    I3D_sector* getCurrSector() { return _sector; }

    void updateCameraMatrices(float aspectRatio);
private:
    I3D_sector* _sector{ nullptr };
    uint32_t _camFlags{};
    float _fov{};
    glm::vec2 _range{};
    glm::mat4 _proj{};
};

//----------------------------

#ifdef _DEBUG
inline I3D_camera* I3DCAST_CAMERA(I3D_frame* f){ return !f ? nullptr : f->getFrameType()!=FRAME_CAMERA ? nullptr : reinterpret_cast<I3D_camera*>(f); }
inline const I3D_camera* I3DCAST_CCAMERA(const I3D_frame* f){ return !f ? nullptr : f->getFrameType()!=FRAME_CAMERA ? nullptr : static_cast<const I3D_camera*>(f); }
#else
inline I3D_camera* I3DCAST_CAMERA(I3D_frame* f){ return reinterpret_cast<I3D_camera*>(f); }
inline const I3D_camera* I3DCAST_CCAMERA(const I3D_frame* f){ return static_cast<const I3D_camera*>(f); }
#endif

//----------------------------