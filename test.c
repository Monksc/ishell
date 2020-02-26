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
#include <pthread.h>

#include <termios.h>
#include <unistd.h>

typedef enum {false, true} bool;

int input_fd[2];

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

void * checkForKeys(void * args) {
    char c;
    while ((c = getch()) != '\0') {
        printf("THREAD: %d %c\n", c, c);
        dprintf(input_fd[1], "%c", c);
    }

    return NULL;
}


int main(int argc, const char * argv[]) {

    char * str = malloc(1024);

    read(0, str, 1023);
    fgets(str, 1023, stdin);

    pipe(input_fd);

    pthread_t tid;
    pthread_create(&tid, NULL, checkForKeys, NULL);
    pthread_detach(tid);


    int size;
    while ((size = read(input_fd[0], str, 1023)) != -1) {
        str[size] = '\0';
        printf("MAIN: |%s|\n", str);   
    }

    return 0;
}
