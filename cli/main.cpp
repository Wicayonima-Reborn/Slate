#include <iostream>
#include "ApplicationContext.hpp"

int main(int argc, char* argv[]) {
    slate::core::ApplicationContext ctx;
    ctx.init_logging();

    std::cout << "Slate v0.1.0 - Understand your codebase.\n";
    // TODO: command parsing
    return 0;
}
