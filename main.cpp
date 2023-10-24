#include <wiringPi.h>
#include <stdio.h>
#include<iostream>
#include<fstream>
#include <string>  
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

void init(); // init gpio, motor enable
int* reset_action(); // reset retract system motion
void retractable_action(int move_steps, double delay_time, int dir); // retract and extent move function
int create_memory_file(int steps); // create memory file to store move steps
int get_move_steps_from_txt_file(); // get move steps from txt file
int plc_retractable_control(int total_steps); // 

void complete_state();

int GPIO_25 = 25;    // laser singal input
int GPIO_27 = 27;    // step- output, high to low is value
int GPIO_28 = 28;    // dir- output, high: anti-clockwise; low-clockwise
int GPIO_29 = 29;    // en- output, low is value; high is unable; low is enable

int read_25 = 0;     // read from gpio 25
int extent_flag = 0; // 0: extent 
int retract_flag = 1; // 1: retract; 
int retract_direction_flag = 0; // 0: clockwise is retract
int extent_direction_flag = 1; // 1: anti-clockwise is extent


int main(void)
{
  wiringPiSetup() ;

  // set gpio mode
  pinMode(GPIO_25, INPUT);
  pinMode(GPIO_27, OUTPUT);
  pinMode(GPIO_28, OUTPUT);
  pinMode(GPIO_29, OUTPUT);

  


  // init gpio, motor enable
  init();

  // -----------------------        reset retract system motion        -----------------------
  int* err = reset_action();
  int reset_flag = *(err+0); // 0: with reset; 1: without reset
  int total_steps = *(err+1);
  printf("Total steps: %d. \n", total_steps);
  if(reset_flag == 0)
  {
    printf("Start extent motion after reset retract system motion: \n");
    retractable_action(total_steps, 100, extent_direction_flag);  //  due to the sensor was retract after reset retract system motion, 
                                                                  //  it is need to extent.
  }
  // -----------------------        reset retract system motion        -----------------------
  
  // while(1)
  // {
  //   // int auto_manual_switch_flag = 0; // TCP order from plc, 0: auto; 1:manual
  //   // int auto_manual_switch_flag_previous = 0;
  //   // int auto_manual_switch_flag_current = 0;
  //   // auto_manual_switch_flag_previous = auto_manual_switch_flag_current;
  //   // auto_manual_switch_flag_current = auto_manual_switch_flag;

  //   plc_retractable_control(total_steps);
  //   // if(auto_manual_switch_flag_previous == 0 && auto_manual_switch_flag_current == 1) // enter maunal mode
  //   // {
  //   //   plc_retractable_control(total_steps);
  //   // }

  //   // if(auto_manual_switch_flag_previous == 1 && auto_manual_switch_flag_current == 0) // enter auto mode
  //   // {

  //   // }
  // }


  complete_state();

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

void init()
{
  digitalWrite(GPIO_27, LOW);   // step High to low is value
  digitalWrite(GPIO_28, LOW);   // dir clockwise
  digitalWrite(GPIO_29, LOW);   // enable
}

int* reset_action()
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

int create_memory_file(int steps)
{

  std::ofstream myfile("../qcmemory.txt"); 
  if (!myfile.is_open())  
  {  
      std::cout << "未成功打开文件" << std::endl;  
      return 1;
  }

  myfile << std::to_string(steps);
  myfile.close(); 

  // FILE *file = fopen("qcmemory.txt", "w");

  // // 检查文件是否成功打开
  // if (file == NULL) {
  //     printf("无法打开文件\n");
  //     return 1;
  // }

  // char buffer[10]; // 存储字符串的缓冲区
  // // 使用 sprintf 将整数转换为字符串
  // sprintf(buffer, "%d", steps);

  // // 写入文本到文件
  // fprintf(file,  "%s\n", buffer);
  // fclose(file); // 关闭文件
  return 0;
}

int get_move_steps_from_txt_file()
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

  // FILE *file = fopen("qcmemory.txt", "r"); // 打开文本文件以进行读取

  // if (file == NULL) {
  //     printf("无法打开文件\n");
  //     return -1;
  // }

  // int move_steps;
  // while (fscanf(file, "%d", &move_steps) != EOF) {
  //     printf("memory move steps is: %d\n", move_steps); // 在这里你可以对读取到的整数执行任何操作
  //     break;
  // }

  // fclose(file); // 关闭文件

  return move_steps;
}

void retractable_action(int move_steps, double delay_time, int dir)
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

int plc_retractable_control(int total_steps)
{
  int static print_flag = 0;
  int PLC_retractable_order = 0; // TCP order from plc, 0: extent, 1: retract
  int PLC_retractable_order_previous = 0;
  int PLC_retractable_order_current = 0;
  PLC_retractable_order_previous = PLC_retractable_order_current;
  PLC_retractable_order_current = PLC_retractable_order;

 
  if(PLC_retractable_order_previous == 0 &&  PLC_retractable_order == 1)
  {
    retractable_action(total_steps, 100, retract_direction_flag); // retract by plc
    printf("------------    Retract by plc. Done!    ------------ \n");
    while(1)
    {
      if(PLC_retractable_order_previous == 1 &&  PLC_retractable_order == 0)
      {
        retractable_action(total_steps, 100, extent_direction_flag); // extent by plc
        printf("------------    Extent by plc. Done!    ------------ \n");
        break;
      }
      else
      {
        if(print_flag == 0)
        {
          printf("Wait for extect order from plc.  \n");
          print_flag = 1;
        }
        else
        {
          printf("Wait for extect order from plc..  \n");
          print_flag = 0;
        }
        
        delay(1000);
      }
    }
  }
  else
  {
    if(print_flag == 0)
    {
      printf("Wait for retract order from plc. \n");
      print_flag = 1;
    }
    else 
    {
      printf("Wait for retract order from plc.. \n");
      print_flag = 0;
    }
    
    delay(1000);

  }
  return 0;
}

void complete_state()
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
