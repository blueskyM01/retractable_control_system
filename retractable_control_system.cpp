#include "rsp.h"
#include <thread>

int tcp_server_heart_beat = 0;
int tcp_client_heart_beat = 0; 
int auto_manual_switch_flag = 0; // 0: auto; 1:manual
int PLC_retractable_order = 0;    // 0: extent, 1: retract
int reset_action_flag = 0; // reset retractable box motor flag
int total_steps = 0;  // 点击在整个伸出、或者缩回运动的步数。
int retract_move = 0; // 0:stop; 1:move
int extent_move = 0; // 0:stop; 1:move
ntu_raspberry4B retractable_box;
int motor_motion_extent_counter = 0;
int motor_motion_retract_counter = 0;
int motor_motion_retract_counter_1 = 0;
int retract_step = 2000;

std::string ip = "10.87.131.50";
int port = 2410;
int iReadCount = 0;

int auto_manual_switch_flag_previous = 0;
int auto_manual_switch_flag_current = 0;
int PLC_retractable_order_previous = 0;
int PLC_retractable_order_current = 0;

int laser_signal_previous = 0;
int laser_signal_current = 0;

int work_signal()
{

  while(1)
  {
    usleep(5*1000);
    laser_signal_previous = laser_signal_current;
    laser_signal_current = retractable_box.read_laser_switch();

    if(laser_signal_previous == retractable_box.extent_flag && laser_signal_current == retractable_box.retract_flag) // impact is detected
    {
      retract_move = 1;
    }

    if(laser_signal_previous == retractable_box.retract_flag && laser_signal_current == retractable_box.extent_flag) // impact disappear
    {
      extent_move = 1;
    }

    if(retract_move == 1 && extent_move == 1)
    {
      retractable_box.emergence_stop = 1;
    }

    
  }
  return 0 ;
}

int motor_motion()
{
  // init
  retractable_box.init_move();
  total_steps = retractable_box.get_move_steps_from_txt_file();
  retract_step = total_steps;
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~retract_step: %d \n", retract_step);


  while(1)
  {
    usleep(30*1000);
    if(auto_manual_switch_flag == 1) // manual mode
    {

    } 
    else // automation mode
    {
      if(retract_move ==1)
      {
        retract_step = retractable_box.retractable_action(total_steps * 2, 0.08, retractable_box.retract_direction_flag);
        retract_move = 0;

      }

      if(extent_move ==1)
      {
        retract_step = retractable_box.retractable_action(total_steps, 0.5, retractable_box.extent_direction_flag);
        extent_move = 0;
        
      }
    }
    
    if(retractable_box.read_approximated_switch() == retractable_box.retract_flag && retractable_box.read_laser_switch() == retractable_box.extent_flag)
    {
      motor_motion_extent_counter ++;
      if(motor_motion_extent_counter >= 80)
      {
        retract_step = retractable_box.retractable_action(total_steps, 0.5, retractable_box.extent_direction_flag);
        extent_move = 0;
      }
    }
    else
    {
      motor_motion_extent_counter = 0;
    }

    if(retractable_box.read_approximated_switch() == retractable_box.extent_flag && 
       retractable_box.read_laser_switch() == retractable_box.extent_flag &&
       retract_step < total_steps*0.7)
    {
      motor_motion_retract_counter ++;
      if(motor_motion_retract_counter > 10)
      {
        printf("```````````````````````````````````````````````````````````retract_step: %d \n", retract_step);
        retractable_box.retractable_action(total_steps * 2, 0.1, retractable_box.retract_direction_flag);
        retract_step = total_steps;
        retract_move = 0;
      }
    }
    else
    {
      motor_motion_retract_counter = 0;
    }

    if(retractable_box.read_approximated_switch() == retractable_box.extent_flag && 
       retractable_box.read_laser_switch() == retractable_box.retract_flag)
    {
      motor_motion_retract_counter_1 ++;
      if(motor_motion_retract_counter_1 > 10)
      {
        retractable_box.retractable_action(total_steps * 2, 0.1, retractable_box.retract_direction_flag);
        retract_step = total_steps;
        retract_move = 0;
      }
    }
    else
    {
      motor_motion_retract_counter_1 == 0;
    }
    

  }



  // while(1)
  // {
  //   usleep(10*1000);
  //   if(auto_manual_switch_flag == 1) // manual mode
  //   {

  //   } 
  //   else // automation mode
  //   {
  //     if(retractable_box.read_approximated_switch() != retractable_box.sensors_retract_limint_signal)
  //     {
  //       retractable_box.auto_retract(total_steps, 0.1, 0.6);
  //     }
  //   }
  // }


  return 0;
}

int main()
{
  // 创建两个线程并分别执行 threadFunction1 和 threadFunction2
  std::thread rb_main(work_signal);
//   std::thread rb_tcp(tcp_get_from_plc);
  std::thread rb_motor(motor_motion);

  // 等待两个线程完成
  rb_main.join();
//   rb_tcp.join();
  rb_motor.join();

  std::cout << "Both threads have finished." << std::endl;

  return 0;
}

