#pragma once
#include "I3D.h"

#include <cstdint>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/shared_ptr.h>
namespace ea = eastl;

#include "IDevice.h"

enum I3D_TEXTURE_FLAGS {
    TXTFLAGS_DIFFUSE = (1 << 1),  
};

// //----------------------------
//                               //create texture flags - used with I3D_CREATETEXTURE structure
// #define TEXTMAP_DIFFUSE          1        //text_name valid, specifying diffuse bitmap
// #define TEXTMAP_OPACITY          2        //text_name valid, specifying opacity bitmap
// #define TEXTMAP_EMBMMAP          4        //text_name valid, specifying env-map bumpmap bitmap
// #define TEXTMAP_CUBEMAP          8        //create cubic texture
// #define TEXTMAP_NO_SYSMEM_COPY   0x10     //do not create system-memory copy of texture (empty textures only)
// #define TEXTMAP_PROCEDURAL       0x20     //proc_name and proc_data valid, specifying procedural texture
// #define TEXTMAP_TRANSP           0x40     //texture with transparent (color-keyed) texels
// #define TEXTMAP_MIPMAP           0x100    //generate mipmap levels
// #define TEXTMAP_NOMIPMAP         0x200    //disable auto-mipmap feature
// #ifndef GL
// #define TEXTMAP_COMPRESS         0x800    //create compressed texture (if supported by hw)
// #endif
// #define TEXTMAP_USEPIXELFORMAT   0x1000   //use specified pixel format
// #define TEXTMAP_HINTDYNAMIC      0x2000   //dymanic texture
// #define TEXTMAP_TRUECOLOR        0x4000   //choose true-color pixel format, if available
// #define TEXTMAP_USE_CACHE        0x8000   //diffuse and/or opacity maps specified by C_cache, rather than by filenames
// #define TEXTMAP_RENDERTARGET     0x10000  //request texture which may be used as rendertarget
// #define TEXTMAP_NORMALMAP        0x20000  //text_name valid, specifying normal map

enum I3D_CREATETEXTURE_FLAGS {
    TXTMAP_DIFFUSE = (1 << 1),
    TXTMAP_OPACITY = (1 << 2)
};

struct I3D_CREATETEXTURE {
    uint32_t _flags{};               //TEXTMAP_ flags
    uint32_t _width{};
    uint32_t _height{};
    ea::string _diffuse{};
    ea::string _op{};
    //struct S_pixelformat *pf; requested pixel format, used with TEXTMAP_USEPIXELFORMAT
};

//----------------------------

class I3D_texture_base {
public:
    I3D_texture_base(I3D_driver* driver);
    const uint32_t getWidth() const { return _width; }
    const uint32_t getHeight() const { return _height; }
    
    void setFlags(uint32_t flags) { _flags = flags; }
    uint32_t getFlags() const { return _flags; }

    virtual const ea::string& getFileName(int i = 0) = 0;
    virtual const ResourceHandle getTextureHandle() = 0;
protected:
    I3D_driver* _driver{ nullptr };
    uint32_t _width{};
    uint32_t _height{};
    uint32_t _flags{};
};

class I3D_texture : public I3D_texture_base {
public:
    I3D_texture(I3D_driver* driver);
    const ea::string& getFileName(int i = 0) override;
    const ResourceHandle getTextureHandle() override;

    bool open(const I3D_CREATETEXTURE& params);
private:
    ea::string _filenames[2];
    ResourceHandle _textureHandle{};
};


class I3D_animated_texture : public I3D_texture_base {
public:
    I3D_animated_texture(I3D_driver* driver);
    void setTextures(ea::vector<ea::shared_ptr<I3D_texture>> textures);
    void setAnimSpeed(uint32_t delay);

    const ea::string& getFileName(int i = 0) override;
    const ResourceHandle getTextureHandle() override;
private:
    uint32_t _lastRenderTime{};
    uint32_t _animCnt{};
    uint32_t _currentFrameIdx{};
    uint32_t _delay{}; // in ms
    ea::vector<ea::shared_ptr<I3D_texture>> _textures{};
};