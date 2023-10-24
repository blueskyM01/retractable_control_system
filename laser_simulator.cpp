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
    int in_2 = std::stoi(argv[2]);
    // printf("Argument %d: %d\n", in_1, in_2);
    int GPIO_26 = 26;    // simulate laser phsical output
    pinMode(GPIO_26, OUTPUT);

    digitalWrite(GPIO_26, in_1);
    delay(1000);
    digitalWrite(GPIO_26, in_2);

    return 0;
}


