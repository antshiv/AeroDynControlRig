#include "application.h"
#include <iostream>

int main() {
    Application app;
    if (!app.init()) {
        return -1;
    }

    while (app.running()) {
        app.update2D();
        app.render2D();
    }

    app.shutdown();
    return 0;
} 
