# this is basically a build system manager that manages and reduces the cmd line prompts required for putting the path of the files 

cmake_minimum_required(VERSION 3.28)

# cmake is the program that will read this file when we will call the program throught the terminal 
# minimum reuired line tells that this particular prject requires 3.28 version of cmake 

project(DeadAim LANGUAGES CXX)
# this project uses c++ as the programming language 

include(FetchContent)
# this basically import library called sfml through whic we dont need to download the complete sfml on our system...it'll just link sfml to our code ...it saves memory 

# below line just tells the requirements or dependency i.e. SFML rn
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0

    GIT_SHALLOW ON
    # "Don't download the entire history. Just download the minimum amount needed to build version 3.1.0."

    EXCLUDE_FROM_ALL
    # "I need SFML as a dependency, but don't treat all of its extra targets as part of my normal build."

    SYSTEM
    # it basically removes faltu and unwanted warnings of the code rom imported libraries like sfml and let you know only your mistakes
)

FetchContent_MakeAvailable(SFML)
# this lines downloads the declared libraries

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

# I want you to create a runnable program named DeadAim that an os can execute
# this line just defines the targets
# target is something that cmake already knows how to make 
add_executable(
    DeadAim 
    src/main.cpp #this is the entry point of our game
    src/Application.cpp 
    src/Window.cpp 
    src/Renderer.cpp
    src/AssetManager.cpp
    src/Scene.cpp
    src/Player.cpp
    src/Gun.cpp
    src/Projectile.cpp
    src/Zombie.cpp
    src/CollisionSystem.cpp
    src/WaveSystem.cpp
    src/VisionClient.cpp
    src/HUD.cpp
    src/MenuScreen.cpp
    src/SaveSystem.cpp
    src/Bow.cpp
    src/Fireball.cpp
    src/Explosion.cpp
    src/AudioSystem.cpp
    src/Environment.cpp
    src/WeaponView.cpp
)

target_include_directories(DeadAim PRIVATE include)
# deadaim is the target....private is the visibility that will be taught later.....include lets the compiler know that while searching for window.h ....just ook inside the include file

target_compile_features(DeadAim PRIVATE cxx_std_17)
# this line basically tells that compile this program using the standard c++ 17 edition

target_link_libraries(DeadAim PRIVATE SFML::Graphics nlohmann_json::nlohmann_json)
# private means that only deadaim requires hese dependencies

target_link_libraries(DeadAim PRIVATE SFML::Graphics SFML::Audio nlohmann_json::nlohmann_json)