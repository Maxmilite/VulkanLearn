#include "headers/GLFWGeneral.hpp"

int main(int argc, char* argv[]) {

    std::cout << "Hello World" << std::endl;
    
    GLFW::initWindow({1600, 900});

    while (!GLFW::shouldClose()) {
        GLFW::fps();
        glfwPollEvents();
    }
    
    GLFW::terminateWindow();

    return 0;
}