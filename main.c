//
//  main.c
//  HW2
//
//  Created by Cameron Monks on 02/25/2020.
//  Copyright © 2020 Cameron Monks. All rights reserved.
//

#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include <termios.h>
#include <unistd.h>

#include <util.h>

typedef enum {false, true} bool;

/* reads from keypress, doesn't echo */
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

char ** lines;
int lines_count;
int lines_allocated;
int prevLineIndex;

// int pipes[2];
int masterfd;

void storeLines(const char* filename) {

    const char * home = getenv("HOME");
    const char * path = "/.ishell/.";
    char* filepath = malloc((strlen(home) + strlen(path) + strlen(filename) + 1)
            * sizeof(char));

    strcpy(filepath, home);
    strcat(filepath, path);
    strcat(filepath, filename);

    FILE* file = fopen(filepath, "a");

    for (int i = 0; i < lines_count; i++) {
        if (lines[i] ==  NULL) continue;

        fprintf(file, "%s\n", lines[i]); 
        free(lines[i]);
    }

    fclose(file);
    free(filepath);
    free(lines);
}

void loadLines(const char* filename) {
    
    const char * home = getenv("HOME");
    const char * path = "/.ishell/.";
    char* filepath = malloc((strlen(home) + strlen(path) + strlen(filename) + 1) 
            * sizeof(char));

    strcpy(filepath, home);
    strcat(filepath, path);
    strcat(filepath, filename);

    FILE* file = fopen(filepath, "r");
    lines_count = 0;
    lines_allocated = 1;
    lines = malloc(lines_allocated * sizeof(char *));

    if (file == NULL) {
        free(filepath);
        return;
    }

    char * line = malloc(1024 * sizeof(char));
    while (fgets(line, 1023, file)) {

        line[strlen(line) - 1] = '\0';
        
        lines[lines_count] = malloc((strlen(line) + 1) * sizeof(char));
        strcpy(lines[lines_count], line);
        lines_count++;
        
        if (lines_count >= lines_allocated) {
            lines_allocated *= 2;
            lines = realloc(lines, lines_allocated * sizeof(char *));
        }
    }

    prevLineIndex = lines_count;

    fclose(file);
    free(filepath);
    free(line);
}

bool containsAlphaNum(const char * str) {

    while (*str != '\0') {
        if (isalnum(*str)) {
            return true;
        }
        str++;
    }

    return false;
}

void saveline(char * str) {

    if (lines_count >= lines_allocated) {
        lines_allocated *= 2;
        lines = realloc(lines, lines_allocated * sizeof(char*)); 
    }

    prevLineIndex = lines_count+1;
    char* newStr = malloc((1 + strlen(str)) * sizeof(char));
    strcpy(newStr, str);
    lines[lines_count++] = newStr;
}

const char * getprevline() {

    if (prevLineIndex < 0) {
        prevLineIndex = 0;
        return "";
    } else if (prevLineIndex >= lines_count) {
        prevLineIndex = lines_count;
        return "";
    }

    return lines[prevLineIndex];
}

void swapLines(char * line, const char * newline) {
    
    for (int i = 0, n = strlen(line); i < n; i++) {
        fputc(8, stdout);
        fputc(24, stdout);
        fputc(' ', stdout);
        fputc(8, stdout);
    }

    fputc(24, stdout);
    strcpy(line, newline);
    printf("%s", line);
    fputc(3, stdout);
}

void deleteCInStr(char * line, int index) {

    unsigned count = 0;
    fputc(8, stdout);
    fputc(24, stdout);
    while (line[index] != '\0') {

        line[index] = line[index+1];
        fputc(line[index], stdout);

        index++;
        count++;
    }
    fputc(' ', stdout);
    fputc(8, stdout);
    line[index] = '\0';

    for (unsigned i = 1; i < count; i++) {
        fputc(8, stdout);
    }
    fputc(24, stdout);
}

void insertCInStr(char * line, int index, char c) {

    unsigned count = 0;
    while (c != '\0') {

        char n = line[index];
        line[index] = c;
        fputc(c, stdout);

        c = n;
        index++;
        count++;
    }
    line[index] = '\0';

    for (unsigned i = 1; i < count; i++) {
        fputc(8, stdout);
    }
    fputc(24, stdout);
}

char * getPathOfProgram(const char * program) {

    char * path = getenv("PATH");
    char * full_path = malloc(1024 * sizeof(char));
    unsigned index = 0;
    while (*path != '\0') {

        if (path[index] == ':' || path[index] == '\0') {
            path += index;
            full_path[index] = '\0';

            full_path = strcat(full_path, "/");
            full_path = strcat(full_path, program);

            struct stat buffer;
            int rc = lstat(full_path, &buffer);
            if (rc != -1) {
                return full_path;
            }

            index = 0;
            if (path[index] == ':') {
                path++;
            }
        } else {
            full_path[index] = path[index];
            index++;
        }
    }

    return NULL;
}

bool runProgram(const char * program_name, char * arguments[]) {

    char * path = getPathOfProgram(program_name);
    if (path == NULL) return false;

    struct winsize win = {
        .ws_col = 80, .ws_row = 24,
        .ws_xpixel = 480, .ws_ypixel = 192,
    };

    int outputfd = dup(1);
    int pid = forkpty(&masterfd, NULL, NULL, &win);
    if (pid == 0) {
        dup2(outputfd, 1);
        dup2(outputfd, 2);
        execv(path, arguments);
        exit(1);
    }

    close(outputfd);

    free(path);

    return true;
}

int main(int argc, char * argv[]) {

    //setvbuf(stdin, NULL, _IONBF, 0);
    setbuf(stdin,NULL);
   
    if (argc < 2) {
        fprintf(stderr, "ERROR: Incorrect usage. Usage: <%s> <program> [arguments]\n",
                argv[0]);
        return 1;
    }
    if (!runProgram(argv[1], argv+1)) {
        fprintf(stderr, "ERROR: Could not find program %s. Usage: <%s> <program> [arguments]\n",
                argv[1], argv[0]);
        return 1;
    }

    loadLines(argv[1]);

    char * line = calloc(1024, sizeof(char));

    int index = 0;
    int lineSize = 0;
    char c;

    while ((c = getch()) != EOF) {


        // Hits enter or typed too much
        if (index >= 1023 || c == '\n' || c == 10) {
            line[lineSize] = '\0';
            if (strcmp("exit", line) == 0) {
                break;
            }
            dprintf(masterfd, "%s\n", line);

            saveline(line);
            printf("\n");

            index = 0;
            lineSize = 0;
            line[0] = '\0';
        }
        // clear screen
        else if (c == 12) {
            printf("%c%c%c%c%c%c%c", 27, 91, 72, 27, 91, 50, 74);
            dprintf(masterfd, "\n");
        }
        // CTRL P
        else if (c == 16) {

            prevLineIndex--;
            const char * newLine = getprevline();
            if (*newLine != '\0') {
                swapLines(line, newLine);
                index = strlen(line);
                lineSize = strlen(line);
            }
        }
        else if (c == 27 && getch() == 91) {
            c = getch();
            if (c == 65) { // UP
                prevLineIndex--;
                const char * newLine = getprevline();
                if (*newLine != '\0') {
                    swapLines(line, newLine);
                    index = strlen(line);
                    lineSize = strlen(line);
                }
            } else if (c == 66) { // DOWN
                prevLineIndex++;
                const char * newLine = getprevline();
                if (*newLine != '\0') {
                    swapLines(line, newLine);
                    index = strlen(line);
                    lineSize = strlen(line);
                }
            } else if (c == 67) { // RIGHT
                if (index < lineSize) {
                    fputc(24, stdout);
                    fputc(line[index], stdout);
                    index++;
                }
            } else if (c == 68) { // LEFT
                if (index > 0) {
                    index--;
                    fputc(8, stdout);
                    fputc(24, stdout);
                }
            }
        }
        // Backspace
        else if (c == 127 && index > 0) {
            index--;
            deleteCInStr(line, index);
        }
        else {
            insertCInStr(line, index, c);
            index++;
            lineSize++;
        }

    }

    free(line);
    storeLines(argv[1]);

    return 0;
}

