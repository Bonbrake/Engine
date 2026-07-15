#include <entt/entt.hpp>
#include <type_traits>
#include <iostream>

struct Transform { float x,y,z; };

int main() {
    entt::registry reg;
    auto* storage = reg.storage<Transform>();
    auto raw = storage->raw();
    std::cout << typeid(decltype(raw)).name() << std::endl;
    return 0;
}
