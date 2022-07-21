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
    LPDIRECT3D9 _d3d{ nullptr };
    LPDIRECT3DDEVICE9 _device{ nullptr};

    eastl::unordered_map<DeviceStates, uint32_t> stateMap;
    eastl::unordered_map<SamplerStates, uint32_t> samplerMap;
    eastl::unordered_map<SamplerValues, uint32_t> samplerValueMap;

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
        setState(DeviceStates::ALPHABLENDENABLE, 0);
        setState(DeviceStates::LIGHTING, 0);
        setState(DeviceStates::ZENABLE, 1);

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
            D3DPOOL_MANAGED, &vbuffer, nullptr))) {
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
    void initStateMap() {
        stateMap[DeviceStates::ZENABLE] = D3DRS_ZENABLE;
        stateMap[DeviceStates::FILLMODE] = D3DRS_FILLMODE;
        stateMap[DeviceStates::SHADEMODE] = D3DRS_SHADEMODE;
        stateMap[DeviceStates::ZWRITEENABLE] = D3DRS_ZWRITEENABLE;
        stateMap[DeviceStates::ALPHATESTENABLE] = D3DRS_ALPHATESTENABLE;
        stateMap[DeviceStates::LASTPIXEL] = D3DRS_LASTPIXEL;
        stateMap[DeviceStates::SRCBLEND] = D3DRS_SRCBLEND;
        stateMap[DeviceStates::DESTBLEND] = D3DRS_DESTBLEND;
        stateMap[DeviceStates::CULLMODE] = D3DRS_CULLMODE;
        stateMap[DeviceStates::ZFUNC] = D3DRS_ZFUNC;
        stateMap[DeviceStates::ALPHAREF] = D3DRS_ALPHAREF;
        stateMap[DeviceStates::ALPHAFUNC] = D3DRS_ALPHAFUNC;
        stateMap[DeviceStates::DITHERENABLE] = D3DRS_DITHERENABLE;
        stateMap[DeviceStates::ALPHABLENDENABLE] = D3DRS_ALPHABLENDENABLE;
        stateMap[DeviceStates::FOGENABLE] = D3DRS_FOGENABLE;
        stateMap[DeviceStates::SPECULARENABLE] = D3DRS_SPECULARENABLE;
        stateMap[DeviceStates::FOGCOLOR] = D3DRS_FOGCOLOR;
        stateMap[DeviceStates::FOGTABLEMODE] = D3DRS_FOGTABLEMODE;
        stateMap[DeviceStates::FOGSTART] = D3DRS_FOGSTART;
        stateMap[DeviceStates::FOGEND] = D3DRS_FOGEND;
        stateMap[DeviceStates::FOGDENSITY] = D3DRS_FOGDENSITY;
        stateMap[DeviceStates::RANGEFOGENABLE] = D3DRS_RANGEFOGENABLE;
        stateMap[DeviceStates::STENCILENABLE] = D3DRS_STENCILENABLE;
        stateMap[DeviceStates::STENCILFAIL] = D3DRS_STENCILFAIL;
        stateMap[DeviceStates::STENCILZFAIL] = D3DRS_STENCILZFAIL;
        stateMap[DeviceStates::STENCILPASS] = D3DRS_STENCILPASS;
        stateMap[DeviceStates::STENCILFUNC] = D3DRS_STENCILFUNC;
        stateMap[DeviceStates::STENCILREF] = D3DRS_STENCILREF;
        stateMap[DeviceStates::STENCILMASK] = D3DRS_STENCILMASK;
        stateMap[DeviceStates::STENCILWRITEMASK] = D3DRS_STENCILWRITEMASK;
        stateMap[DeviceStates::TEXTUREFACTOR] = D3DRS_TEXTUREFACTOR;
        stateMap[DeviceStates::WRAP0] = D3DRS_WRAP0;
        stateMap[DeviceStates::WRAP1] = D3DRS_WRAP1;
        stateMap[DeviceStates::WRAP2] = D3DRS_WRAP2;
        stateMap[DeviceStates::WRAP3] = D3DRS_WRAP3;
        stateMap[DeviceStates::WRAP4] = D3DRS_WRAP4;
        stateMap[DeviceStates::WRAP5] = D3DRS_WRAP5;
        stateMap[DeviceStates::WRAP6] = D3DRS_WRAP6;
        stateMap[DeviceStates::WRAP7] = D3DRS_WRAP7;
        stateMap[DeviceStates::CLIPPING] = D3DRS_CLIPPING;
        stateMap[DeviceStates::LIGHTING] = D3DRS_LIGHTING;
        stateMap[DeviceStates::AMBIENT] = D3DRS_AMBIENT;
        stateMap[DeviceStates::FOGVERTEXMODE] = D3DRS_FOGVERTEXMODE;
        stateMap[DeviceStates::COLORVERTEX] = D3DRS_COLORVERTEX;
        stateMap[DeviceStates::LOCALVIEWER] = D3DRS_LOCALVIEWER;
        stateMap[DeviceStates::NORMALIZENORMALS] = D3DRS_NORMALIZENORMALS;
        stateMap[DeviceStates::DIFFUSEMATERIALSOURCE] = D3DRS_DIFFUSEMATERIALSOURCE;
        stateMap[DeviceStates::SPECULARMATERIALSOURCE] = D3DRS_SPECULARMATERIALSOURCE;
        stateMap[DeviceStates::AMBIENTMATERIALSOURCE] = D3DRS_AMBIENTMATERIALSOURCE;
        stateMap[DeviceStates::EMISSIVEMATERIALSOURCE] = D3DRS_EMISSIVEMATERIALSOURCE;
        stateMap[DeviceStates::VERTEXBLEND] = D3DRS_VERTEXBLEND;
        stateMap[DeviceStates::CLIPPLANEENABLE] = D3DRS_CLIPPLANEENABLE;
        stateMap[DeviceStates::POINTSIZE] = D3DRS_POINTSIZE;
        stateMap[DeviceStates::POINTSIZE_MIN] = D3DRS_POINTSIZE_MIN;
        stateMap[DeviceStates::POINTSPRITEENABLE] = D3DRS_POINTSPRITEENABLE;
        stateMap[DeviceStates::POINTSCALEENABLE] = D3DRS_POINTSCALEENABLE;
        stateMap[DeviceStates::POINTSCALE_A] = D3DRS_POINTSCALE_A;
        stateMap[DeviceStates::POINTSCALE_B] = D3DRS_POINTSCALE_B;
        stateMap[DeviceStates::POINTSCALE_C] = D3DRS_POINTSCALE_C;
        stateMap[DeviceStates::MULTISAMPLEANTIALIAS] = D3DRS_MULTISAMPLEANTIALIAS;
        stateMap[DeviceStates::MULTISAMPLEMASK] = D3DRS_MULTISAMPLEMASK;
        stateMap[DeviceStates::PATCHEDGESTYLE] = D3DRS_PATCHEDGESTYLE;
        stateMap[DeviceStates::DEBUGMONITORTOKEN] = D3DRS_DEBUGMONITORTOKEN;
        stateMap[DeviceStates::POINTSIZE_MAX] = D3DRS_POINTSIZE_MAX;
        stateMap[DeviceStates::INDEXEDVERTEXBLENDENABLE] = D3DRS_INDEXEDVERTEXBLENDENABLE;
        stateMap[DeviceStates::COLORWRITEENABLE] = D3DRS_COLORWRITEENABLE;
        stateMap[DeviceStates::TWEENFACTOR] = D3DRS_TWEENFACTOR;
        stateMap[DeviceStates::BLENDOP] = D3DRS_BLENDOP;
        stateMap[DeviceStates::POSITIONDEGREE] = D3DRS_POSITIONDEGREE;
        stateMap[DeviceStates::NORMALDEGREE] = D3DRS_NORMALDEGREE;
        stateMap[DeviceStates::SCISSORTESTENABLE] = D3DRS_SCISSORTESTENABLE;
        stateMap[DeviceStates::SLOPESCALEDEPTHBIAS] = D3DRS_SLOPESCALEDEPTHBIAS;
        stateMap[DeviceStates::ANTIALIASEDLINEENABLE] = D3DRS_ANTIALIASEDLINEENABLE;
        stateMap[DeviceStates::MINTESSELLATIONLEVEL] = D3DRS_MINTESSELLATIONLEVEL;
        stateMap[DeviceStates::MAXTESSELLATIONLEVEL] = D3DRS_MAXTESSELLATIONLEVEL;
        stateMap[DeviceStates::ADAPTIVETESS_X] = D3DRS_ADAPTIVETESS_X;
        stateMap[DeviceStates::ADAPTIVETESS_Y] = D3DRS_ADAPTIVETESS_Y;
        stateMap[DeviceStates::ADAPTIVETESS_Z] = D3DRS_ADAPTIVETESS_Z;
        stateMap[DeviceStates::ADAPTIVETESS_W] = D3DRS_ADAPTIVETESS_W;
        stateMap[DeviceStates::ENABLEADAPTIVETESSELLATION] = D3DRS_ENABLEADAPTIVETESSELLATION;
        stateMap[DeviceStates::TWOSIDEDSTENCILMODE] = D3DRS_TWOSIDEDSTENCILMODE;
        stateMap[DeviceStates::CCW_STENCILFAIL] = D3DRS_CCW_STENCILFAIL;
        stateMap[DeviceStates::CCW_STENCILZFAIL] = D3DRS_CCW_STENCILZFAIL;
        stateMap[DeviceStates::CCW_STENCILPASS] = D3DRS_CCW_STENCILPASS;
        stateMap[DeviceStates::CCW_STENCILFUNC] = D3DRS_CCW_STENCILFUNC;
        stateMap[DeviceStates::COLORWRITEENABLE1] = D3DRS_COLORWRITEENABLE1;
        stateMap[DeviceStates::COLORWRITEENABLE2] = D3DRS_COLORWRITEENABLE2;
        stateMap[DeviceStates::COLORWRITEENABLE3] = D3DRS_COLORWRITEENABLE3;
        stateMap[DeviceStates::BLENDFACTOR] = D3DRS_BLENDFACTOR;
        stateMap[DeviceStates::SRGBWRITEENABLE] = D3DRS_SRGBWRITEENABLE;
        stateMap[DeviceStates::DEPTHBIAS] = D3DRS_DEPTHBIAS;
        stateMap[DeviceStates::WRAP8] = D3DRS_WRAP8;
        stateMap[DeviceStates::WRAP9] = D3DRS_WRAP9;
        stateMap[DeviceStates::WRAP10] = D3DRS_WRAP10;
        stateMap[DeviceStates::WRAP11] = D3DRS_WRAP11;
        stateMap[DeviceStates::WRAP12] = D3DRS_WRAP12;
        stateMap[DeviceStates::WRAP13] = D3DRS_WRAP13;
        stateMap[DeviceStates::WRAP14] = D3DRS_WRAP14;
        stateMap[DeviceStates::WRAP15] = D3DRS_WRAP15;
        stateMap[DeviceStates::SEPARATEALPHABLENDENABLE] = D3DRS_SEPARATEALPHABLENDENABLE;
        stateMap[DeviceStates::SRCBLENDALPHA] = D3DRS_SRCBLENDALPHA;
        stateMap[DeviceStates::DESTBLENDALPHA] = D3DRS_DESTBLENDALPHA;
        stateMap[DeviceStates::BLENDOPALPHA] = D3DRS_BLENDOPALPHA;
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
};

IDevice* createDeviceD3D9() {
    return new IDevice_D3D9();
}