//
//  output.c
//  HW2
//
//  Created by Cameron Monks on 02/25/2020.
//  Copyright © 2020 Cameron Monks. All rights reserved.
//

#include <stdio.h> 
#include <stdlib.h> 

int main(int argc, const char * argv[]) {

    char c;
    while ((c = getchar()) != EOF) {
        putchar(c);
    }

    return 0;
}

