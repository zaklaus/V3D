#pragma once
#include "I3D_frame.h"

class I3D_dummy : public I3D_frame {
public:
    I3D_dummy();
    
    void duplicate(I3D_frame* frame);
    
    void setBBox(const I3D_bbox& bbox);
    const I3D_bbox& getBBox() const;
private:
    I3D_bbox _bbox{};
};

//----------------------------

#ifdef _DEBUG
inline I3D_dummy* I3DCAST_DUMMY(I3D_frame* f){ return !f ? nullptr : f->getFrameType()!=FRAME_DUMMY ? nullptr : reinterpret_cast<I3D_dummy*>(f); }
inline const I3D_dummy* I3DCAST_CDUMMY(const I3D_frame* f){ return !f ? nullptr : f->getFrameType()!=FRAME_DUMMY ? nullptr : static_cast<const I3D_dummy*>(f); }
#else
inline I3D_dummy* I3DCAST_DUMMY(I3D_frame* f){ return reinterpret_cast<I3D_dummy*>(f); }
inline const I3D_dummy* I3DCAST_CDUMMY(const I3D_frame* f){ return static_cast<const I3D_dummy*>(f); }
#endif

//----------------------------