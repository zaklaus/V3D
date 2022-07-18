#pragma once
#include <stdint.h>
#include <EASTL/vector.h>
#include <glm/glm.hpp>

enum ClearFlags : uint32_t {
    CLEAR_COLOR =   (1 << 0),
    CLEAR_DEPTH =   (1 << 1),
    CLEAR_STENCIL = (1 << 2),
};

enum class TextureFormat {
    RGB,
    RGBA,
    BGRA
};

enum class ResourceType {
    TEXTURE,
    BUFFER_INDEX,
    BUFFER_VERTEX,
    VERTEX_DECL
};

struct ResourceHandle {
    ResourceType Type;
    void* _userData{ nullptr };
};

enum VertexDeclType {
    DECLTYPE_FLOAT1     = 0,
    DECLTYPE_FLOAT2     = 1,
    DECLTYPE_FLOAT3     = 2,
    DECLTYPE_FLOAT4     = 3,
    DECLTYPE_D3DCOLOR   = 4,
    DECLTYPE_UBYTE4     = 5,
    DECLTYPE_SHORT2     = 6,
    DECLTYPE_SHORT4     = 7,
    DECLTYPE_UBYTE4N    = 8,
    DECLTYPE_SHORT2N    = 9,
    DECLTYPE_SHORT4N    = 10,
    DECLTYPE_USHORT2N   = 11,
    DECLTYPE_USHORT4N   = 12,
    DECLTYPE_UDEC3      = 13,
    DECLTYPE_DEC3N      = 14,
    DECLTYPE_FLOAT16_2  = 15,
    DECLTYPE_FLOAT16_4  = 16,
    DECLTYPE_UNUSED     = 17
};

enum VertexDeclUsage {
    DECLUSAGE_POSITION      = 0,
    DECLUSAGE_BLENDWEIGHT   = 1,
    DECLUSAGE_BLENDINDICES  = 2,
    DECLUSAGE_NORMAL        = 3,
    DECLUSAGE_PSIZE         = 4,
    DECLUSAGE_TEXCOORD      = 5,
    DECLUSAGE_TANGENT       = 6,
    DECLUSAGE_BINORMAL      = 7,
    DECLUSAGE_TESSFACTOR    = 8,
    DECLUSAGE_POSITIONT     = 9,
    DECLUSAGE_COLOR         = 10,
    DECLUSAGE_FOG           = 11,
    DECLUSAGE_DEPTH         = 12,
    DECLUSAGE_SAMPLE        = 13
};

struct VertexDeclElement {
    uint16_t Offset{};
    VertexDeclType DeclType{};
    VertexDeclUsage DeclUsage{};
    uint8_t UsageIndex{};
};

enum class BufferUsage {
    STATIC,
    DYNAMIC,
    STREAM
};

class IDevice {
public:
    virtual bool init(void* windowHandle = nullptr) = 0;
    virtual void destroy() = 0;

    //NOTE: textures
    virtual ResourceHandle createTexture(TextureFormat format, int width, int height, const void* data = nullptr, size_t size = 0, int levels = 1) = 0;
    virtual void bindTexture(const ResourceHandle& handle, int samplerId) = 0;
    virtual void clearTexture(int samplerId) = 0;

    //NOTE: buffers
    virtual ResourceHandle createVertexDeclaration(const eastl::vector<VertexDeclElement>& vertexDecl) = 0;
    virtual void setVertexDeclaration(const ResourceHandle& handle) = 0;

    virtual ResourceHandle createVertexBuffer(const void* vertices, size_t verticesCnt, size_t vertexStride, BufferUsage usage = BufferUsage::STATIC) = 0;
    virtual ResourceHandle createIndexBuffer(const uint32_t* indices, size_t indicesCnt) = 0;
    virtual void bindBuffer(const ResourceHandle& handle) = 0;

    virtual void destroyResource(ResourceHandle& handle ) = 0;

    virtual void setViewport(const glm::ivec2& size) = 0;
    virtual void setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj) = 0;
    virtual void setModelMatrix(const glm::mat4& model) = 0;

    virtual void drawPrimitives(uint32_t vertexCount, uint32_t indicesCount, uint32_t vertexOffset = 0, uint32_t indexOffset = 0) = 0;
    virtual void clear(uint32_t clearFlags, const glm::vec3& color = { 0.0f, 0.0f, 0.0f }) const = 0;
    virtual void beginScene() = 0;
    virtual void endScene() = 0;
    virtual void present() = 0;
};