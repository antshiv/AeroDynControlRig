#include "application.h"
#include <iostream>

int main() {
    Application app;
    if (!app.init()) {
        return -1;
    }

    while (app.running()) {
       // app.update();
        app.render();
    }

    app.shutdown();
    return 0;
} 
