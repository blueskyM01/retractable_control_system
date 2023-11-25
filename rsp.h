#ifndef RSP_H
#define RSP_H

#include <wiringPi.h>
#include <stdio.h>
#include<iostream>
#include<fstream>
#include <string>  
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

class ntu_raspberry4B
{
    public:
    ntu_raspberry4B();
    ~ntu_raspberry4B();

    int GPIO_24 = 24;    // approximated switch input
    int GPIO_25 = 25;    // laser singal input
    int GPIO_27 = 27;    // step- output, high to low is value
    int GPIO_28 = 28;    // dir- output, high: anti-clockwise; low-clockwise
    int GPIO_29 = 29;    // en- output, low is value; high is unable; low is enable

    int read_24 = 0;     // read from gpio 24
    int read_25 = 0;     // read from gpio 25
    int extent_flag = 0; // 0: extent 激光器开关信号输入到respbrerry
    int retract_flag = 1; // 1: retract  激光器开关信号输入到respbrerry
    int sensors_retract_limint_signal = 1;  // 传感器完全收回，接近开关给出高电平
    int retract_direction_flag = 0; // 0: clockwise is retract
    int extent_direction_flag = 1; // 1: anti-clockwise is extent


    int PLC_retractable_order_previous = 0;
    int PLC_retractable_order_current = 0;

    int timer_counter = 0;
    int actived_counter = 0;
    int current_laser_state = 0; 
    int previous_laser_state = 0;

    void init(); // init gpio, motor enable
    void init_move();
    int* reset_action(); // reset retract system motion
    void retractable_action(int move_steps, double delay_time, int dir); // retract and extent move function
    int create_memory_file(int steps); // create memory file to store move steps
    int get_move_steps_from_txt_file(); // get move steps from txt file
    int plc_retractable_control(int total_steps, int PLC_retractable_order); // 
    void complete_state();

    void is_detect_reset_retractable_system_signal();
    void auto_retract(int move_steps, double retract_delay_time, double extent_delay_time);
    int read_approximated_switch();
    int read_laser_switch();
};


#endif