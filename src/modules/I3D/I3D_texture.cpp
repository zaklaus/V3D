#include "I3D_texture.h"
#include "I3D_driver.h"

// --- I3D_texture_base
I3D_texture_base::I3D_texture_base(I3D_driver* driver) : 
    _driver(driver) {
}

// --- I3D_texture
I3D_texture::I3D_texture(I3D_driver* driver) : 
    I3D_texture_base(driver) {
}

const ea::string& I3D_texture::getFileName(int i) {
    assert(i > -1 && i < 2);
    return _filenames[i];
}

const ResourceHandle I3D_texture::getTextureHandle() {
    return _textureHandle;
}

// --- I3D_animated_texture
I3D_animated_texture::I3D_animated_texture(I3D_driver* driver) :
    I3D_texture_base(driver) {
}

void I3D_animated_texture::setTextures(ea::vector<ea::shared_ptr<I3D_texture>> textures) {
    _textures = std::move(textures);
}

void I3D_animated_texture::setAnimSpeed(uint32_t delay) {
    _delay = delay;
}

const ea::string& I3D_animated_texture::getFileName(int i) {
    static ea::string _e = "";
    return _textures.empty() ? _e : _textures.front()->getFileName(i);
}

const ResourceHandle I3D_animated_texture::getTextureHandle() {
    if(_textures.empty()) return {};

    uint32_t currentRenderTime = _driver->getRenderTime();
    if(_lastRenderTime != currentRenderTime) {
        uint32_t delta = currentRenderTime - _lastRenderTime;
        _animCnt += delta;

        while(_animCnt >= _delay) {
            _animCnt -= _delay;
            ++_currentFrameIdx %= _textures.size();
        }

        _lastRenderTime = currentRenderTime;
    }

    return _textures[_currentFrameIdx]->getTextureHandle();
}