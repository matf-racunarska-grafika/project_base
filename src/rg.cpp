#include "rg/input_controller.h"
#include "rg/process_controller.h"
#include "rg/entity_controller.h"
#include "rg/Error.h"
#include "rg/event_controller.h"
#include "rg/service_locator.h"
#include <algorithm>

namespace rg {
void InputController::processKeyCallback(GLFWwindow* window, int key, int action) {
    auto& keyState = m_keys[key];
    keyState.glfwKeyAction = action;
}

void InputController::update(float dt) {

    for (auto &key : m_keys) {
        auto& keyFrameState = key.second;
        int glfwKeyCode = key.first;
        int glfwKeyAction = keyFrameState.glfwKeyAction;
        bool keyStateChanged = false;
        switch (keyFrameState.keyState) {
            case KeyState::Released: {
                if (glfwKeyAction == GLFW_PRESS) {
                    keyFrameState.keyState = KeyState::JustPressed;
                    keyStateChanged = true;
                }
            }break;
            case KeyState::JustPressed: {
                if (glfwKeyAction == GLFW_PRESS || glfwKeyAction == -1) {
                    keyFrameState.keyState = KeyState::Pressed;
                    keyStateChanged = true;
                }
            }break;
            case KeyState::Pressed: {
                if (glfwKeyAction == GLFW_RELEASE) {
                    keyFrameState.keyState = KeyState::JustReleased;
                    keyStateChanged = true;
                }
            }break;
            case KeyState::JustReleased: {
                if (glfwKeyAction == GLFW_RELEASE || glfwKeyAction == -1) {
                    keyFrameState.keyState = KeyState::Released;
                    keyStateChanged = true;
                }
            }break;
        }
        if (keyStateChanged) {
            Event event;
            event.eventType = EventType::Keyboard;
            event.keyboard.key = glfwKeyCode;
            event.keyboard.keyState = keyFrameState.keyState;
            ServiceLocator::Get().getEventController().pushEvent(event);
        }
        keyFrameState.glfwKeyAction = -1;
    }
}

void InputController::processMouseMovementCallback(double xpos, double ypos) {
    m_mouse.lastX = m_mouse.currentX;
    m_mouse.lastY = m_mouse.currentY;
    m_mouse.currentX = xpos;
    m_mouse.currentY = ypos;

    auto& eventController = ServiceLocator::Get().getEventController();
    Event event;
    event.eventType = EventType::MouseMoved;
    event.mouseMoved.xoffset = m_mouse.currentX - m_mouse.lastX;
    event.mouseMoved.yoffset = m_mouse.lastY - m_mouse.currentY;

    eventController.pushEvent(event);
}

void InputController::processMouseScrollCallback(double ypos) {
    m_scroll.lastY = m_scroll.currentY;
    m_scroll.currentY = ypos;
}

InputController::KeyState InputController::getKeyState(int key) const {
    auto it = m_keys.find(key);
    return it != m_keys.end() ? it->second.keyState : KeyState::Released;
}

glm::vec2 InputController::getMouseOffset() const {
    double xoffset = m_mouse.currentX - m_mouse.lastX;
    double yoffset = m_mouse.lastY - m_mouse.currentY; // reversed since y-coordinates go from bottom to top
    return glm::vec2(xoffset, yoffset);
}


template<typename Process, typename ...Args>
void ProcessController::pushProcess(Args &&...args) {
    static_assert(std::is_base_of<ProcessBase, Process>::value);
    m_next_frame_processes.push_back(std::make_unique<Process>(std::forward<Args>(args)...));
}

void ProcessController::pushProcess(std::unique_ptr<ProcessBase> p) {
    m_next_frame_processes.push_back(std::move(p));
}


void ProcessController::update(float dt) {
    m_current_frame_processes.erase(
            std::remove_if(
                    m_current_frame_processes.begin(),
                    m_current_frame_processes.end(),
                    [](auto& p) { return p->isDone(); }
                    ),
            m_current_frame_processes.end()
    );
    std::move(
            m_next_frame_processes.begin(),
            m_next_frame_processes.end(),
            std::back_inserter(m_current_frame_processes)
    );
    m_next_frame_processes.clear();

    std::stable_sort(
        m_current_frame_processes.begin(),
        m_current_frame_processes.end(),
        [](auto& p1, auto& p2) {
            return p1->priority() > p2->priority();
        }
    );

    for (auto& p : m_current_frame_processes) {
        p->update(dt);
    }
}


void EventController::pushEvent(Event event) {
    LOG(std::cerr);
    auto& eventObservers = m_observers[event.eventType];
    for (auto& observer : eventObservers) {
        observer->notify(event);
    }
}
void EventController::subscribeToEvent(EventType eventType, Observer* observer) {
    auto& eventObservers = m_observers[eventType];
    if (std::find(eventObservers.begin(), eventObservers.end(), observer) == eventObservers.end())
        eventObservers.push_back(observer);
}
void EventController::unsubscribeFromEvent(EventType eventType, Observer* observer) {
    auto& eventObservers = m_observers[eventType];
    eventObservers.erase(
            std::remove(eventObservers.begin(), eventObservers.end(), observer),
            eventObservers.end()
    );
}
void EventController::unsubscribeFromAll(Observer* observer) {
    for (auto& eventPairs : m_observers) {
        unsubscribeFromEvent(eventPairs.first, observer);
    }
}


void clearAllOpenGlErrors() {
    while (glGetError() != GL_NO_ERROR) {
        ;
    }
}
const char* openGLErrorToString(GLenum error) {
    switch(error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    }
    ASSERT(false, "Passed something that is not an error code");
    return "THIS_SHOULD_NEVER_HAPPEN";
}
bool wasPreviousOpenGLCallSuccessful(const char* file, int line, const char* call) {
    bool success = true;
    while (GLenum error = glGetError()) {
        std::cerr << "[OpenGL error] " << error << " " << openGLErrorToString(error)
                  << "\nFile: " << file
                  << "\nLine: " << line
                  << "\nCall: " << call
                  << "\n\n";
        success = false;
    }
    return success;
}


const char* ToString(InputController::KeyState state) {
    using KeyState = InputController::KeyState;
    switch (state) {
        case KeyState::Released: return "Released";
        case KeyState::Pressed: return "Pressed";
        case KeyState::JustReleased: return "JustReleased";
        case KeyState::JustPressed: return "JustPressed";
    }
}

};

