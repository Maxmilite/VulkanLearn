#include "headers/GLFWGeneral.hpp"

int main(int argc, char* argv[]) {

    GLFW::initWindow({1600, 900});

    while (!GLFW::shouldClose()) {
        GLFW::fps();
        glfwPollEvents();
    }
    
    GLFW::terminateWindow();

    return 0;
}