#include <dlfcn.h>
#include <functional>
#include <iostream>
#include <stdexcept>

#include "vamp/vamp.h"

static void resetDllError() {
    dlerror();
}

static void checkDllError() {
    const char* error = dlerror();
    if (error) {
        throw std::runtime_error(error);
    }
}

int main() {
    void* handler = dlopen("example-plugin.so", RTLD_NOW);
    if (!handler) {
        throw std::runtime_error(dlerror());
    }
    resetDllError();

    auto func = (const VampPluginDescriptor* (*)(unsigned int, unsigned int)) dlsym(handler, "vampGetPluginDescriptor");
    checkDllError();

    const VampPluginDescriptor* descriptor = func(2, 0);
    if (!descriptor) {
        throw std::runtime_error("No descriptor returned");
    }
    std::cout << "Got descriptor" << std::endl;

    dlclose(handler);
}
