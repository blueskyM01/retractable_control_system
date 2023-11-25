#include "rsp.h"
#include <thread>

int tcp_server_heart_beat = 0;
int tcp_client_heart_beat = 0; 
int auto_manual_switch_flag = 0; // 0: auto; 1:manual
int PLC_retractable_order = 0;    // 0: extent, 1: retract
int reset_action_flag = 0; // reset retractable box motor flag
int total_steps = 0;  // 点击在整个伸出、或者缩回运动的步数。
ntu_raspberry4B retractable_box;


std::string ip = "10.87.131.50";
int port = 2410;
int iReadCount = 0;

int auto_manual_switch_flag_previous = 0;
int auto_manual_switch_flag_current = 0;
int PLC_retractable_order_previous = 0;
int PLC_retractable_order_current = 0;

// int work_signal()
// {

//   while(1)
//   {
//     retractable_box.is_detect_reset_retractable_system_signal();
//     if(retractable_box.actived_counter == 4) // if detect reset action signal
//     {
//       if(retractable_box.read_approximated_switch() != retractable_box.sensors_retract_limint_signal)
//       {
//         reset_action_flag = 1;
//         retractable_box.actived_counter = 0;
//       }
//       else
//       {
//         retractable_box.actived_counter = 0;
//         printf("*********------Sensor in extented stage, can't enter reset system mode!------ \n*********");
//       }
//     }
//   }
//   return 0 ;
// }

int motor_motion()
{
  // init
  retractable_box.init_move();
  total_steps = retractable_box.get_move_steps_from_txt_file();
  while(1)
  {
    usleep(30*1000);
    if(auto_manual_switch_flag == 1) // manual mode
    {

    } 
    else // automation mode
    {
      if(retractable_box.read_approximated_switch() != retractable_box.sensors_retract_limint_signal)
      {
        retractable_box.auto_retract(total_steps, 0.1, 0.6);
      }
    }
  }


  return 0;
}

int main()
{
  // 创建两个线程并分别执行 threadFunction1 和 threadFunction2
//   std::thread rb_main(work_signal);
//   std::thread rb_tcp(tcp_get_from_plc);
  std::thread rb_motor(motor_motion);

  // 等待两个线程完成
//   rb_main.join();
//   rb_tcp.join();
  rb_motor.join();

  std::cout << "Both threads have finished." << std::endl;

  return 0;
}

