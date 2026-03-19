cmake_minimum_required(VERSION 3.31)

project(my_c_game CXX)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# SDL3
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG release-3.4.2
)

# SDL_ttf
FetchContent_Declare(
        SDL_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-3.2.2
)
set(SDLTTF_VENDORED ON)
set(SDLTTF_HARFBUZZ OFF)

# SDL_image
FetchContent_Declare(
        SDL_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-3.4.0
)

# cglm
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG v0.9.6
)

# EnTT
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.11.0
)

FetchContent_MakeAvailable(SDL3 SDL_ttf SDL_image cglm entt)

add_executable(my_c_game
        src/main.cpp
        src/app.cpp
        src/label.cpp
        src/my_scene.cpp
        src/my_scene.h
        src/Scene.cpp
        src/System.h
        src/render_system.cpp
        src/render_system.h
)

target_link_libraries(my_c_game PRIVATE
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        SDL3_image::SDL3_image
)

target_include_directories(my_c_game PRIVATE
        ${cglm_SOURCE_DIR}/include
        ${entt_SOURCE_DIR}/single_include
)

add_custom_command(TARGET my_c_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:my_c_game>
        $<TARGET_FILE_DIR:my_c_game>
        COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET my_c_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:my_c_game>/assets
)
