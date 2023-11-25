#include <wiringPi.h>
#include <stdio.h>
#include <iostream>
#include "rsp.h"

int main()
{
  // 创建两个线程并分别执行 threadFunction1 和 threadFunction2

  ntu_raspberry4B retractable_box;

  // for(int i=0; i <100; i++)
  // {
  //   retractable_box.retractable_action(2200, 0.1, retractable_box.retract_direction_flag); // retract_direction_flag, extent_direction_flag

  //   retractable_box.retractable_action(2200, 0.6, retractable_box.extent_direction_flag); // retract_direction_flag, extent_direction_flag
  // }

  // retractable_box.retractable_action(2200, 1, retractable_box.extent_direction_flag); // retract_direction_flag, extent_direction_flag

  // // std::cout << "Both threads have finished." << std::endl;
  while(1)
  {
    int read_25 = digitalRead(retractable_box.GPIO_25); 
    int read_24 = digitalRead(retractable_box.GPIO_24); 

    std::cout << "switch:" << read_24 << std::endl;
    std::cout << "laser:" << read_25 << std::endl;
    usleep(1000*1000);
  }


  return 0;
}