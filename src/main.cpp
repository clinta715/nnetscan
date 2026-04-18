#include "gui.h"
#include "cli.h"

#include <windows.h>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1) {
        return runCli(argc, argv);
    }

    FreeConsole();

    MainWindow window;

    if (!window.create()) {
        MessageBoxA(nullptr, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    window.run();

    return 0;
}
