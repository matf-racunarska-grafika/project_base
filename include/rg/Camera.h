//
// Created by matf-rg on 15.11.20..
//

#ifndef PROJECT_BASE_CAMERA_H
#define PROJECT_BASE_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <rg/event_controller.h>
#include <vector>
#include <rg/Error.h>
enum Direction {
    FORWARD = 0,
    BACKWARD = 1,
    LEFT = 2,
    RIGHT = 3
};

class Camera : public rg::Observer {
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
    std::vector<rg::Event> m_events;
    std::array<bool, 4> m_movementDirectionVector = {false};

public:
    float Zoom = 45.f;
    float MovementSpeed = 2.5f;
    float Yaw = -90.0f;
    float Pitch = 0.0f;
    float MouseSensitivity = 0.1f;

    glm::vec3 Position = glm::vec3(2,2,10);
    glm::vec3 WorldUp = glm::vec3(0, 1, 0);
    glm::vec3 Up = glm::vec3(0, 1, 0);
    glm::vec3 Right;
    glm::vec3 Front = glm::vec3(0, 0, -4);

    Camera() {
        updateCameraVectors();
    }

    Camera(glm::vec3 front) {
        Front = front;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Direction direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
       switch (direction) {
           case FORWARD: {
              Position += Front * velocity;
           }break;
           case BACKWARD: {
                Position -= Front * velocity;
           }break;
           case LEFT: {
                Position -= Right * velocity;
           }break;
           case RIGHT: {
                Position += Right * velocity;
           }break;
       }
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;
        if (constrainPitch) {
            if (Pitch > 89.0f) {
                Pitch = 89.0f;
            }
            if (Pitch < -89.0f) {
                Pitch = -89.0f;
            }
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f) {
            Zoom = 1.0f;
        }
        if (Zoom > 45.0f) {
            Zoom = 45.0f;
        }
    }

    void notify(rg::Event event) override {
        LOG(std::cerr);
        m_events.push_back(event);
    }

    void update(float dt) {
        for (rg::Event& event : m_events) {
            if (event.eventType == rg::EventType::MouseMoved) {
                ProcessMouseMovement(event.mouseMoved.xoffset, event.mouseMoved.yoffset);
            } else if (event.eventType == rg::EventType::Keyboard) {
                auto keyboard = event.keyboard;
                std::cerr << rg::ToString(keyboard.keyState) << ' ' << keyboard.key << '\n';
                if (keyboard.key == GLFW_KEY_W) {
                    m_movementDirectionVector[FORWARD] = keyboard.keyState == rg::InputController::KeyState::Pressed;
                }
                if (keyboard.key == GLFW_KEY_S) {
                    m_movementDirectionVector[BACKWARD] = keyboard.keyState == rg::InputController::KeyState::Pressed;
                }
                if (keyboard.key == GLFW_KEY_A) {
                    m_movementDirectionVector[LEFT] = keyboard.keyState == rg::InputController::KeyState::Pressed;
                }
                if (keyboard.key == GLFW_KEY_D) {
                    m_movementDirectionVector[RIGHT] = keyboard.keyState == rg::InputController::KeyState::Pressed;
                }
            }
        }
        for (int direction = 0; direction < 4; ++direction) {
            if (m_movementDirectionVector[direction])
                ProcessKeyboard(static_cast<Direction>(direction), dt);
        }
        m_events.clear();
    }

};

#endif //PROJECT_BASE_CAMERA_H
