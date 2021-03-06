#include "Obstacle.h"
#include "KalmanFilter.h"


IR IR;
Ultrasonic Ultrasonic;
extern Balanced Balanced;
extern Mpu6050 Mpu6050;

void Ultrasonic::Pin_init()
{
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
}


char Ultrasonic ::measure_flag = 0;
unsigned long Ultrasonic ::measure_prev_time = 0;
double Ultrasonic::distance_value;

void Function::Spiral_Mode(bool & obstacle_encountered)
{
  IR.Send();
  Ultrasonic.Get_Distance();

  if (millis() - follow_prev_time >= 100)
  {
    spiral_time = millis();
    //start with 1500ms forward movement, increases from there
    while (millis() - spiral_time < 1500 + forward_time) {
      Balanced.Motion_Control(FORWARD);

      if (OBSTACLE_JUDAGEMENT || If_IR_TRIGGERED) {
        obstacle_encountered = true;
        return; //break out of spiral mode if obstacle gets encountered
      }
    }
    forward_time += 750; //extend forward movement by 750ms each leg
    turn_90 = millis();
    while (millis() - turn_90 < 1825) { //1825ms ~ 90 degree turn
      Balanced.Motion_Control(RIGHT);
    }
  }
}

void Function::Obstacle_Mode()
{
  IR.Send();
  Ultrasonic.Get_Distance();

  if (millis() - follow_prev_time >= 100)
  {
    follow_prev_time = millis();
    Balanced.Motion_Control(FORWARD);

    while (OBSTACLE_JUDAGEMENT || If_IR_TRIGGERED)
    {
      Balanced.Motion_Control(STOP);
      if (millis() - Obstacle_time > 10000)
      { 
        Obstacle_time = millis();
        Back_time = millis();
        while (millis() - Back_time < 500)
        {
          Balanced.Motion_Control(BACK);
        }
      }
      Turning_Time = millis();
      while (millis() - Turning_Time < 500)
      {
        if (turn_flag)
        { turn_flag = 0;
          IR.Check();
        }
      }
      turn_flag = 1;
    }

  }
  Obstacle_time = millis();
}

void Function::Follow_Mode()
{
  IR.Send();
  Ultrasonic.Get_Distance();

  if (millis() - follow_prev_time >= 100)
  {
    follow_prev_time = millis();
    Balanced.Motion_Control(FORWARD);

    while (OBSTACLE_JUDAGEMENT || If_IR_TRIGGERED)
    {
      Balanced.Motion_Control(STOP);
      if (millis() - Obstacle_time > 10000)
      { 
        Obstacle_time = millis();
        Back_time = millis();
        while (millis() - Back_time < 500)
        {
          Balanced.Motion_Control(BACK);
        }
      }
      Turning_Time = millis();
      while (millis() - Turning_Time < 750)
      {
        if (turn_flag)
        { turn_flag = 0;
          IR.Check();
        }
      }
      while (millis() - Turning_Time < 1000) {
        Balanced.Motion_Control(LEFT);
      }
      turn_flag = 1;
    }

  }
  Obstacle_time = millis();
}

void IR::Check()
{
  int motion = left_is_obstacle + right_is_obstacle;
  switch (motion)
  {
    case Meet_OBSTACLE: Balanced.Motion_Control(LEFT);
      left_is_obstacle = 0; break;

    case FOLLOW_RIGHT:
      Balanced.Motion_Control(RIGHT);
      left_is_obstacle = 0; break;

    case FOLLOW_LEFT:
      Balanced.Motion_Control(LEFT);
      right_is_obstacle = 0; break;

    case FOLLOW_BACK:
      Balanced.Motion_Control(LEFT);
      right_is_obstacle = 0;
      left_is_obstacle = 0; break;
  }

}

void IR::Send()
{
  static unsigned long ir_send_time;

  if (millis() - ir_send_time > 15)
  {
    for (int i = 0; i < 39; i++)
    {
      digitalWrite(IR_SEND_PIN, LOW);
      delayMicroseconds(9);
      digitalWrite(IR_SEND_PIN, HIGH);
      delayMicroseconds(9);
    }
    ir_send_time = millis();
  }
}
