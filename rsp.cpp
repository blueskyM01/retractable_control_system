#include "rsp.h"


ntu_raspberry4B::ntu_raspberry4B()
{
  wiringPiSetup();
  
  // set gpio mode
  pinMode(GPIO_24, INPUT);
  pinMode(GPIO_25, INPUT);
  pinMode(GPIO_27, OUTPUT);
  pinMode(GPIO_28, OUTPUT);
  pinMode(GPIO_29, OUTPUT);

  // init gpio, motor enable
  init();
  std::cout << "#############    Init retractable system successful!    #############" << std::endl;
}

ntu_raspberry4B::~ntu_raspberry4B()
{
  
}

void ntu_raspberry4B::init()
{
  digitalWrite(GPIO_27, LOW);   // step High to low is value
  digitalWrite(GPIO_28, LOW);   // dir clockwise
  digitalWrite(GPIO_29, LOW);   // enable

}

void ntu_raspberry4B::init_move()
{
  if(read_approximated_switch() != sensors_retract_limint_signal)
  {
    digitalWrite(GPIO_28, retract_direction_flag);    // dir LOW: clockwise, HIGH: anti-clockwise
    digitalWrite(GPIO_29, LOW);   // enable
    while(1)
    {
      // usleep(1000 * 1000);
      if(read_approximated_switch() != sensors_retract_limint_signal)
      {
        // usleep(1 * 1000); // delay 1ms
        // digitalWrite(GPIO_27, HIGH);
        // usleep(1 * 1000); // delay 1ms
        // digitalWrite(GPIO_27, LOW);
        retractable_action(6000, 1, retract_direction_flag);
        // printf("tttttttttttttttttttttttttttttt \n");
      }
      else
      {
        retractable_action(get_move_steps_from_txt_file(), 1, extent_direction_flag);
        printf("!!!!    Rectractable system start successfully! Total step: %d      !!!!\n", get_move_steps_from_txt_file());
        break;
      }
    }
  }
  else
  {
    retractable_action(get_move_steps_from_txt_file(), 1, extent_direction_flag);
    printf("!!!!    Rectractable system start successfully!     !!!!\n");
    printf("Total step: %d \n", get_move_steps_from_txt_file());
    // printf("oooooooooooooooooooooooooooooooooooooooooo \n");
  }

}


void ntu_raspberry4B::is_detect_reset_retractable_system_signal()
{
  usleep(50 * 1000); // delay 50ms
  if(timer_counter > 100*6)
  {
    // printf("!!!!    Reset rectractable system motion: timeout, now entering system work station! \n");
    actived_counter = 0;
    timer_counter = 0;
  }
  timer_counter ++;
  
  read_25 = digitalRead(GPIO_25); 
  previous_laser_state = current_laser_state;
  current_laser_state = read_25;

  if(previous_laser_state == extent_flag && current_laser_state == retract_flag)
  {
    actived_counter += 1;
  }
  if(timer_counter % 20 == 0)
  {
    printf("***********    Once again please! Index: %d / 4    ***********\n", actived_counter);
  }
  
  if(actived_counter == 4 && read_approximated_switch() != sensors_retract_limint_signal)
  {
    printf("****************************************************************************** \n");
    printf("*************                                                    ************* \n");
    printf("*************    Enter reset retractable system motion stage!    ************* \n");
    printf("*************                                                    ************* \n");
    printf("****************************************************************************** \n");
  }

}

int ntu_raspberry4B::read_approximated_switch()
{
  return digitalRead(GPIO_24);
}

int ntu_raspberry4B::read_laser_switch()
{
  return digitalRead(GPIO_25);
}


int* ntu_raspberry4B::reset_action()
{
  static int err[2]={0};
  int move_steps = 0;
  digitalWrite(GPIO_28, retract_direction_flag);    // dir LOW: clockwise, HIGH: anti-clockwise
  digitalWrite(GPIO_29, LOW);   // enable
    
  while(1)
  {    
    digitalWrite(GPIO_27, HIGH);
    digitalWrite(GPIO_27, LOW);
    delay(50);
    
    move_steps += 1;
    printf("Motor is moving now! Index: % d \n", move_steps);

    read_24 = read_approximated_switch(); 
    if(read_24 == sensors_retract_limint_signal)
    {
      printf("#######*********    Reset retractable system motion successfully!    *********####### \n");
      int write_flag = create_memory_file(move_steps);
      if(write_flag == 0)
      {
        printf("create and write move steps successfully! \n");
      }
      else
      {
        printf("create and write move steps faild! \n");
      }

      err[0] = 0;
      err[1] = move_steps;
      return err;
    }
  }

  // int memory_move_steps = get_move_steps_from_txt_file();
  // err[0] = 1;
  // err[1] = memory_move_steps;
  // return err;    
}

int ntu_raspberry4B::create_memory_file(int steps)
{

  std::ofstream myfile("/home/psa/aQC/retractable_control_system/qcmemory.txt"); 
  if (!myfile.is_open())  
  {  
      std::cout << "未成功打开文件" << std::endl;  
      return 1;
  }

  myfile << std::to_string(steps);
  myfile.close(); 
  return 0;
}

int ntu_raspberry4B::get_move_steps_from_txt_file()
{
  std::ifstream myfile("/home/psa/aQC/retractable_control_system/qcmemory.txt"); 
  if (!myfile.is_open())  
  {  
      std::cout << "未成功打开文件" << std::endl;  
      return -1;
  }

  std::string line; 
  int move_steps; 
  while(std::getline(myfile, line))
  {
    move_steps = std::stoi(line);
    break;
  }
  myfile.close(); 
  return move_steps;
}

int ntu_raspberry4B::retractable_action(int move_steps, double delay_time, int dir)
{
  int print_counter = 0;
  int test_counter = 0;
  int ad = 100;
  digitalWrite(GPIO_28, dir);    // dir LOW: clockwise, HIGH: anti-clockwise
  digitalWrite(GPIO_29, LOW);   // enable
  for(int i=0; i<move_steps; i++)
  { 
    if(dir == retract_direction_flag && read_approximated_switch() == retract_flag)
    {
      // printf("dir: %d, read_approximated_switch: %d \n", retract_direction_flag, retract_flag);
      break;
    }

    if(dir == extent_direction_flag && emergence_stop == 1)
    {
      emergence_stop = 0;
      printf("Emergence stop!");
      break;
    }

    if(i < ad)
    {
      usleep(1 * 1000 - ((1-delay_time) * 1000 *(i/ad))); // delay 1ms
      digitalWrite(GPIO_27, HIGH);
      // usleep(1 * 1000); // delay 1ms
      usleep(1 * 1000 - ((1-delay_time) * 1000 *(i/ad))); // delay 1ms
      digitalWrite(GPIO_27, LOW);
      
      test_counter += 1;
    }
    else if (i > move_steps-ad)
    {
      // usleep(1 * 1000); // delay 1ms
      usleep(delay_time * 1000 + ((1-delay_time) * 1000 *((ad-move_steps+i)/ad))); // delay 1ms
      digitalWrite(GPIO_27, HIGH);
      // usleep(1 * 1000); // delay 1ms
      usleep(delay_time * 1000 + ((1-delay_time) * 1000 *((ad-move_steps+i)/ad))); // delay 1ms
      digitalWrite(GPIO_27, LOW);
      test_counter += 1;
    }
    else
    {
      usleep(delay_time * 1000); // delay 1ms
      digitalWrite(GPIO_27, HIGH);
      usleep(delay_time * 1000); // delay 1ms
      digitalWrite(GPIO_27, LOW);
      test_counter += 1;
    }

  //   if(print_counter % 100 == 1)
  //   {
  //     printf("motor is moving. \n");
  //   }
  //   else if(print_counter % 100 == 50)
  //   {
  //     printf("motor is moving. . \n");
  //   }
  //   else if(print_counter % 100 == 99)
  //   {
  //     printf("motor is moving. . . \n");
  //   }
  //   print_counter += 1;
  }



  digitalWrite(GPIO_27, LOW); // Duo to high to low is value, we set low at the end
  if(dir == retract_direction_flag)
  {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!    One cycle done! Retract move %d steps.    !!!!!!!!!!!!!!!!!!!!!!!!!!!\n", test_counter);
  }
  else
  {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!    One cycle done! Extent move %d steps.    !!!!!!!!!!!!!!!!!!!!!!!!!!! \n", test_counter);
  }

  return test_counter;
  
}

int ntu_raspberry4B::plc_retractable_control(int total_steps, int PLC_retractable_order)
{
  int static print_flag = 0;
  // PLC_retractable_order:  TCP order from plc, 0: extent, 1: retract

  while(1)
  {
    PLC_retractable_order_previous = PLC_retractable_order_current;
    PLC_retractable_order_current = PLC_retractable_order;
    std::cout << "o-PLC_retractable_order_previous: " << PLC_retractable_order_previous << "  PLC_retractable_order_current: " << PLC_retractable_order_current << std::endl;
    if(PLC_retractable_order_previous == 0 &&  PLC_retractable_order == 1)
    {
      retractable_action(total_steps, 100, retract_direction_flag); // retract by plc
      printf("------------    Retract by plc. Done!    ------------ \n");
      while(1)
      {
        PLC_retractable_order_previous = PLC_retractable_order_current;
        PLC_retractable_order_current = PLC_retractable_order;
        std::cout << "PLC_retractable_order_previous: " << PLC_retractable_order_previous << "  PLC_retractable_order_current: " << PLC_retractable_order_current << std::endl;
        if(PLC_retractable_order_previous == 1 &&  PLC_retractable_order == 0)
        {
          retractable_action(total_steps, 100, extent_direction_flag); // extent by plc
          printf("------------    Extent by plc. Done!    ------------ \n");
          break;
        }
        else
        {
          printf("*******    Wait for extent order from plc..    *******\n");
          delay(1000);
        }
      }
      break;
    }
    else
    {
      printf("#########    Wait for retract order from plc.    #########\n");
      delay(1000);
    }
  }


  
  return 0;
}

void ntu_raspberry4B::auto_retract(int move_steps, double retract_delay_time, double extent_delay_time)
{
  read_25 = read_laser_switch(); 
  previous_laser_state = current_laser_state;
  current_laser_state = read_25;

  if(previous_laser_state == extent_flag && current_laser_state == retract_flag)
  {
    retractable_action(move_steps, retract_delay_time, retract_direction_flag);

    while(1)
    {
      usleep(30*1000);
      if(read_laser_switch() == extent_flag)
      {
        retractable_action(move_steps, extent_delay_time, extent_direction_flag);

        break;
      }
    }
    printf("previous_laser_state:%d, current_laser_state: %d \n", previous_laser_state, current_laser_state);
    printf("read_25:%d \n", read_25);

  }

  // printf("read_25:%d \n", current_laser_state);
  
}

void ntu_raspberry4B::complete_state()
{
  int my_counter = 0;
  while(1)
  {
    my_counter += 1;
    if(my_counter % 4 != 0)
    {
      digitalWrite(GPIO_27, HIGH); 
      digitalWrite(GPIO_28, HIGH); 
      digitalWrite(GPIO_29, HIGH); 
      delay(500);
      digitalWrite(GPIO_27, LOW); 
      digitalWrite(GPIO_28, LOW); 
      digitalWrite(GPIO_29, LOW); 
      delay(500);
    }
    else
    {
      digitalWrite(GPIO_27, HIGH); 
      digitalWrite(GPIO_28, HIGH); 
      digitalWrite(GPIO_29, HIGH); 
      delay(100);
      digitalWrite(GPIO_27, LOW); 
      digitalWrite(GPIO_28, LOW); 
      digitalWrite(GPIO_29, LOW); 
      delay(100);
    }

    if(my_counter >= 12)
    {
      break;
    }
    printf("This is complete state: %d / 12. \n",  my_counter);
  }
  printf("Finished! \n");
}
