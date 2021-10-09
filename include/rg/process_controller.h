#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include<memory>
#include<vector>

namespace rg {
    class ProcessBase {
    public:
        virtual ~ProcessBase() = default;
        virtual void update(float dt) = 0;
        virtual bool isDone() { return false; }
        virtual int priority() { return 0; }
    };

    class ProcessController {
    public:
        virtual ~ProcessController() = default;
        template<typename T, typename ...Args>
        void pushProcess(Args &&...args);

        void pushProcess(std::unique_ptr<ProcessBase> p);

        void update(float dt);
        ProcessController() {
            m_next_frame_processes.reserve(1024);
            m_current_frame_processes.reserve(1024);
        }
    private:
        std::vector<std::unique_ptr<ProcessBase>> m_current_frame_processes;
        std::vector<std::unique_ptr<ProcessBase>> m_next_frame_processes;
    };
}
#endif // PROCESSMANAGER_H
