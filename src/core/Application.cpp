#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <iostream>

// YAGNI: Everything in main() for M0 to prove the pipeline before abstracting.
int main(int argc, char* argv[]) {
    // 1. Initialize SDL3
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to init SDL3: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Engine M0 - Skeleton", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        return -1;
    }

    // 2. Bootstrapping Vulkan (Instance & Debug Messenger)
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("ZombieEngine")
                           .request_validation_layers() // Essential for catching errors early
                           .use_default_debug_messenger()
                           .build();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance: " << inst_ret.error().message() << "\n";
        return -1;
    }
    vkb::Instance vkb_inst = inst_ret.value();

    // 3. Create Window Surface
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, vkb_inst.instance, nullptr, &surface)) {
        std::cerr << "Failed to create Vulkan surface: " << SDL_GetError() << "\n";
        return -1;
    }

    // 4. Select Physical Device (GPU)
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto phys_ret = selector.set_surface(surface)
                            .set_minimum_version(1, 2) // Demand at least Vulkan 1.2
                            .select();
    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan Physical Device: " << phys_ret.error().message() << "\n";
        return -1;
    }
    vkb::PhysicalDevice phys_dev = phys_ret.value();
    std::cout << "Selected GPU: " << phys_dev.name << "\n";

    // 5. Create Logical Device
    vkb::DeviceBuilder device_builder{phys_dev};
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan Logical Device: " << dev_ret.error().message() << "\n";
        return -1;
    }
    vkb::Device vkb_device = dev_ret.value();

    // 6. Main Loop
    bool running = true;
    SDL_Event event;
    
    // Fixed timestep constants
    const double dt = 1.0 / 60.0;
    double accumulator = 0.0;
    uint64_t last_time = SDL_GetTicksNS();

    while (running) {
        // Calculate delta time
        uint64_t current_time = SDL_GetTicksNS();
        double frame_time = (current_time - last_time) / 1e9;
        last_time = current_time;
        accumulator += frame_time;

        // Input & Events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }

        // Fixed Update
        while (accumulator >= dt) {
            // physics_world->Update(dt);
            accumulator -= dt;
        }

        // Render pass will go here
    }

    // 7. Teardown
    vkb::destroy_device(vkb_device);
    vkb::destroy_surface(vkb_inst.instance, surface);
    vkb::destroy_instance(vkb_inst);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}