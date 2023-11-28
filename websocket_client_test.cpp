#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>



int main()
{

    int socket_fd = -1;
    bool connect_flag = false;
    struct timeval tvTimeout;
    struct timeval tvTimeout_recv;
    int res;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2820);
    addr.sin_addr.s_addr = inet_addr("192.168.1.151");

    int counter = 0;

    struct RetractableBox
    {
        int heart_beat; // heart beat
        int auto_manual_switch_flag; // 0: auto; 1:manual
        int PLC_retractable_order;    // 0: extent, 1: retract
    };

    


    while(1)
    {
        if(socket_fd == -1 && connect_flag == false)
        {
            close(socket_fd);
            socket_fd = -1;
        }

        if(connect_flag == false)
        {
            // ------ socket ------
            std::cout << "socket 启动中......" << std::endl;
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            if(socket_fd == -1)
            {
                std::cout << "socket 创建失败：" << std::endl;
            }
            else
            {
                tvTimeout.tv_sec = 3;
                tvTimeout.tv_usec = 0;
                tvTimeout_recv.tv_sec = 1;
                tvTimeout_recv.tv_usec = 0;
                setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tvTimeout, sizeof(tvTimeout));
                setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tvTimeout_recv, sizeof(tvTimeout_recv));

                // ------判断是否连接成功------
                res = connect(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
                // ------判断是否连接成功------
                if (res != 0)
                {
                    std::cout << "res = " << res << std::endl;
                    std::cout << "bind 链接失败, 再次尝试请求!" << std::endl;
                }
                else
                {
                    connect_flag = true; 
                    std::cout << "bind 链接成功!" << std::endl;
                }
                
            } 
        }

        if(connect_flag == true)
        {
            usleep(30*1000); // 30 ms 
            counter += 1;
            std::cout << counter << std::endl;

            


            int iWriteCount = 0;
            RetractableBox retract_pkg_send_to_plc;
            retract_pkg_send_to_plc.heart_beat = counter;
            retract_pkg_send_to_plc.auto_manual_switch_flag = 111;
            retract_pkg_send_to_plc.PLC_retractable_order = 112;
            iWriteCount = write(socket_fd, &retract_pkg_send_to_plc, sizeof(retract_pkg_send_to_plc));
            std::cout << "send to plc:" << std::endl;
            std::cout << "retract_pkg_send_to_plc.index: " << retract_pkg_send_to_plc.heart_beat << std::endl;
            std::cout << "retract_pkg_send_to_plc.auto_manual_switch_flag: " << retract_pkg_send_to_plc.auto_manual_switch_flag << std::endl;
            std::cout << "retract_pkg_send_to_plc.PLC_retractable_order: " << retract_pkg_send_to_plc.PLC_retractable_order << std::endl;

            int iReadCount = 0;
            RetractableBox retract_pkg_read_from_plc;
            iReadCount = read(socket_fd, &retract_pkg_read_from_plc, sizeof(retract_pkg_read_from_plc));
            std::cout << "recevie from plc:" << std::endl;
            std::cout << "retract_pkg_read_from_plc.index: " << retract_pkg_read_from_plc.heart_beat << std::endl;
            std::cout << "retract_pkg_read_from_plc.auto_manual_switch_flag: " << retract_pkg_read_from_plc.auto_manual_switch_flag << std::endl;
            std::cout << "retract_pkg_read_from_plc.PLC_retractable_order: " << retract_pkg_read_from_plc.PLC_retractable_order << std::endl;

            // usleep(50*1000);
            // RetractableBox retract_pkg_send_to_plc;
            // retract_pkg_send_to_plc.heart_beat = counter;
            // retract_pkg_send_to_plc.auto_manual_switch_flag = 111;
            // retract_pkg_send_to_plc.PLC_retractable_order = 112;
            // int iWriteCount = write(socket_fd, &retract_pkg_send_to_plc, sizeof(retract_pkg_send_to_plc));

            if(iReadCount <= 0)
            {
                connect_flag = false;
            }
        }

    }









    // int socket_fd = socket(AF_INET, SOCK_STREAM,0);
    // if(socket_fd == -1)
    // {
    //     cout<<"socket 创建失败："<<endl;
    //     exit(-1);
    // }

    // struct sockaddr_in addr;
    // addr.sin_family = AF_INET;
    // addr.sin_port = htons(9999);
    // addr.sin_addr.s_addr = inet_addr("10.87.131.50");

    // int res = connect(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    // if(res == -1)
    // {
    //     cout<<"bind 链接失败："<<endl;
    //     exit(-1);
    // }
    // cout<<"bind 链接成功："<<endl;
    // while(1)
    // {
    //     write(socket_fd,"hello hebinbing",15);
    // }
    

    // close(socket_fd);

    return 0;
}
