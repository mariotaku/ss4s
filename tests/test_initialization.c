#include "ss4s.h"

int main(int argc, char *argv[]) {
    SS4S_Config config = {.audioDriver = NULL, .videoDriver = "empty"};
    SS4S_Init(argc, argv, &config);
    SS4S_PostInit(argc, argv);
    SS4S_Quit();
    return 0;
}