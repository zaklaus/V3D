#include "IDevice_D3D9.h"
#include "../IDevice.h"

#include <d3d9.h>
#define GLM_DX(X) *(D3DMATRIX*)(&X)

struct VertexDecl_Userdata {
    LPDIRECT3DVERTEXBUFFER9 Buffer;
    size_t Stride;
};

class IDevice_D3D9 : public IDevice {
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

        //NOTE: some device states
        _device->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
        _device->SetRenderState(D3DRS_LIGHTING, FALSE);
        _device->SetRenderState(D3DRS_ZENABLE, TRUE);
        
        _device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        _device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        _device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        _device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
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
                auto* data = reinterpret_cast<VertexDecl_Userdata*>(handle._userData);
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
    ResourceHandle createVertexDeclaration(const eastl::vector<VertexDeclElement>& vertexDecl) override {
        if(vertexDecl.empty()) return {};

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

        VertexDecl_Userdata* bufferUserData = new VertexDecl_Userdata();
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
            auto* data = reinterpret_cast<VertexDecl_Userdata*>(handle._userData);
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