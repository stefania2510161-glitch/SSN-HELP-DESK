#include "commands.h"

int main(int argc, char* argv[]) {
    if (argc == 1) return 0;
    return run_command(argc, argv);
}
