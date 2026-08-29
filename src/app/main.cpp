#include "application.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::string render_evidence_path;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--render-check" && index + 1 < argc) {
            render_evidence_path = argv[++index];
        } else {
            std::cerr << "Unknown or incomplete argument: " << argument << '\n';
            return 2;
        }
    }

    Application app;
    if (!app.init()) {
        return -1;
    }

    if (!render_evidence_path.empty()) {
        const bool passed = app.runRenderEvidenceCheck(render_evidence_path);
        app.shutdown();
        return passed ? 0 : 1;
    }

    while (app.running()) {
        app.tick();
    }

    app.shutdown();
    return 0;
} 
