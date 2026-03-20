//
// Created by jhone on 19/03/2026.
//

#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

struct PendingTexture {
    std::string path;
    SDL_Surface* surface;
};

class AssetManager {
public:
    AssetManager(SDL_Renderer* renderer);
    ~AssetManager();

    void load_all_from_folder(const std::string& folder);
    void request_texture(const std::string& path);

    SDL_Texture* get_texture(const std::string& path);

    void process_gpu_uploads();

    void shutdown();

private:
    void worker_thread();

    SDL_Renderer* renderer;

    std::unordered_map<std::string, SDL_Texture*> textures;

    std::queue<std::string> load_queue;
    std::queue<PendingTexture> gpu_queue;

    std::mutex mutex;
    std::condition_variable cond_var;

    std::thread worker;
    bool running = true;
};


#endif //ASSET_MANAGER_H
