// #include <wiringPi.h>
// #include <stdio.h>
// #include <iostream>

// int create_memory_file(int steps); 
// int main(int argc, char *argv[])
// {
//     wiringPiSetup() ;
//     printf("Number of command line arguments: %d\n", argc);
//     for (int i = 0; i < argc; i++) {
//         printf("Argument %d: %s\n", i, argv[i]);
//     }

//     int in_1 = std::stoi(argv[1]);
//     // printf("Argument %d: %d\n", in_1, in_2);
//     int GPIO_1 = 1;    // simulate laser phsical output
//     pinMode(1, OUTPUT);
//     digitalWrite(GPIO_1, in_1);
//     return 0;
// }

#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>

using namespace std;
struct RetractableBox
{
    int heart_beat; // heart beat
    int auto_manual_switch_flag; // 0: auto; 1:manual
    int PLC_retractable_order;    // 0: extent, 1: retract
};

int main()
{
    int counter = 0;
    int socket_fd = socket(AF_INET, SOCK_STREAM,0);
    if(socket_fd == -1)
    {
        cout<<"socket 创建失败："<<endl;
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2820);
    addr.sin_addr.s_addr = inet_addr("192.168.1.151");

    int res = connect(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    if(res == -1)
    {
        cout<<"bind 链接失败："<<endl;
        exit(-1);
    }
    cout<<"bind 链接成功："<<endl;

    // write(socket_fd,"hello hebinbing",15);
    while(1)
    {
        usleep(50*1000);
        RetractableBox retract_pkg_send_to_plc;
        retract_pkg_send_to_plc.heart_beat = counter;
        retract_pkg_send_to_plc.auto_manual_switch_flag = 111;
        retract_pkg_send_to_plc.PLC_retractable_order = 112;
        int iWriteCount = write(socket_fd, &retract_pkg_send_to_plc, sizeof(retract_pkg_send_to_plc));
        counter ++;
    }


    close(socket_fd);

    return 0;
}