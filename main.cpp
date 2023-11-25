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

int work_signal()
{

  while(1)
  {
    retractable_box.is_detect_reset_retractable_system_signal();
    if(retractable_box.actived_counter == 4) // if detect reset action signal
    {
      if(retractable_box.read_approximated_switch() != retractable_box.sensors_retract_limint_signal)
      {
        reset_action_flag = 1;
        retractable_box.actived_counter = 0;
      }
      else
      {
        retractable_box.actived_counter = 0;
        printf("*********------Sensor in extented stage, can't enter reset system mode!------ \n*********");
      }
    }
  }
  return 0 ;
}


int motor_motion()
{
  total_steps = retractable_box.get_move_steps_from_txt_file();
  printf("&&&&&&&&&&&&&&&&&&    memory total_steps:%d    &&&&&&&&&&&&&&&&&&", total_steps);
  while(1)
  {
    if(auto_manual_switch_flag == 1)
    {
      if(reset_action_flag == 1)
      {
        int* err = retractable_box.reset_action();
        total_steps = *(err+1);
        reset_action_flag = 0;
        std::cout << "@@@@@@@@@@    reset total steps to " << total_steps << "!    @@@@@@@@@@" << std::endl;

        printf("Start extent motion after reset retract system motion: \n");
        retractable_box.retractable_action(total_steps, 100, retractable_box.extent_direction_flag);  //  due to the sensor was retract after reset retract system motion, 
                                                                    //  it is need to extent.
        // retractable_box.complete_state();
      }
    }


    

    if(auto_manual_switch_flag == 1)  // manual mode
    {
      PLC_retractable_order_previous = PLC_retractable_order_current;
      PLC_retractable_order_current = PLC_retractable_order;
      if(PLC_retractable_order_previous == 0 && PLC_retractable_order_current == 1)
      {
        retractable_box.retractable_action(total_steps, 100, retractable_box.retract_direction_flag); // retract
        // retractable_box.complete_state();
        std::cout << "@@@@@@@@@@------------------------------------------- @@@@@@@@@@" << std::endl;
        std::cout << "PLC_retractable_order_previous:" << PLC_retractable_order_previous << "PLC_retractable_order_current:" << PLC_retractable_order_current << std::endl;
      } 
      else if(PLC_retractable_order_previous == 1 && PLC_retractable_order_current == 0)
      {
        retractable_box.retractable_action(total_steps, 100, retractable_box.extent_direction_flag); // extent
        // retractable_box.complete_state();
        std::cout << "***********-------------------------------------------***********" << std::endl;
      }
    }
    else // automation mode
    {
      if(retractable_box.read_approximated_switch() != retractable_box.sensors_retract_limint_signal)
      {
        retractable_box.auto_retract(total_steps, 1, 0.6);
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
        
        // std::cout << counter << std::endl;

        
        RetractableBox retract_pkg_read_from_plc;
        iReadCount = read(socket_fd, &retract_pkg_read_from_plc, sizeof(retract_pkg_read_from_plc));
        tcp_server_heart_beat = retract_pkg_read_from_plc.heart_beat; 
        auto_manual_switch_flag = retract_pkg_read_from_plc.auto_manual_switch_flag; // 0: auto; 1:manual
        PLC_retractable_order = retract_pkg_read_from_plc.PLC_retractable_order;    // 0: extent, 1: retract

        // std::cout << "recevie from plc:" << std::endl;
        if( tcp_server_heart_beat % 30 == 0)
        {
          std::cout << "receive data length: " << iReadCount << std::endl;
          std::cout << "tcp_server_heart_beat: " << tcp_server_heart_beat << std::endl;
        }
        
        // std::cout << "retract_pkg_read_from_plc.auto_manual_switch_flag: " << auto_manual_switch_flag << std::endl;
        // std::cout << "retract_pkg_read_from_plc.PLC_retractable_order: " << PLC_retractable_order << std::endl;

        if(tcp_client_heart_beat > 1000)
        {
          tcp_client_heart_beat = 0;
        }
        tcp_client_heart_beat ++;
        int iWriteCount = 0;
        RetractableBox retract_pkg_send_to_plc;
        retract_pkg_send_to_plc.heart_beat = tcp_client_heart_beat;
        retract_pkg_send_to_plc.auto_manual_switch_flag = 111;
        retract_pkg_send_to_plc.PLC_retractable_order = 112;
        iWriteCount = write(socket_fd, &retract_pkg_send_to_plc, sizeof(retract_pkg_send_to_plc));
        // std::cout << "send to plc:" << std::endl;
        // std::cout << "retract_pkg_send_to_plc.heart_beat: " << retract_pkg_send_to_plc.heart_beat << std::endl;
        // std::cout << "retract_pkg_send_to_plc.auto_manual_switch_flag: " << retract_pkg_send_to_plc.auto_manual_switch_flag << std::endl;
        // std::cout << "retract_pkg_send_to_plc.PLC_retractable_order: " << retract_pkg_send_to_plc.PLC_retractable_order << std::endl;

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

  std::cout << "Both threads have finished." << std::endl;

  return 0;
}
