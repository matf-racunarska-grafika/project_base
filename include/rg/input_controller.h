//
// Created by matf-rg on 6.10.21..
//

#ifndef PROJECT_BASE_INPUT_CONTROLLER_H
#define PROJECT_BASE_INPUT_CONTROLLER_H


#include<array>
#include<GLFW/glfw3.h>
#include<cassert>
#include<glm/vec2.hpp>
#include <rg/Error.h>
#include <unordered_map>
namespace rg {

    class InputController {
        friend class ServiceLocator;
    public:
        enum class KeyState {
            Released,
            JustPressed,
            Pressed,
            JustReleased
        };
        struct MouseMovementData {
            double lastX = 0.0;
            double lastY = 0.0;
            double currentX = 0.0;
            double currentY = 0.0;
        };
        struct ScrollMovementData {
            double lastY = 0.0;
            double currentY = 0.0;
        };

        void update(float dt);

        void processKeyCallback(GLFWwindow *window, int key, int action);

        void processMouseMovementCallback(double xpos, double ypos);

        void processMouseScrollCallback(double ypos);

        double getScrollOffset() const { return m_scroll.currentY - m_scroll.lastY; }

        const MouseMovementData &getMouseMovementData() const { return m_mouse; }

        glm::vec2 getMouseOffset() const;

        KeyState getKeyState(int key) const;

        InputController(const InputController &) = delete;
        InputController &operator=(const InputController &) = delete;
    private:
        struct KeyFrameState {
            KeyState keyState{KeyState::Released};
            int glfwKeyAction = -1;
        };
        InputController() {
            m_keys.reserve(1024);
        }
        std::unordered_map<int, KeyFrameState> m_keys;
        MouseMovementData m_mouse;
        ScrollMovementData m_scroll;
    };

    const char* ToString(InputController::KeyState state);

}
#endif //PROJECT_BASE_INPUT_CONTROLLER_H
