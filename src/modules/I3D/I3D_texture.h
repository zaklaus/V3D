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

enum I3D_CREATETEXTURE_FLAGS {
    TXTMAP_DIFFUSE = (1 << 1),
    TXTMAP_OPACITY = (1 << 2)
};

struct I3D_CREATETEXTURE {
    uint32_t _flags{};
    uint32_t _width{};
    uint32_t _height{};
    ea::string _diffuse{};
    ea::string _op{};
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
    virtual const Image getTextureHandle() = 0;
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
    const Image getTextureHandle() override;

    bool open(const I3D_CREATETEXTURE& params);
private:
    ea::string _filenames[2];
    Image _textureHandle{};
};

class I3D_animated_texture : public I3D_texture_base {
public:
    I3D_animated_texture(I3D_driver* driver);
    void setTextures(ea::vector<ea::shared_ptr<I3D_texture>> textures);
    void setAnimSpeed(uint32_t delay);

    const ea::string& getFileName(int i = 0) override;
    const Image getTextureHandle() override;
private:
    uint32_t _lastRenderTime{};
    uint32_t _animCnt{};
    uint32_t _currentFrameIdx{};
    uint32_t _delay{}; // in ms
    ea::vector<ea::shared_ptr<I3D_texture>> _textures{};
};