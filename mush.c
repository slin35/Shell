#include "mush.h"

int main(int argc, char *argv[]) {
    FILE *filePtr;

    if (argc == 1) {
        process_args(argc, argv, stdin, INTERACTIVE);
    }
    else if (argc == 2) {
        if ((filePtr = fopen(argv[1], "r")) == NULL) {
            perror("file doesn't exist");
            exit(1);
        }
        process_args(argc, argv, filePtr, BATCH);
        fclose(filePtr);
    }
    else {
        perror("usage: mush [scriptfile]");
        exit(1);
    }
    
    return 0;
}
