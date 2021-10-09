//
// Created by matf-rg on 6.10.21..
//

#ifndef PROJECT_BASE_SERVICELOCATOR_H
#define PROJECT_BASE_SERVICELOCATOR_H

#include <rg/input_controller.h>
#include <rg/process_controller.h>
#include <rg/entity_controller.h>
#include <rg/event_controller.h>
namespace rg {
    class ServiceLocator {
    public:
        InputController& getInputController() { return m_InputController; }
        ProcessController& getProcessController() { return m_ProcessController; }
        EntityController& getEntityController()  { return m_EntityController; }
        EventController& getEventController() { return m_EventController; }
        static ServiceLocator& Get() {
            static ServiceLocator serviceLocator;
            return  serviceLocator;
        }
    private:
        ServiceLocator() = default;
        InputController m_InputController;
        ProcessController m_ProcessController;
        EntityController m_EntityController;
        EventController m_EventController;
    };

}


#endif //PROJECT_BASE_SERVICELOCATOR_H
