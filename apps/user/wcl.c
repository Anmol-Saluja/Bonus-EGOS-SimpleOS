#include "app.h"
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: wcl [FILE1] [FILE2] ...\n");
        return 1;
    }
    int total_lines = 0;
    // Loop over all filenames given on command line
    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        int file_ino = dir_lookup(workdir_ino, filename);
        // Stop immediately on file-not-found
        if (file_ino < 0) {
            printf("wcl: file not found: %s\n", filename);
            return 1;   
        }
        char buf[BLOCK_SIZE];       // buffer for one block
        int file_lines = 0;         // line count for this file
        int block_index = 0;        // which block we are reading
        int any_chars = 0;        
        int prev_is_newline = 1;    // start as "at beginning of a line"
        while (1) {
            memset(buf, 0, BLOCK_SIZE);

            int rc = file_read(file_ino, block_index, buf);
            if (rc != 0){
                break;              // no more blocks to read
            }
            for (int j = 0; j < BLOCK_SIZE; j++) {
                char c = buf[j];
                if (c == '\0') {     // c == '\0' means unused region in this block
                    goto END_OF_FILE_BLOCK;
                }
                any_chars = 1;
                if (c == '\n') {
                    file_lines++;
                    prev_is_newline = 1;
                } else {
                    prev_is_newline = 0;
                }
            }

            block_index++;
            continue;

        END_OF_FILE_BLOCK:
            break;
        }

        // Handle last line without newline at EOF
        if (any_chars && !prev_is_newline){
            file_lines++;
        }
        printf("%s: %d\n", filename, file_lines);
        total_lines += file_lines;
    }

    printf("Total: %d\n", total_lines);
    return 0;
}

