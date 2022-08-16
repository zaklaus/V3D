#pragma once
#include "I3D_frame.h"

class I3D_sector : public I3D_frame {
public:
    I3D_sector(I3D_driver* driver);
private:
};

//----------------------------

#ifdef _DEBUG
inline I3D_sector* I3DCAST_SECTOR(I3D_frame* f){ return !f ? nullptr : f->getFrameType()!=FRAME_SECTOR ? nullptr : reinterpret_cast<I3D_sector*>(f); }
inline const I3D_sector* I3DCAST_CSECTOR(const I3D_frame* f){ return !f ? nullptr : f->getFrameType()!=FRAME_SECTOR ? nullptr : static_cast<const I3D_sector*>(f); }
#else
inline I3D_sector* I3DCAST_SECTOR(I3D_frame* f){ return reinterpret_cast<I3D_sector*>(f); }
inline const I3D_sector* I3DCAST_CSECTOR(const I3D_frame* f){ return static_cast<const I3D_sector*>(f); }
#endif

//----------------------------