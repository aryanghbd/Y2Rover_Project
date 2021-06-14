#include "PID.h"

PID::PID(float minimum, float maximum, float _kp, float _ki, float _kd){      //Creates PID object with minimum and maximum control output and PID gains.
    kp = _kp;
    ki = _ki;
    kd = _kd;
    u_min = minimum;
    u_max = maximum;
    };

float PID::Control(float input, float measout, float dt){
      e0 = input - measout;                                                   //Error signal calculation
      e_integration = e0;                                                    
      T = dt;                                                                 //Change in time
     
      //Anti-windup, if last-time pid output reaches the limitation, this time there won't be any integrations.
      if(u1 >= u_max) {
        e_integration = 0;
      } else if (u1 <= u_min) {
        e_integration = 0;
      }
      
      float delta_u = kp*(e0-e1) + ki*T*e_integration + kd/T*(e0-2*e1+e2);  //Incremental PID programming. The change to the required output is calcualted and integration occurs by summing changes.
      u0 = u1 + delta_u;                                                      //This time's control output
    
      //Output limitation
      if (u0 > u_max){
        u0 = u_max;}
      if (u0 < u_min ){ 
        u0 = u_min;}
      
      u1 = u0; //Update last time's control output
      e2 = e1; //Update last last time's error
      e1 = e0; //Update last time's error
      return u0;
    };
