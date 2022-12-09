#pragma once
#include <cstdint>
#include <EASTL/vector.h>
#include <glm/glm.hpp>

#include "sokol_gfx.h"

using Buffer = sg_buffer;
using Image = sg_image;

//typedef sg_image     Image;
//typedef sg_shader    Shader;
//typedef sg_pipeline  Pipeline;
//typedef sg_pass      Pass;
//typedef sg_context   Context;
using ImageDesc = sg_image_desc;
using BufferDesc = sg_buffer_desc;

class IDevice {
public:
    bool init();
    void destroy();

    Image createImage(const ImageDesc& imageDesc);
    void destroyImage(Image& imageHandle);
    void bindImage(const Image& imageHandle, int samplerId);

    Buffer createBuffer(const BufferDesc& bufferDesc);
    void destroyBuffer(Buffer& bufferHnalde);
    void bindVertexBuffer(const Buffer& bufferHandle);
    void bindIndexBuffer(const Buffer& bufferHandle);

    void setViewport(const glm::ivec2& size);
    void setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj);
    void setModelMatrix(const glm::mat4& model);

    void clear(const glm::vec3& color = { 0.0f, 0.0f, 0.0f });
    void beginPass();
    void endPass();
    void present();
};

IDevice* createDevice();