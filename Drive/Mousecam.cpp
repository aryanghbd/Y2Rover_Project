#include "Mousecam.h"
#include <SPI.h>

Mousecam::Mousecam(int IPIN_SS, int IPIN_MISO, int IPIN_MOSI, int IPIN_SCK, int IPIN_MOUSECAM_RESET, int IPIN_MOUSECAM_CS, int IADNS3080_PIXELS_X, int IADNS3080_PIXELS_Y, int IADNS3080_PRODUCT_ID, 
           int IADNS3080_REVISION_ID, int IADNS3080_MOTION, int IADNS3080_DELTA_X, int IADNS3080_DELTA_Y, int IADNS3080_SQUAL, int IADNS3080_PIXEL_SUM, int IADNS3080_MAXIMUM_PIXEL, 
           int IADNS3080_CONFIGURATION_BITS, int IADNS3080_EXTENDED_CONFIG, int IADNS3080_DATA_OUT_LOWER, int IADNS3080_DATA_OUT_UPPER, int IADNS3080_SHUTTER_LOWER, int IADNS3080_SHUTTER_UPPER,
           int IADNS3080_FRAME_PERIOD_LOWER, int IADNS3080_FRAME_PERIOD_UPPER, int IADNS3080_MOTION_CLEAR, int IADNS3080_FRAME_CAPTURE, int IADNS3080_SROM_ENABLE, int IADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER,
           int IADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER, int IADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER, int IADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER, int IADNS3080_SHUTTER_MAX_BOUND_LOWER, int IADNS3080_SHUTTER_MAX_BOUND_UPPER,
           int IADNS3080_SROM_ID, int IADNS3080_OBSERVATION, int IADNS3080_INVERSE_PRODUCT_ID, int IADNS3080_PIXEL_BURST, int IADNS3080_MOTION_BURST, int IADNS3080_SROM_LOAD, int IADNS3080_PRODUCT_ID_VAL){
           
           _PIN_SS = IPIN_SS;        
           _PIN_MISO = IPIN_MISO;      
           _PIN_MOSI = IPIN_MOSI;      
           _PIN_SCK = IPIN_SCK;       
           _PIN_MOUSECAM_RESET = IPIN_MOUSECAM_RESET;     
           _PIN_MOUSECAM_CS = IPIN_MOUSECAM_CS;        
           _ADNS3080_PIXELS_X = IADNS3080_PIXELS_X;                 
           _ADNS3080_PIXELS_Y = IADNS3080_PIXELS_Y;                 
           _ADNS3080_PRODUCT_ID = IADNS3080_PRODUCT_ID;            
           _ADNS3080_REVISION_ID = IADNS3080_REVISION_ID;           
           _ADNS3080_MOTION = IADNS3080_MOTION;                
           _ADNS3080_DELTA_X = IADNS3080_DELTA_X;               
           _ADNS3080_DELTA_Y = IADNS3080_DELTA_Y;              
           _ADNS3080_SQUAL = IADNS3080_SQUAL;                 
           _ADNS3080_PIXEL_SUM = IADNS3080_PIXEL_SUM;             
           _ADNS3080_MAXIMUM_PIXEL = IADNS3080_MAXIMUM_PIXEL;         
           _ADNS3080_CONFIGURATION_BITS = IADNS3080_CONFIGURATION_BITS;   
           _ADNS3080_EXTENDED_CONFIG = IADNS3080_EXTENDED_CONFIG;       
           _ADNS3080_DATA_OUT_LOWER = IADNS3080_DATA_OUT_LOWER;        
           _ADNS3080_DATA_OUT_UPPER = IADNS3080_DATA_OUT_UPPER;        
           _ADNS3080_SHUTTER_LOWER = IADNS3080_SHUTTER_LOWER;         
           _ADNS3080_SHUTTER_UPPER = IADNS3080_SHUTTER_UPPER;         
           _ADNS3080_FRAME_PERIOD_LOWER = IADNS3080_FRAME_PERIOD_LOWER;    
           _ADNS3080_FRAME_PERIOD_UPPER = IADNS3080_FRAME_PERIOD_UPPER;    
           _ADNS3080_MOTION_CLEAR = IADNS3080_MOTION_CLEAR;          
           _ADNS3080_FRAME_CAPTURE = IADNS3080_FRAME_CAPTURE ;         
           _ADNS3080_SROM_ENABLE = IADNS3080_SROM_ENABLE;           
           _ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER = IADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER;      
           _ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER = IADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER;      
           _ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER = IADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER;      
           _ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER = IADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER;      
           _ADNS3080_SHUTTER_MAX_BOUND_LOWER = IADNS3080_SHUTTER_MAX_BOUND_LOWER;           
           _ADNS3080_SHUTTER_MAX_BOUND_UPPER = IADNS3080_SHUTTER_MAX_BOUND_UPPER;           
           _ADNS3080_SROM_ID = IADNS3080_SROM_ID;               
           _ADNS3080_OBSERVATION = IADNS3080_OBSERVATION;           
           _ADNS3080_INVERSE_PRODUCT_ID = IADNS3080_INVERSE_PRODUCT_ID;    
           _ADNS3080_PIXEL_BURST = IADNS3080_PIXEL_BURST;           
           _ADNS3080_MOTION_BURST = IADNS3080_MOTION_BURST;          
           _ADNS3080_SROM_LOAD = IADNS3080_SROM_LOAD;
           _ADNS3080_PRODUCT_ID_VAL = IADNS3080_PRODUCT_ID_VAL;
           
           }

void Mousecam::mousecam_reset()
{
  digitalWrite(_PIN_MOUSECAM_RESET,HIGH);
  delay(1); // reset pulse >10us
  digitalWrite(_PIN_MOUSECAM_RESET,LOW);
  delay(35); // 35ms from reset to functional
}

int Mousecam::mousecam_init()
{
  pinMode(_PIN_MOUSECAM_RESET,OUTPUT);
  pinMode(_PIN_MOUSECAM_CS,OUTPUT);
  
  digitalWrite(_PIN_MOUSECAM_CS,HIGH);
  
  mousecam_reset();
  
  int pid = mousecam_read_reg(_ADNS3080_PRODUCT_ID);
  if(pid != _ADNS3080_PRODUCT_ID_VAL)
    return -1;

  // turn on sensitive mode
  mousecam_write_reg(_ADNS3080_CONFIGURATION_BITS, 0x19);

  return 0;
}

void Mousecam::mousecam_write_reg(int reg, int val)
{
  digitalWrite(_PIN_MOUSECAM_CS, LOW);
  SPI.transfer(reg | 0x80);
  SPI.transfer(val);
  digitalWrite(_PIN_MOUSECAM_CS,HIGH);
  delayMicroseconds(50);
}

int Mousecam::mousecam_read_reg(int reg)
{
  digitalWrite(_PIN_MOUSECAM_CS, LOW);
  SPI.transfer(reg);
  delayMicroseconds(75);
  int ret = SPI.transfer(0xff);
  digitalWrite(_PIN_MOUSECAM_CS,HIGH); 
  delayMicroseconds(1);
  return ret;
}

void Mousecam::mousecam_read_motion(byte *motion, char *dx, char *dy, byte *squal, word* shutter, byte *max_pix)
{
  digitalWrite(_PIN_MOUSECAM_CS, LOW);
  SPI.transfer(_ADNS3080_MOTION_BURST);
  delayMicroseconds(75);
  *motion =  SPI.transfer(0xff);
  *dx =  SPI.transfer(0xff);
  *dy =  SPI.transfer(0xff);
  *squal =  SPI.transfer(0xff);
  *shutter =  SPI.transfer(0xff)<<8;
  *shutter |=  SPI.transfer(0xff);
  *max_pix =  SPI.transfer(0xff);
  digitalWrite(_PIN_MOUSECAM_CS,HIGH); 
  delayMicroseconds(5);
}

int Mousecam::mousecam_frame_capture(byte *pdata)
{
  mousecam_write_reg(_ADNS3080_FRAME_CAPTURE,0x83);
  
  digitalWrite(_PIN_MOUSECAM_CS, LOW);
  
  SPI.transfer(_ADNS3080_PIXEL_BURST);
  delayMicroseconds(50);
  
  int pix;
  byte started = 0;
  int count;
  int timeout = 0;
  int ret = 0;
  for(count = 0; count < _ADNS3080_PIXELS_X * _ADNS3080_PIXELS_Y; )
  {
    pix = SPI.transfer(0xff);
    delayMicroseconds(10);
    if(started==0)
    {
      if(pix&0x40)
        started = 1;
      else
      {
        timeout++;
        if(timeout==100)
        {
          ret = -1;
          break;
        }
      }
    }
    if(started==1)
    {
      pdata[count++] = (pix & 0x3f)<<2; // scale to normal grayscale byte range
    }
  }
  digitalWrite(_PIN_MOUSECAM_CS,HIGH); 
  delayMicroseconds(14);
  
  return ret;
}
