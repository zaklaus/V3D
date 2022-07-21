#include "IGraph.h"
#include "IDevice.h"

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3.h>
    #include <GLFW/glfw3native.h>

    #include "devices/IDevice_D3D9.h"
#else
    #include <GLFW/glfw3.h>
#endif

#include "devices/IDevice_GL.h"

IGraph::IGraph() { }

IGraph::~IGraph() { }

bool IGraph::init(RenderingBackend backendType, int width, int height, const char* title) {

    _backendType = backendType;

    if(!glfwInit()) {
        spdlog::error("unable to initialize glfw !");
        return false;
    }

    if(backendType != RenderingBackend::OpenGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (_window == nullptr) {
        spdlog::error("unable to create glfwWindow ! w: {} h: {}", width, height);
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(_window);

    //NOTE: for GL we need extension loader
    if(backendType == RenderingBackend::OpenGL) {
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    }

    //NOTE: init callbacks
    {
        glfwSetWindowUserPointer(_window, this);

        auto keyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            (reinterpret_cast<IGraph*>(glfwGetWindowUserPointer(window)))->keyCallback(key, scancode, action, mods);
        };

        glfwSetKeyCallback(_window, keyCallback);

        auto mouseCallback = [](GLFWwindow* window, double posX, double posY) {
            (reinterpret_cast<IGraph*>(glfwGetWindowUserPointer(window)))->mouseMove(posX, posY);
        };

        glfwSetCursorPosCallback(_window, mouseCallback);

        auto resizeCallback = [](GLFWwindow* window, int width, int height) {
            (reinterpret_cast<IGraph*>(glfwGetWindowUserPointer(window)))->onResize(width, height);
        };

        glfwSetWindowSizeCallback(_window, resizeCallback);
    }

    glfwSwapInterval(1);

    _monitor = glfwGetPrimaryMonitor();
    glfwGetWindowSize(_window, &_windowSize[0], &_windowSize[1]);
    glfwGetWindowPos(_window, &_windowPos[0], &_windowPos[1]);

    if(!initRenderBackend()) {
        return false;
    }

    spdlog::info("IGraph successfully created rendering backed: {}", _renderBackend->getRenderingApiName());
    _inited = true;
    return true;
}

bool IGraph::initRenderBackend() {
    switch(_backendType) {

        #ifdef _WIN32
        case RenderingBackend::D3D9:
            _renderBackend = createDeviceD3D9();
        break;
        #endif

        case RenderingBackend::OpenGL:
            _renderBackend = createDeviceGL();
        break;

        default: {
            spdlog::error("unsuported rendering backed !");
            _renderBackend = nullptr;
        } break;
    }

    if(_renderBackend == nullptr)
        return false;

    void* windowHandle{ nullptr};

#ifdef _WIN32
    windowHandle = glfwGetWin32Window(_window);
#endif

    if(!_renderBackend->init(windowHandle)) {
        spdlog::error("unable to init rendering backend: {} !");
        return false;
    }

    _renderBackend->setViewport(_windowSize);
    return true;
}

void IGraph::destroy() {
    if(!_inited) return;
    _renderBackend->destroy();
    glfwTerminate();
}

void IGraph::pollEvents() {
    if (!_inited) return;
    glfwPollEvents();
}

void IGraph::render() {
    if(_backendType == RenderingBackend::OpenGL)
        glfwSwapBuffers(_window);

    _mouseDelta = {};
}

bool IGraph::closeRequested() const {
    return glfwWindowShouldClose(_window);
}

bool IGraph::isKeyDown(int key) const {
    return glfwGetKey(_window, key) == GLFW_PRESS;
}

bool IGraph::isMouseKeyDown(int key) const {
    return glfwGetMouseButton(_window, key) == GLFW_PRESS;
}

void IGraph::onResize(int width, int height) {
    glfwGetFramebufferSize(_window, &_windowSize[0], &_windowSize[1]);
}

void IGraph::mouseMove(float posX, float posY) {
    _mouseDelta.x += (posX - _lastMouse.x);
    _mouseDelta.y += (posY - _lastMouse.y);
    _lastMouse.x = posX;
    _lastMouse.y = posY;
}

void IGraph::keyCallback(int key, int scancode, int action, int mods) {
    if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(_window, true);
}

const glm::vec3& IGraph::getMouseDelta() {
    return _mouseDelta;
}

void IGraph::setCursorPos(float x, float y) {
    glfwSetCursorPos(_window, x, y);
    _lastMouse.x = x;
    _lastMouse.y = y;
}

double IGraph::getTime() {
    return glfwGetTime();
}