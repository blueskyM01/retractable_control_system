#include "rsp.h"


ntu_raspberry4B::ntu_raspberry4B()
{

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

int* ntu_raspberry4B::reset_action()
{
  static int err[2]={0};
  int timer_counter = 0;
  int actived_counter = 0;

  int current_laser_state = 0; 
  int previous_laser_state = 0;
  
  while(1)
  {
    if(timer_counter > 60)
    {
      printf("!!!!    Reset rectractable system motion: timeout, now entering system work station! \n");
      break;
    }

    // get retractable signal
    read_25 = digitalRead(GPIO_25); 
    previous_laser_state = current_laser_state;
    current_laser_state = read_25;

    if(previous_laser_state == extent_flag && current_laser_state == retract_flag)
    {
      actived_counter += 1;
      
    }
    printf("Once again please! Index: %d / 3 \n", actived_counter);

    if(actived_counter == 3)
    {
      printf("****************************************************************************** \n");
      printf("*************                                                    ************* \n");
      printf("*************    Enter reset retractable system motion stage!    ************* \n");
      printf("*************                                                    ************* \n");
      printf("****************************************************************************** \n");
      
      while(1)
      {
        printf("reset retractable system motion: actived! Please wait until arrive! \n");
        int move_steps = 0;
        while(1)
        {
          read_25 = digitalRead(GPIO_25); 
          previous_laser_state = current_laser_state;
          current_laser_state = read_25;

          
          digitalWrite(GPIO_27, LOW);
          delay(50);
          digitalWrite(GPIO_27, HIGH);
          delay(50);
          
          move_steps += 1;
          printf("Motor is moving now! If arrived, please move barrier away! Index: % d \n", move_steps);

          if(previous_laser_state == retract_flag && current_laser_state == extent_flag)
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

        delay(1000);
      }
    }
    else
    {
      printf("------------    Waiting for 'reset retractable system motion' active signal! Index: %d / 60     ------------ \n", timer_counter);
    }

    delay(1000);
    timer_counter += 1;
  }
  int memory_move_steps = get_move_steps_from_txt_file();
  err[0] = 1;
  err[1] = memory_move_steps;
  return err;    
}

int ntu_raspberry4B::create_memory_file(int steps)
{

  std::ofstream myfile("../qcmemory.txt"); 
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
  std::ifstream myfile("../qcmemory.txt"); 
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

void ntu_raspberry4B::retractable_action(int move_steps, double delay_time, int dir)
{
  int print_counter = 0;
  int test_counter = 0;
  digitalWrite(GPIO_28, dir);    // dir LOW: clockwise, HIGH: anti-clockwise
  digitalWrite(GPIO_29, LOW);   // enable
  for(int i=0; i<move_steps; i++)
  {
    if(i < 32)
    {
      digitalWrite(GPIO_27, HIGH);
      digitalWrite(GPIO_27, LOW);
      delay(10);
      test_counter += 1;
    }
    else if (i < move_steps-32)
    {
      digitalWrite(GPIO_27, HIGH);
      digitalWrite(GPIO_27, LOW);
      delay(delay_time);
      test_counter += 1;
    }
    else
    {
      digitalWrite(GPIO_27, HIGH);
      digitalWrite(GPIO_27, LOW);
      delay(10);
      test_counter += 1;
    }

    if(print_counter % 100 == 1)
    {
      printf("motor is moving. \n");
    }
    else if(print_counter % 100 == 50)
    {
      printf("motor is moving. . \n");
    }
    else if(print_counter % 100 == 99)
    {
      printf("motor is moving. . . \n");
    }
    print_counter += 1;
    

  }
  digitalWrite(GPIO_27, LOW); // Duo to high to low is value, we set low at the end
  if(dir == retract_direction_flag)
  {
    printf("One cycle done! Retract move %d steps \n", test_counter);
  }
  else
  {
    printf("One cycle done! Extent move %d steps \n", test_counter);
  }
  
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
