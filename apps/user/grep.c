//grep.c
#include "app.h"
#include <string.h>
#define MAX_LINE_LEN 256  // maximum size of one line

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: grep [PATTERN] [FILE]\n");
        return 1;
    }
    char *pattern  = argv[1];
    char *filename = argv[2];
    // Look up file inode
    int file_ino = dir_lookup(workdir_ino, filename);
    if (file_ino < 0) {
        printf("grep: file not found: %s\n", filename);
        return 1;
    }
    char block[BLOCK_SIZE];          // buffer for one file block
    char line_buf[MAX_LINE_LEN];     // buffer for one line
    int  line_len    = 0;            // current line length
    int  block_index = 0;            // which block we are reading
    while (1) {
        // Read next block from file
        memset(block, 0, BLOCK_SIZE);
        int rc = file_read(file_ino, block_index, block);
        if (rc != 0) {            // No more blocks (EOF or error).
              
            // to handle if there's a last line without '\n'
            if (line_len > 0) {
                line_buf[line_len] = '\0';
                if (strstr(line_buf, pattern)) {
                    printf("%s\n", line_buf);
                }
            }
            return 0;
        }
        for (int i = 0; i < BLOCK_SIZE; i++) {
            char c = block[i];
            if (c == '\0') {
                // Null byte means end of file
                if (line_len > 0) {
                    line_buf[line_len] = '\0';
                    if (strstr(line_buf, pattern)) {
                        printf("%s\n", line_buf);
                    }
                }
                return 0;
            }
            if (c == '\n') {
                // End of a line so terminate string and check pattern
                line_buf[line_len] = '\0';
                if (strstr(line_buf, pattern)) {
                    printf("%s\n", line_buf);
                }
                line_len = 0;
            } else {
                if (line_len < MAX_LINE_LEN - 1) {
                    line_buf[line_len++] = c;
                }
            }
        }
        block_index++;
    }
    return 0;
}
