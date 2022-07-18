#pragma once
#include <glm/glm.hpp>

struct GLFWwindow;
struct GLFWmonitor;
class IDevice;

enum class RenderingBackend {
    D3D9,
    OpenGL
};

class IGraph {
    public:
        IGraph();
        ~IGraph();
        bool init(RenderingBackend backendType, int width, int height, const char* title);
        void destroy();
        void pollEvents();
        void render();
        bool closeRequested() const;
        bool isKeyDown(int key) const;
        bool isMouseKeyDown(int key) const;
        const glm::ivec2& getWindowSize() const { return _windowSize; }

        //NOTE: used internally
        void onResize(int width, int height);
        void mouseMove(float posX, float posY);
        void setCursorPos(float x, float y);
        void keyCallback(int key, int scancode, int action, int mods);

        const glm::vec3& getMouseDelta();
        double getTime();

        IDevice* getDevice() { return _renderBackend; }
    private:
        bool initRenderBackend();

        GLFWwindow* _window{ nullptr };
        GLFWmonitor* _monitor{ nullptr };
        IDevice* _renderBackend{ nullptr };
        RenderingBackend _backendType;
        bool _inited{ false };
        glm::ivec2 _windowSize{};
        glm::ivec2 _windowPos{};
        glm::vec3 _mouseDelta{};
        glm::vec3 _lastMouse{};
};

