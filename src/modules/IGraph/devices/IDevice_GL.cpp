#include "IDevice_GL.h"
#include "../IDevice.h"

#undef APIENTRY // to suppress warnings
#include <glad/glad.h>
#include <stdio.h>

const char* vsShader = R"(#version 400 core
layout (location = 0) in vec4 pos;
layout (location = 1) in uint vertexColor;

uniform bool u_positionTransformed;

out vec4 vColor;

void main() {
    // TODO: store screen info in a shader and use it here
    if (u_positionTransformed) {
        gl_Position = pos;
        gl_Position.x = -gl_Position.x;
        gl_Position.y = 300 - gl_Position.y;
        gl_Position.x /= 400;
        gl_Position.y /= 300;
    } else {
        gl_Position = pos;
    }
    vColor = unpackUnorm4x8(vertexColor);
}
)";

const char* fsShader = R"(#version 400 core
in vec4 vColor;
out vec4 fragColor;

void main() {
    fragColor = vColor;
}
)";

struct VertexDecl_Userdata {
    GLuint Vao{};
    eastl::vector<VertexDeclElement> DeclElements;
};

struct GLResource_Userdata {
    GLuint Handle{};
};

class IDevice_GL : public IDevice {
private:
    GLuint _positionTransformedLoc{};
    bool _positionTransformed{false};

public:
    bool init(void* windowHandle) override {
        glEnable(GL_DEPTH_TEST);

        _shader = createProgram(vsShader, fsShader);
        glUseProgram(_shader);

        return true;
    }

    void destroy() override {
        glDisable(GL_DEPTH_TEST);
        glDeleteProgram(_shader);
    }

    //NOTE: textures
    ResourceHandle createTexture(TextureFormat format, int width, int height, const void* data, size_t size, int levels) override {
        bool generateMipmaps = (levels == 0);
        GLuint textureId{ 0 };
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        const GLint textureFormats[] = { GL_RGB, GL_RGBA, GL_BGRA };
        const GLint textureFormat = textureFormats[(uint32_t)format];
        glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, data);

        if (generateMipmaps)
            glGenerateMipmap(GL_TEXTURE_2D);

        // NOTE: anisotropic filtering
        /*GLfloat maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);*/

        auto* userData = new GLResource_Userdata();
        userData->Handle = textureId;

        return {
            ResourceType::TEXTURE,
            (void*)userData
        };
    }

    void destroyResource(ResourceHandle& handle) override {
        if(handle._userData == nullptr) return;

        switch(handle.Type) {
            case ResourceType::TEXTURE: {
                //TODO: here remove from vector
                auto* data = reinterpret_cast<GLResource_Userdata*>(handle._userData);
                assert(data->Handle != 0);
                glDeleteTextures(1, (const GLuint*)&data->Handle);
                delete data;
            } break;

            case ResourceType::BUFFER_INDEX:
            case ResourceType::BUFFER_VERTEX: {
                auto* data = reinterpret_cast<GLResource_Userdata*>(handle._userData);
                assert(data->Handle != 0);
                glDeleteBuffers(1, (const GLuint*)&data->Handle);
                delete data;
            } break;

            case ResourceType::VERTEX_DECL: {
                auto* data = reinterpret_cast<VertexDecl_Userdata*>(handle._userData);
                assert(data->Vao != 0);
                glDeleteVertexArrays(1, (const GLuint*)&data->Vao);
                delete data;
            } break;
        }

        handle._userData = nullptr;
    }

    void bindTexture(const ResourceHandle& handle, int samplerId) override {
        assert(handle.Type == ResourceType::TEXTURE);
        if(handle._userData == nullptr) return;

        auto* data = reinterpret_cast<GLResource_Userdata*>(handle._userData);
        assert(data->Handle != 0);

        glActiveTexture(GL_TEXTURE0 + samplerId);
        glBindTexture(GL_TEXTURE_2D, data->Handle);
    }

    void clearTexture(int samplerId) override {
        glActiveTexture(GL_TEXTURE0 + samplerId);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    constexpr size_t getVertexDeclElementCount(const VertexDeclElement& elem) {
        constexpr size_t declTypeSizes[] = {1, 2, 3, 4, 1, 4, 2, 4, 4, 2, 4, 2, 4, 3, 3, 2, 4, 0};
        return declTypeSizes[elem.DeclType];
    }

    constexpr GLenum getVertexDeclElementType(const VertexDeclElement& elem) {
        constexpr GLenum declTypes[] = {GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_INT, GL_UNSIGNED_BYTE, GL_SHORT, GL_SHORT, GL_UNSIGNED_BYTE, GL_SHORT, GL_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_INT, GL_HALF_FLOAT, GL_HALF_FLOAT, 0};
        return declTypes[elem.DeclType];
    }

    constexpr bool isVertexDeclElementNormalized(const VertexDeclElement& elem) {
        constexpr bool declTypesNormalized[] = {false, false, false, false, false, false, false, false, true, true, true, true, true, false, true, false, false, false };
        return declTypesNormalized[elem.DeclType];
    }

    GLuint getVertexDeclStride(const eastl::vector<VertexDeclElement>& vertexDecl) {
        constexpr GLuint declTypesSizes[] = {sizeof(float), sizeof(float), sizeof(float), sizeof(float), sizeof(uint32_t), sizeof(uint8_t), sizeof(int16_t), sizeof(int16_t), sizeof(uint8_t), sizeof(int16_t), sizeof(int16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint32_t), sizeof(int32_t), sizeof(int16_t), sizeof(int16_t), 0};

        GLuint strideSize{ 0 };
        for(auto& elem : vertexDecl) {
            strideSize += declTypesSizes[elem.DeclType] * getVertexDeclElementCount(elem);
        }

        return strideSize;
    }

    //NOTE: buffers
    ResourceHandle createVertexDeclaration(const eastl::vector<VertexDeclElement>& vertexDecl, const ResourceHandle& vertexBuffer) override {
        if(vertexDecl.empty()) return {};
        assert(vertexBuffer.Type == ResourceType::BUFFER_VERTEX);

        GLuint vao{};
        glGenVertexArrays(1, &vao);
        assert(vao != 0);

        glBindVertexArray(vao);

        // NOTE: vertex buffer needs to be bound before we map it to the VAO
        bindBuffer(vertexBuffer);

        GLuint vertexDeclStride = getVertexDeclStride(vertexDecl);

        for(size_t i = 0; i < vertexDecl.size(); i++) {
            VertexDeclElement vsElem = vertexDecl[i];
            auto type = getVertexDeclElementType(vsElem);
            if(type == GL_UNSIGNED_INT)
                glVertexAttribIPointer(i, getVertexDeclElementCount(vsElem), type, vertexDeclStride, (void*)vsElem.Offset);
            else
                glVertexAttribPointer(i, getVertexDeclElementCount(vsElem), type, isVertexDeclElementNormalized(vsElem), vertexDeclStride, (void*)vsElem.Offset);

            glEnableVertexAttribArray(i);
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        auto* data = new VertexDecl_Userdata();
        data->DeclElements = vertexDecl;
        data->Vao = vao;

        return {
            ResourceType::VERTEX_DECL,
            (void*)data
        };
    }

    void setVertexDeclaration(const ResourceHandle& handle) override {
        assert(handle.Type == ResourceType::VERTEX_DECL);
        if(handle._userData == nullptr) return;

        auto* data = reinterpret_cast<VertexDecl_Userdata*>(handle._userData);
        assert(data->Vao != 0);
        glBindVertexArray(data->Vao);

        _positionTransformed = false;

        for(auto& elem : data->DeclElements) {
            if (elem.DeclUsage == DECLUSAGE_POSITIONT) {
                _positionTransformed = true;
                break;
            }
        }

        glUniform1i(_positionTransformedLoc, _positionTransformed);
    }

    static constexpr GLuint getGlBufferUsage(BufferUsage usage) {
        switch (usage) {
            case BufferUsage::STATIC:
                return GL_STATIC_DRAW;
                break;
            case BufferUsage::DYNAMIC:
                return GL_DYNAMIC_DRAW;
                break;
            case BufferUsage::STREAM:
                return GL_STREAM_DRAW;
                break;
            default:
                return 0;
                break;
        }
    }

    ResourceHandle createVertexBuffer(const void* vertices, size_t verticesCnt, size_t vertexStride, BufferUsage usage) override {
        assert(vertexStride > 0 && verticesCnt > 0);

        GLuint vbo{ 0 };
        glGenBuffers(1, &vbo);
        assert(vbo != 0);

        if(vertices != nullptr) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verticesCnt * vertexStride), vertices, getGlBufferUsage(usage));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        auto* data = new GLResource_Userdata();
        data->Handle = vbo;

        return {
            ResourceType::BUFFER_VERTEX,
            (void*)data
        };
    }

    ResourceHandle createIndexBuffer(const uint32_t* indices, size_t indicesCnt) override {
        assert(indicesCnt > 0);

        GLuint indexBufer{ 0 };
        glGenBuffers(1, &indexBufer);
        assert(indexBufer != 0);

        if(indices != nullptr) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCnt * sizeof(uint32_t), indices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        auto* data = new GLResource_Userdata();
        data->Handle = indexBufer;

        return {
            ResourceType::BUFFER_INDEX,
            (void*)data
        };
    }

    void bindBuffer(const ResourceHandle& handle) override {
        if(handle._userData == nullptr) return;
        assert(handle.Type == ResourceType::BUFFER_INDEX || handle.Type == ResourceType::BUFFER_VERTEX);
        assert(handle._userData != nullptr);

        auto* data = reinterpret_cast<GLResource_Userdata*>(handle._userData);
        assert(data->Handle != 0);

        if(handle.Type == ResourceType::BUFFER_VERTEX) {
            glBindBuffer(GL_ARRAY_BUFFER, data->Handle);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->Handle);
        }
    }

    void setViewport(const glm::ivec2& size) override {
        glViewport(0, 0, size.x, size.y);
    }

    void setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj) override {

    }

    void setModelMatrix(const glm::mat4& model) override {

    }

    void drawPrimitives(uint32_t vertexCount, uint32_t indicesCount, uint32_t vertexOffset = 0, uint32_t indexOffset = 0) override {
        glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, (const void*)(sizeof(uint32_t) * vertexOffset));
    }

    void clear(uint32_t clearFlags, const glm::vec3& color = { 0.0f, 0.0f, 0.0f }) const override {
        GLuint flags{};
        if(clearFlags & CLEAR_COLOR)
            flags |= GL_COLOR_BUFFER_BIT;

        if(clearFlags & CLEAR_DEPTH)
            flags |= GL_DEPTH_BUFFER_BIT;

        if( clearFlags & CLEAR_STENCIL)
            flags |= GL_STENCIL_BUFFER_BIT;

        if(flags & CLEAR_COLOR)
            glClearColor(color.r, color.g, color.b, 1.0f);

        glClear(flags);
    }

    void beginScene() override {
    }

    void endScene() override {
    }

    void present() override {
        //glfwSwapBuffers ?
    }

    void checkProgramCompileErrors(GLuint shader, const char* type) {
        GLint success;
        GLchar infoLog[1024];
        if (strcmp(type, "PROGRAM") != 0) {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n\n", type, infoLog);
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n\n", type, infoLog);
            }
        }
    }

    GLuint createProgram(const char* vs, const char* fs) {
        GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vs, nullptr);
        glCompileShader(vshader);
        checkProgramCompileErrors(vshader, "VERTEX");

        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fs, nullptr);
        glCompileShader(fshader);
        checkProgramCompileErrors(fshader, "FRAGMENT");

        GLuint program = glCreateProgram();
        glAttachShader(program, vshader);
        glAttachShader(program, fshader);
        glLinkProgram(program);
        checkProgramCompileErrors(program, "PROGRAM");

        assert(vshader && fshader && program);
        glDeleteShader(vshader);
        glDeleteShader(fshader);

        _positionTransformedLoc = glGetUniformLocation(program, "u_positionTransformed");

        return program;
    }
private:
    GLuint _shader{};
};

IDevice* createDeviceGL() {
    return new IDevice_GL();
}