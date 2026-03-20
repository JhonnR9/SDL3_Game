//
// Created by jhone on 12/08/2025.
//

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include "json.h"
#include <unordered_map>
#include <ranges>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

namespace fs = std::filesystem;

struct PackedImage {
    std::string name;
    int width{}, height{}, channels{};
    unsigned char *pixels = nullptr;

    PackedImage(std::string name_, int w, int h, int c, unsigned char *data)
        : name(std::move(name_)), width(w), height(h), channels(c), pixels(data) {
    }

    ~PackedImage() {
        if (pixels) stbi_image_free(pixels);
    }

    // Prevent accidental copy
    PackedImage(const PackedImage &) = delete;

    PackedImage &operator=(const PackedImage &) = delete;

    // Move operations
    PackedImage(PackedImage &&other) noexcept {
        *this = std::move(other);
    }

    PackedImage &operator=(PackedImage &&other) noexcept {
        if (this != &other) {
            if (pixels) stbi_image_free(pixels);
            name = std::move(other.name);
            width = other.width;
            height = other.height;
            channels = other.channels;
            pixels = other.pixels;
            other.pixels = nullptr;
        }
        return *this;
    }
};

class TexturePacker {
    const int ATLAS_WIDTH = 2048;
    const int ATLAS_HEIGHT = 2048;
    const int MARGIN = 1;

public:
    void packer(
        const std::string &inputDir,
        const std::string &outputAtlasBase,
        const std::string &outputJson) const;

private:
    static std::vector<PackedImage> loadImages(const std::string &inputDir);

    void writeAtlasImage(const std::string &filename, const std::vector<unsigned char> &data) const;
};

std::vector<PackedImage> TexturePacker::loadImages(const std::string &inputDir) {
    std::vector<PackedImage> images;

    for (const auto &entry: fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            int w, h, c;
            unsigned char *data = stbi_load(entry.path().string().c_str(), &w, &h, &c, 4);
            if (data) {
                images.emplace_back(entry.path().filename().string(), w, h, 4, data);
                std::cout << "Loaded image: " << entry.path().filename() << " (" << w << "x" << h << ")\n";
            } else {
                std::cerr << "Error loading image: " << entry.path().string() << "\n";
            }
        }
    }

    return images;
}

void TexturePacker::writeAtlasImage(const std::string &filename, const std::vector<unsigned char> &data) const {
    if (!stbi_write_png(filename.c_str(), ATLAS_WIDTH, ATLAS_HEIGHT, 4, data.data(), ATLAS_WIDTH * 4)) {
        std::cerr << "Failed to save PNG: " << filename << "\n";
    } else {
        std::cout << "Atlas saved: " << filename << "\n";
    }
}

void TexturePacker::packer(
    const std::string &inputDir,
    const std::string &outputAtlasBase,
    const std::string &outputJson) const {
    std::vector<PackedImage> allImages = loadImages(inputDir);
    if (allImages.empty()) {
        std::cerr << "No images found in: " << inputDir << "\n";
        return;
    }

    nlohmann::json globalJson;
    int atlasCount = 0;

    while (!allImages.empty()) {
        std::vector<stbrp_rect> rects;
        std::vector<PackedImage> currentImages;

        // Prepare rectangles for packing
        for (auto &image: allImages) {
            stbrp_rect rect;
            rect.id = static_cast<int>(currentImages.size());
            rect.w = image.width + MARGIN * 2;
            rect.h = image.height + MARGIN * 2;
            rects.push_back(rect);
            currentImages.push_back(std::move(image));
        }

        const int NUM_NODES = ATLAS_WIDTH;
        std::vector<stbrp_node> nodes(NUM_NODES);
        stbrp_context context;
        stbrp_init_target(&context, ATLAS_WIDTH, ATLAS_HEIGHT, nodes.data(), NUM_NODES);
        stbrp_pack_rects(&context, rects.data(), static_cast<int>(rects.size()));

        std::vector<PackedImage> packedImages;
        std::vector<stbrp_rect> packedRects;
        std::vector<PackedImage> remainingImages;

        for (size_t i = 0; i < rects.size(); ++i) {
            if (rects[i].was_packed) {
                packedImages.push_back(std::move(currentImages[i]));
                packedRects.push_back(rects[i]);
            } else {
                remainingImages.push_back(std::move(currentImages[i]));
            }
        }

        if (packedImages.empty()) {
            std::cerr << "Error: Image(s) too large to fit in atlas.\n";
            break;
        }

        std::vector<unsigned char> atlas(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0);

        for (size_t i = 0; i < packedImages.size(); ++i) {
            const auto &img = packedImages[i];
            const auto &rect = packedRects[i];

            int dstX = rect.x + MARGIN;
            int dstY = rect.y + MARGIN;

            for (int y = 0; y < img.height; ++y) {
                for (int x = 0; x < img.width; ++x) {
                    int srcIdx = (y * img.width + x) * 4;
                    int dstIdx = ((dstY + y) * ATLAS_WIDTH + (dstX + x)) * 4;
                    std::memcpy(&atlas[dstIdx], &img.pixels[srcIdx], 4);
                }
            }

            globalJson[img.name] = {
                {"x", dstX},
                {"y", dstY},
                {"width", img.width},
                {"height", img.height},
                {"atlas_id", atlasCount}
            };
        }

        std::string atlasFile = outputAtlasBase + "_" + std::to_string(atlasCount) + ".png";
        writeAtlasImage(atlasFile, atlas);

        allImages = std::move(remainingImages);
        ++atlasCount;
    }

    if (std::ofstream outJson(outputJson); outJson) {
        outJson << globalJson.dump(4);
        std::cout << "Metadata saved to: " << outputJson << "\n";
    } else {
        std::cerr << "Error saving JSON.\n";
    }

    std::cout << "Process finished: " << atlasCount << " atlases generated.\n";
}


struct FileInfo {
    std::string name;
    std::filesystem::file_time_type mtime;
};

std::unordered_map<std::string, std::filesystem::file_time_type> loadCache(const std::string &cacheFile);

void saveCache(const std::string &cacheFile,
               const std::unordered_map<std::string, std::filesystem::file_time_type> &cache);

bool hasDirectoryChanged(const std::string &dir,
                         const std::unordered_map<std::string, std::filesystem::file_time_type> &oldCache) {
    for (const auto &entry: std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            auto path = entry.path().filename().string();
            if (auto mtime = entry.last_write_time(); !oldCache.contains(path) || oldCache.at(path) != mtime) {
                return true;
            }
        }
    }

    const bool removedFile = std::ranges::any_of(oldCache, [&](const auto &pair) {
        const auto &oldPath = pair.first;
        return !std::filesystem::exists(dir + "/" + oldPath);
    });

    if (removedFile) return true;
    return false;
}

std::unordered_map<std::string, std::filesystem::file_time_type> loadCache(const std::string &cacheFile) {
    std::unordered_map<std::string, std::filesystem::file_time_type> cache;

    std::ifstream inFile(cacheFile);
    if (!inFile.is_open()) {
        std::cout << "Cache file not found, starting fresh.\n";
        return cache;
    }

    nlohmann::json j;
    inFile >> j;

    for (auto &[key, val]: j.items()) {
        auto timestamp = val.get<int64_t>();
        const std::filesystem::file_time_type mtime{std::chrono::file_clock::duration(timestamp)};

        cache[key] = mtime;
    }

    return cache;
}

void saveCache(const std::string &cacheFile,
               const std::unordered_map<std::string, std::filesystem::file_time_type> &cache) {
    nlohmann::json j;

    for (const auto &[key, mtime]: cache) {
        // Salva o tempo como int64_t
        int64_t timestamp = mtime.time_since_epoch().count();
        j[key] = timestamp;
    }

    std::ofstream outFile(cacheFile);
    if (!outFile.is_open()) {
        std::cerr << "Failed to save cache to " << cacheFile << "\n";
        return;
    }

    outFile << j.dump(4);
}

int main(int argc, char **argv) {
    constexpr auto inputDir = "assets/images/";
    constexpr auto outputAtlasBase = "assets/atlas_texture/atlas";
    constexpr auto outputJson = "assets/atlas_texture/atlas.json";
    constexpr auto cacheFile = "assets/atlas_texture/cache.json";

    if (!std::filesystem::exists(inputDir)) {
        std::cout << "Input directory not found. Creating: " << inputDir << "\n";
        std::filesystem::create_directories(inputDir);
    }

    if (!std::filesystem::exists("assets/atlas_texture")) {
        std::cout << "Input directory not found. Creating: " << "assets/atlas_texture" << "\n";
        std::filesystem::create_directories("assets/atlas_texture");
    }
    auto oldCache = loadCache(cacheFile);

    if (hasDirectoryChanged(inputDir, oldCache)) {
        constexpr TexturePacker texturePacker;
        texturePacker.packer(inputDir, outputAtlasBase, outputJson);

        std::unordered_map<std::string, std::filesystem::file_time_type> newCache;
        for (const auto &entry: std::filesystem::directory_iterator(inputDir)) {
            if (entry.is_regular_file()) {
                newCache[entry.path().filename().string()] = entry.last_write_time();
            }
        }
        saveCache(cacheFile, newCache);
    } else {
        std::cout << "No changes detected, skipping packing.\n";
    }

    return 0;
}
