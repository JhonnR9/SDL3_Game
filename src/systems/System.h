//
// Created by jhone on 18/03/2026.
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include <entt/entt.hpp>

class System {
protected:
    entt::registry &registry;

public:
    explicit System(entt::registry &registry) : registry(registry) {};

    virtual ~System() = default;

    virtual void run(float dt) =0;
};
#endif //SYSTEM_H
