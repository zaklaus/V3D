#include "I3D_sector.h"

I3D_sector::I3D_sector(I3D_driver* driver) : 
    I3D_frame(driver)
{
    _type = FRAME_SECTOR;
}