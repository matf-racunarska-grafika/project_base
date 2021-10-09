//
// Created by matf-rg on 6.10.21..
//

#ifndef PROJECT_BASE_ENTITY_CONTROLLER_H
#define PROJECT_BASE_ENTITY_CONTROLLER_H
#include<variant>
struct GLFWwindow;

struct Monster{
    void update(float dt);
    void draw();
};
struct Dragon{
    void update(float dt);
    void draw();
};
struct Human{
    void update(float dt);
    void draw();
};

namespace rg {
    namespace {
        struct HandleImpl {
            size_t id{0};
            size_t generation{0};
        };
    }


    template<typename TEntity>
    class EntityHandle {
        friend class EntityController;
    private:
        HandleImpl handle;
    };

    class EntityController {
    public:
        template<typename TEntity>
        EntityHandle<TEntity> createEntity() {
            auto handle_it = std::find_if(m_handles.begin(), m_handles.end(), [](HandleImpl h) {
                return h.id == 0;
            });
            assert(handle_it != m_handles.end());
            const size_t id = std::distance(m_handles.begin(), handle_it);
            handle_it->id = id;
            handle_it->generation += 1;
            m_entities[id] = TEntity{};

            EntityHandle<TEntity> result;
            result.handle = *handle_it;
            return result;
        }

        template<typename TEntity>
        TEntity* getEntity(EntityHandle<TEntity> handle) {
            assert(handle.id < m_handles.size());
            if (m_handles[handle.id].generation == handle.generation) {
                if (TEntity *e = std::get<TEntity>(&m_entities[handle.id])) {
                    return e;
                }
            }
            return nullptr;
        }

        template<typename TEntity>
        bool destroyEntity(EntityHandle<TEntity> handle) {
            assert(handle.id < m_handles.size());
            if (m_handles[handle.id].generation == handle.generation) {
                m_handles[handle.id].id = 0;
                return true;
            }
            return false;
        }

        void updateEntities(float dt);
        void drawEntities();

    private:
        std::array<std::variant<Monster, Dragon, Human>, 1024> m_entities;
        std::array<HandleImpl, 1024> m_handles;
    };
}

/*
 * class ServiceLocator:
 *    InputController: procesira i cuva stanje ulaza (mis, tastatura...)
 *    EntityController: kontrolise sve entitet i njihovo crtanje
 *    ProcessController: kontrolise sva desavanja na sceni
 *    EventController: obradjuje dogadjaje
 */

#endif //PROJECT_BASE_ENTITY_CONTROLLER_H


