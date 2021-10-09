//
// Created by matf-rg on 6.10.21..
//

#ifndef PROJECT_BASE_EVENT_CONTROLLER_H
#define PROJECT_BASE_EVENT_CONTROLLER_H
#include<unordered_map>
#include<vector>
#include <rg/input_controller.h>
namespace rg {


    enum class EventType {
        MouseMoved,
        Keyboard,
    };

    struct EventMouseMoved {
        double xoffset;
        double yoffset;
    };

    struct EventKeyboard {
        int key;
        InputController::KeyState keyState;
    };

    struct Event {
        EventType eventType;
        union {
            EventMouseMoved mouseMoved;
            EventKeyboard keyboard;
        };
    };

    class Observer {
    public:
        virtual ~Observer() = default;
        virtual void notify(Event event) = 0;
    };

    class Observable {
    public:
        virtual void subscribe(EventType, Observer*);
    };



    class EventController {
    public:
        void pushEvent(Event event);
        void subscribeToEvent(EventType eventType, Observer* observer);
        void unsubscribeFromEvent(EventType eventType, Observer* observer);
        void unsubscribeFromAll(Observer* observer);
    private:
        std::unordered_map<EventType, std::vector<Observer*>> m_observers;
    };
}

// C++ nacin
// std::shared_ptr, std::weak_ptr

// EntityController

#endif //PROJECT_BASE_EVENT_CONTROLLER_H
