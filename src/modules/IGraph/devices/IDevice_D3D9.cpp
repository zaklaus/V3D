#include "IDevice_D3D9.h"
#include "../IDevice.h"

#include <EASTL/unordered_map.h>

#include <d3d9.h>
#define GLM_DX(X) *(D3DMATRIX*)(&X)

struct VertexBuffer_Userdata {
    LPDIRECT3DVERTEXBUFFER9 Buffer;
    size_t Stride;
};

class IDevice_D3D9 : public IDevice {
private:
    eastl::unordered_map<DeviceStates, uint32_t> stateMap;
    eastl::unordered_map<SamplerStates, uint32_t> samplerMap;
    eastl::unordered_map<SamplerValues, uint32_t> samplerValueMap;

    void initStateMap() {
        stateMap[DeviceStates::FOG] = D3DRS_FOGENABLE;
        stateMap[DeviceStates::ZBUFFER] = D3DRS_ZENABLE;
        stateMap[DeviceStates::ALPHA_BLEND] = D3DRS_ALPHABLENDENABLE;
        stateMap[DeviceStates::ALPHA_TEST] = D3DRS_ALPHATESTENABLE;
        stateMap[DeviceStates::LIGHTING] = D3DRS_LIGHTING;
        //todo
    }

    void initSamplerMap() {
        samplerMap[SamplerStates::ADDRESSU] = D3DSAMP_ADDRESSU;
        samplerMap[SamplerStates::ADDRESSV] = D3DSAMP_ADDRESSV;
        samplerMap[SamplerStates::ADDRESSW] = D3DSAMP_ADDRESSW;
        samplerMap[SamplerStates::BORDERCOLOR] = D3DSAMP_BORDERCOLOR;
        samplerMap[SamplerStates::MAGFILTER] = D3DSAMP_MAGFILTER;
        samplerMap[SamplerStates::MINFILTER] = D3DSAMP_MINFILTER;
        samplerMap[SamplerStates::MIPFILTER] = D3DSAMP_MIPFILTER;
        samplerMap[SamplerStates::MIPMAPLODBIAS] = D3DSAMP_MIPMAPLODBIAS;
        samplerMap[SamplerStates::MAXMIPLEVEL] = D3DSAMP_MAXMIPLEVEL;
        samplerMap[SamplerStates::MAXANISOTROPY] = D3DSAMP_MAXANISOTROPY;
        samplerMap[SamplerStates::SRGBTEXTURE] = D3DSAMP_SRGBTEXTURE;
        samplerMap[SamplerStates::ELEMENTINDEX] = D3DSAMP_ELEMENTINDEX;
        samplerMap[SamplerStates::DMAPOFFSET] = D3DSAMP_DMAPOFFSET;

        samplerValueMap[SAMPVAL_NONE] = D3DTEXF_NONE;
        samplerValueMap[SAMPVAL_POINT] = D3DTEXF_POINT;
        samplerValueMap[SAMPVAL_LINEAR] = D3DTEXF_LINEAR;
        samplerValueMap[SAMPVAL_ANISOTROPIC] = D3DTEXF_ANISOTROPIC;
        samplerValueMap[SAMPVAL_PYRAMIDALQUAD] = D3DTEXF_PYRAMIDALQUAD;
        samplerValueMap[SAMPVAL_GAUSSIANQUAD] = D3DTEXF_GAUSSIANQUAD;
    }

public:
    bool init(void* windowHandle) override {
        _d3d = Direct3DCreate9(D3D_SDK_VERSION);
        if(_d3d == nullptr)
            return false;

        D3DPRESENT_PARAMETERS d3dpp{};
        d3dpp.Windowed                  = TRUE;
        d3dpp.SwapEffect                = D3DSWAPEFFECT_DISCARD;
        d3dpp.EnableAutoDepthStencil    = TRUE;
        d3dpp.PresentationInterval      = D3DPRESENT_INTERVAL_ONE;
        d3dpp.AutoDepthStencilFormat    = D3DFMT_D24S8;

        if (FAILED(_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)windowHandle,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &d3dpp, &_device))) {
            return false;
        }

        initStateMap();
        initSamplerMap();

        //NOTE: setup default states
        setState(DeviceStates::ALPHA_BLEND, 0);
        setState(DeviceStates::LIGHTING, 0);
        setState(DeviceStates::ZBUFFER, 1);

        setSamplerState(0, SamplerStates::MINFILTER, SAMPVAL_LINEAR);
        setSamplerState(0, SamplerStates::MAGFILTER, SAMPVAL_LINEAR);
        setSamplerState(1, SamplerStates::MINFILTER, SAMPVAL_LINEAR);
        setSamplerState(1, SamplerStates::MAGFILTER, SAMPVAL_LINEAR);
        return true;
    }

    void destroy() override {
        if(_device != nullptr)
            _device->Release();

        if(_d3d != nullptr)
            _d3d->Release();
    }

    //NOTE: textures
    ResourceHandle createTexture(TextureFormat format, int width, int height, const void* data, size_t size, int levels) override {
        LPDIRECT3DTEXTURE9 texture{ nullptr };
        _device->CreateTexture(width, height, levels, levels == 0 ? D3DUSAGE_AUTOGENMIPMAP : 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, 0);
        if (texture == nullptr) {
            return {};
        }

        if(data != nullptr && size > 0) {
            D3DLOCKED_RECT rect{};
            if (FAILED(texture->LockRect(0, &rect, 0, D3DLOCK_DISCARD))) {

                if(texture != nullptr)
                    texture->Release();

                return {};
            }

            memcpy(rect.pBits, data, size);
            texture->UnlockRect(0);
        }

        return {
            ResourceType::TEXTURE,
            texture
        };
    }

    void destroyResource(ResourceHandle& handle) override {
        if(handle._userData == nullptr) return;

        switch(handle.Type) {
            case ResourceType::TEXTURE: {
                //TODO: here remove from vector
                (reinterpret_cast<LPDIRECT3DTEXTURE9>(handle._userData))->Release();
            } break;

            case ResourceType::BUFFER_VERTEX: {
                auto* data = reinterpret_cast<VertexBuffer_Userdata*>(handle._userData);
                assert(data->Buffer != nullptr);

                data->Buffer->Release();
                delete data;
            } break;

            case ResourceType::BUFFER_INDEX: {
                (reinterpret_cast<LPDIRECT3DINDEXBUFFER9>(handle._userData))->Release();
            } break;

            case ResourceType::VERTEX_DECL: {
                (reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9>(handle._userData))->Release();
            } break;
        }

        handle._userData = nullptr;
    }

    void bindTexture(const ResourceHandle& handle, int samplerId) override {
        assert(handle.Type == ResourceType::TEXTURE);
        if(handle._userData == nullptr) return;

        _device->SetTexture(samplerId, reinterpret_cast<LPDIRECT3DTEXTURE9>(handle._userData));
    }

    void clearTexture(int samplerId) override {
        _device->SetTexture(samplerId, nullptr);
    }

    //NOTE: buffers
    ResourceHandle createVertexDeclaration(const eastl::vector<VertexDeclElement>& vertexDecl, const ResourceHandle& vertexBuffer) override {
        if(vertexDecl.empty()) return {};
        assert(vertexBuffer.Type == ResourceType::BUFFER_VERTEX);

        eastl::vector<D3DVERTEXELEMENT9> vertexElements(vertexDecl.size() + 1);
        for(size_t i = 0; i < vertexDecl.size(); i++) {
            D3DVERTEXELEMENT9* elem = &vertexElements[i];
            VertexDeclElement vsElem = vertexDecl[i];
            elem->Stream        = 0;
            elem->Offset        = vsElem.Offset;
            elem->Type          = vsElem.DeclType;
            elem->Usage         = vsElem.DeclUsage;
            elem->UsageIndex    = vsElem.UsageIndex;
            elem->Method        = D3DDECLMETHOD_DEFAULT;
        }

        vertexElements[vertexDecl.size()] = D3DDECL_END();

        LPDIRECT3DVERTEXDECLARATION9 vsDecl{ nullptr };
        if(FAILED(_device->CreateVertexDeclaration(vertexElements.data(),  &vsDecl))) {
            return {};
        }

        return {
            ResourceType::VERTEX_DECL,
            vsDecl
        };
    }

    void setVertexDeclaration(const ResourceHandle& handle) override {
        assert(handle.Type == ResourceType::VERTEX_DECL);
        if(handle._userData == nullptr) return;

        _device->SetVertexDeclaration(reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9>(handle._userData));
    }

    ResourceHandle createVertexBuffer(const void* vertices, size_t verticesCnt, size_t vertexStride, BufferUsage usage) override {
        assert(vertexStride > 0 && verticesCnt > 0);
        size_t verticesSize = verticesCnt * vertexStride;
        LPDIRECT3DVERTEXBUFFER9 vbuffer{ nullptr };
        if (FAILED(_device->CreateVertexBuffer(verticesSize,
            0,
            0,
            D3DPOOL_DEFAULT, &vbuffer, nullptr))) {
            return {};
        }

        if(vertices != nullptr) {
            void* verticesBuffer{ nullptr };
            if (FAILED(vbuffer->Lock(0, verticesSize, (void**)&verticesBuffer, 0))) {
                if (vbuffer)
                    vbuffer->Release();
                return {};
            }

            memcpy(verticesBuffer, vertices, verticesSize);
            vbuffer->Unlock();
        }

        VertexBuffer_Userdata* bufferUserData = new VertexBuffer_Userdata();
        bufferUserData->Buffer = vbuffer;
        bufferUserData->Stride = vertexStride;

        return {
            ResourceType::BUFFER_VERTEX,
            bufferUserData
        };
    }

    ResourceHandle createIndexBuffer(const uint32_t* indices, size_t indicesCnt) override {
        assert(indicesCnt > 0);
        size_t indicesSize = indicesCnt * sizeof(uint32_t);

        LPDIRECT3DINDEXBUFFER9 ibuffer{ nullptr };
        if (FAILED(_device->CreateIndexBuffer(indicesSize,
            0, D3DFMT_INDEX32, D3DPOOL_DEFAULT,
            &ibuffer, NULL))) {
            return {};
        }

        if(indices != nullptr) {
            void* indicesBuffer{ nullptr };
            if (FAILED(ibuffer->Lock(0, indicesSize, (void**)&indicesBuffer, 0))) {
                if (ibuffer)
                    ibuffer->Release();
                return {};
            }

            memcpy(indicesBuffer, (const void*)indices, indicesSize);
            ibuffer->Unlock();

        }

        return {
            ResourceType::BUFFER_INDEX,
            ibuffer
        };
    }

    void bindBuffer(const ResourceHandle& handle) override {
        if(handle._userData == nullptr) return;
        assert(handle.Type == ResourceType::BUFFER_INDEX || handle.Type == ResourceType::BUFFER_VERTEX);

        if(handle.Type == ResourceType::BUFFER_VERTEX) {
            auto* data = reinterpret_cast<VertexBuffer_Userdata*>(handle._userData);
            assert(data->Buffer != nullptr);
            _device->SetStreamSource(0, data->Buffer, 0, data->Stride);
        } else {
            _device->SetIndices(reinterpret_cast<LPDIRECT3DINDEXBUFFER9>(handle._userData));
        }
    }

    void setViewport(const glm::ivec2& size) override {
        D3DVIEWPORT9 viewport{};
        viewport.X = 0;
        viewport.Y = 0;
        viewport.Width = size.x;
        viewport.Height = size.y;
        _device->SetViewport(&viewport);
    }

    void setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj) override {
        auto p = GLM_DX(proj);
        _device->SetTransform(D3DTS_PROJECTION, &p);

        auto v = GLM_DX(view);
        _device->SetTransform(D3DTS_VIEW, &v);
    }

    void setModelMatrix(const glm::mat4& model) override {
        auto m = GLM_DX(model);
        _device->SetTransform(D3DTS_WORLD, &m);
    }

    void setState(DeviceStates state, uint32_t value) override {
        _device->SetRenderState(static_cast<D3DRENDERSTATETYPE>(stateMap[state]), value);
    }

    uint32_t getState(DeviceStates state) override {
        DWORD value{0};
        _device->GetRenderState(static_cast<D3DRENDERSTATETYPE>(stateMap[state]), &value);
        return value;
    }

    void setSamplerState(uint32_t slot, SamplerStates state, uint32_t value) override {
        uint32_t value_ = value;
        if (samplerValueMap.find(static_cast<SamplerValues>(value)) != samplerValueMap.end()) {
            value_ = samplerValueMap[static_cast<SamplerValues>(value)];
        }
        _device->SetSamplerState(slot, static_cast<D3DSAMPLERSTATETYPE>(samplerMap[state]), value_);
    }

    uint32_t getSamplerState(uint32_t slot, SamplerStates state) override {
        DWORD value{0};
        _device->GetSamplerState(slot, static_cast<D3DSAMPLERSTATETYPE>(samplerMap[state]), &value);
        return value;
    }

    void drawPrimitives(uint32_t vertexCount, uint32_t indicesCount, uint32_t vertexOffset = 0, uint32_t indexOffset = 0) override {
        _device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vertexOffset, 0, vertexCount, indexOffset, indicesCount / 3);
    }

    void clear(uint32_t clearFlags, const glm::vec3& color = { 0.0f, 0.0f, 0.0f }) const override {
        DWORD flags{};
        if(clearFlags & CLEAR_COLOR)
            flags |= D3DCLEAR_TARGET;

        if(clearFlags & CLEAR_DEPTH)
            flags |= D3DCLEAR_ZBUFFER;

        if( clearFlags& CLEAR_STENCIL)
            flags |= D3DCLEAR_STENCIL;

        _device->Clear(0,
        NULL,
        flags,
        D3DCOLOR_XRGB(uint32_t(color.x * 255.0), uint32_t(color.y * 255.0), uint32_t(color.z * 255.0)),
        1.0f, 0);
    }

    void beginScene() override {
        _device->BeginScene();
    }

    void endScene() override {
        _device->EndScene();
    }

    void present() override {
        _device->Present(0, 0, 0, 0);
    }
private:
    LPDIRECT3D9 _d3d{ nullptr };
    LPDIRECT3DDEVICE9 _device{ nullptr};
};

IDevice* createDeviceD3D9() {
    return new IDevice_D3D9();
}