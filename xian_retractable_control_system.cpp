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

std::string ip = "192.168.1.151";
int port = 2820;
int iReadCount = 0;

int laser_signal_previous = 0;
int laser_signal_current = 0;

int retractable_motion_flag = 0;
int retractable_box_status = 0;
int error_code = 0;

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
      while(retractable_box.read_approximated_switch() == retractable_box.extent_flag )
      {
        if(retract_step > 3 * total_steps)
        {
          error_code = 9001;
          break;
        }
        retract_step = retractable_box.retractable_action(total_steps * 2, 0.08, retractable_box.retract_direction_flag);
      }
    } 
    else // automation mode
    {
      PLC_retractable_order = 0;
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


      if(retractable_box.read_approximated_switch() == retractable_box.retract_flag && retractable_box.read_laser_switch() == retractable_box.extent_flag)
      {
        motor_motion_extent_counter ++;
        if(motor_motion_extent_counter >= 20)
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

  }


  return 0;
}




int tcp_get_from_plc()
{
  char const* server_ip = ip.c_str();
  int socket_fd = -1;
  bool connect_flag = false;
  struct timeval tvTimeout;
  struct timeval tvTimeout_recv;
  int res;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(server_ip);

  int counter = 0;

  struct RetractableBoxToServer
  {
      int tcp_retrable_box_heart_beat = 0;    // tcp client heart beat
      int retractable_motion_flag = 0;  // 0: stop 1: in processing
      int retractable_box_status = 0;   // 0: extent, 1: retract
      int error_code = 9000;               // 9001: approximated switch error
  };

  struct ServerToRetractableBox
  {
      int tcp_server_heart_beat = 0;       // heart beat
      int auto_manual_switch_flag = 0;     // 0: auto; 1:manual
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
        if(tcp_client_heart_beat > 1000)
        {
          tcp_client_heart_beat = 0;
        }
        tcp_client_heart_beat ++;

        int iWriteCount = 0;
        RetractableBoxToServer send_to_server;
        send_to_server.tcp_retrable_box_heart_beat = tcp_client_heart_beat;
        send_to_server.retractable_motion_flag = retractable_motion_flag;
        send_to_server.retractable_box_status = retractable_box_status;
        send_to_server.error_code = error_code;
        iWriteCount = write(socket_fd, &send_to_server, sizeof(send_to_server));



        ServerToRetractableBox read_from_server;
        iReadCount = read(socket_fd, &read_from_server, sizeof(read_from_server));
        tcp_server_heart_beat = read_from_server.tcp_server_heart_beat; 
        auto_manual_switch_flag = read_from_server.auto_manual_switch_flag; // 0: auto; 1:manual

        // std::cout << "recevie from plc:" << std::endl;
        if( tcp_server_heart_beat % 30 == 0)
        {
          printf("tcp_retrable_box_heart_beat_tl: %d \n", tcp_client_heart_beat);
          printf("retractable_motion_flag: %d \n", retractable_motion_flag);
          printf("retractable_box_status: %d \n", retractable_box_status);
          printf("error_code: %d \n", error_code);
          printf("tcp_server_heart_beat: %d \n", tcp_server_heart_beat);
          printf("auto_manual_switch_flag: %d \n", auto_manual_switch_flag);
          printf("--------------------------------------------------------------------");
        }

        

        if(iReadCount <= 0)
        {
            connect_flag = false;
        }
    }
  }
  return 0;
}



int main()
{
  // 创建两个线程并分别执行 threadFunction1 和 threadFunction2
  std::thread rb_main(work_signal);
  std::thread rb_tcp(tcp_get_from_plc);
  std::thread rb_motor(motor_motion);

  // 等待两个线程完成
  rb_main.join();
  rb_tcp.join();
  rb_motor.join();

  std::cout << "Both threads have finished." << std::endl;

  return 0;
}

