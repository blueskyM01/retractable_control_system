#include "rsp.h"
#include <thread>

int tcp_server_heart_beat = 0;
int tcp_client_heart_beat = 0; 
int auto_manual_switch_flag = 0; // 0: auto; 1:manual
int PLC_retractable_order = 0;    // 0: extent, 1: retract

std::string ip = "10.87.131.50";
int port = 2410;
int iReadCount = 0;

int auto_manual_switch_flag_previous = 0;
int auto_manual_switch_flag_current = 0;

int retractable_box()
{
  wiringPiSetup();
  ntu_raspberry4B retractable_box;

  // set gpio mode
  pinMode(retractable_box.GPIO_25, INPUT);
  pinMode(retractable_box.GPIO_27, OUTPUT);
  pinMode(retractable_box.GPIO_28, OUTPUT);
  pinMode(retractable_box.GPIO_29, OUTPUT);

  // init gpio, motor enable
  std::cout << "tcp data length: " << iReadCount << std::endl;
  retractable_box.init();

  // -----------------------        reset retract system motion        -----------------------
  int* err = retractable_box.reset_action();
  int reset_flag = *(err+0); // 0: with reset; 1: without reset
  int total_steps = *(err+1);
  printf("Total steps: %d. \n", total_steps);
  if(reset_flag == 0)
  {
    printf("Start extent motion after reset retract system motion: \n");
    retractable_box.retractable_action(total_steps, 100, retractable_box.extent_direction_flag);  //  due to the sensor was retract after reset retract system motion, 
                                                                  //  it is need to extent.
  }
  // -----------------------        reset retract system motion        -----------------------

  printf("system init successfully! \n");
  while(1)
  {
    
    // std::cout << "tcp data length: " << iReadCount << std::endl;
    // std::cout << "auto_manual_switch_flag_previous: " << auto_manual_switch_flag_previous << "  auto_manual_switch_flag_current: " << auto_manual_switch_flag_current << std::endl;
    // std::cout << "PLC_retractable_order: " << PLC_retractable_order << std::endl;
    

    auto_manual_switch_flag_previous = auto_manual_switch_flag_current;
    auto_manual_switch_flag_current = auto_manual_switch_flag;



    
    // from auto mode to manual mode
    if(auto_manual_switch_flag_previous == 0 && auto_manual_switch_flag_current == 1) // enter maunal mode
    {
      retractable_box.plc_retractable_control(total_steps, PLC_retractable_order);
    }

    // if(auto_manual_switch_flag_previous == 1 && auto_manual_switch_flag_current == 0) // enter auto mode
    // {

    // }
  }


  retractable_box.complete_state();

  // int counter = 0;

  // for(;;)
  // {
  //   // repetitive retract
  //   if(counter % 2 == 0)
  //   {
  //     digitalWrite(GPIO_26, LOW) ; 
  //   }
  //   else
  //   {
  //     digitalWrite(GPIO_26, HIGH) ;
  //   }
    
  //   // get retractable signal
  //   read_25 = digitalRead(25) ; 
  //   previous_laser_state = current_laser_state;
  //   current_laser_state = read_25;
    
    

  //   if(read_25 == retractable_flag)  // if detect retract
  //   {
  //     retractable_action(0.05, 200);
      
  //   }
  //   else
  //   {
  //     extent_action(0.05, 200);
  //   }

  //   delay(500);

  //   counter ++;
    
  //   printf("Num: %d, GPIO口 %d 处于低电平状态\n", counter, read_25);
  // }
  return 0 ;
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
  std::thread rb_main(retractable_box);
  std::thread rb_tcp(tcp_get_from_plc);

  // 等待两个线程完成
  rb_main.join();
  rb_tcp.join();

  std::cout << "Both threads have finished." << std::endl;

  return 0;
}
