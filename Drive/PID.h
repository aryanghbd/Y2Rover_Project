#ifndef PID_H
#define PID_H

#include <Arduino.h>

class PID
{
  private:
    float kp;                      //PID gains
    float ki;
    float kd;
    float u0;                      //Current control output
    float u_min;                   //Minimum and maximum controul outputs
    float u_max;              
    float T;                      //Time change
    float e_integration;           //Previous error and control signals
    float e0;
    float e1;
    float e2;
    float u1;
  
  public:

    PID(float minimum, float maximum, float _kp, float _ki, float _kd);                           //Create a PID object

    float Control(float input, float measout, float dt);                                           //Calculate a control output from an input, the current value of the output
                                                                                                  //and the time since the previous iteration
};

#endif
