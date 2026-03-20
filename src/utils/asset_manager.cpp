//
// Created by jhone on 19/03/2026.
//

#include "asset_manager.h"
#include <filesystem>
#include <iostream>
#include <SDL3_image/SDL_image.h>

namespace fs = std::filesystem;

AssetManager::AssetManager(SDL_Renderer* renderer)
    : renderer(renderer)
{
    worker = std::thread(&AssetManager::WorkerThread, this);
}

AssetManager::~AssetManager() {
    Shutdown();

    for (auto& [k, v] : textures) {
        SDL_DestroyTexture(v);
    }
}

void AssetManager::Shutdown() {
    {
        std::lock_guard lock(mutex);
        running = false;
    }

    cond_var.notify_all();

    if (worker.joinable())
        worker.join();
}

void AssetManager::load_all_from_folder(const std::string& folder) {
    for (auto& entry : fs::recursive_directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();

        if (path.ends_with(".png") || path.ends_with(".jpg")) {
            request_texture(path);
        }
    }
}

void AssetManager::request_texture(const std::string& path) {
    std::lock_guard lock(mutex);

    if (textures.contains(path))
        return;

    load_queue.push(path);
    cond_var.notify_one();
}

SDL_Texture* AssetManager::GetTexture(const std::string& path) {
    std::lock_guard lock(mutex);

    if (textures.contains(path))
        return textures[path];

    return nullptr;
}

void AssetManager::WorkerThread() {
    while (true) {
        std::unique_lock lock(mutex);

        cond_var.wait(lock, [&]() {
            return !load_queue.empty() || !running;
        });

        if (!running && load_queue.empty())
            break;

        std::string path = load_queue.front();
        load_queue.pop();

        lock.unlock();

        SDL_Surface* surface = IMG_Load(path.c_str());

        if (!surface) {
            std::cerr << "fail to load image: " << path << std::endl;
            continue;
        }

        lock.lock();
        gpu_queue.push({path, surface});
    }
}

void AssetManager::ProcessGPUUploads() {
    std::lock_guard lock(mutex);

    while (!gpu_queue.empty()) {
        auto& item = gpu_queue.front();

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, item.surface);

        SDL_DestroySurface(item.surface);

        if (tex) {
            textures[item.path] = tex;
            std::cout << "[GPU] Texture created: " << item.path << std::endl;
        }

        gpu_queue.pop();
    }
}