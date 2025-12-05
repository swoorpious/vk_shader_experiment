#include <SDL3/SDL_main.h>

#include <iostream>
#include "Engine.h"

/*
 * assuming windows build platform
 */
int main(int argc, char* argv[]) {
    try {
        Engine app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}