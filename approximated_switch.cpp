#include <wiringPi.h>
#include <stdio.h>
#include <iostream>

int create_memory_file(int steps); 
int main(int argc, char *argv[])
{
    wiringPiSetup() ;
    printf("Number of command line arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }

    int in_1 = std::stoi(argv[1]);
    // printf("Argument %d: %d\n", in_1, in_2);
    int GPIO_1 = 1;    // simulate laser phsical output
    pinMode(1, OUTPUT);
    digitalWrite(GPIO_1, in_1);
    return 0;
}