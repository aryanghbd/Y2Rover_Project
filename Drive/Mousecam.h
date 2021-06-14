#ifndef MOUSECAM_H
#define MOUSECAM_H

#include <Arduino.h>
#include <SPI.h>
class Mousecam
{
  private:                                                                //Define mouscam parameters
    int _PIN_SS;        
    int _PIN_MISO;      
    int _PIN_MOSI;      
    int _PIN_SCK;       
    int _PIN_MOUSECAM_RESET;     
    int _PIN_MOUSECAM_CS;        
    int _ADNS3080_PIXELS_X;                 
    int _ADNS3080_PIXELS_Y;                 
    int _ADNS3080_PRODUCT_ID;            
    int _ADNS3080_REVISION_ID;           
    int _ADNS3080_MOTION;                
    int _ADNS3080_DELTA_X;               
    int _ADNS3080_DELTA_Y;              
    int _ADNS3080_SQUAL;                 
    int _ADNS3080_PIXEL_SUM;             
    int _ADNS3080_MAXIMUM_PIXEL;         
    int _ADNS3080_CONFIGURATION_BITS;    
    int _ADNS3080_EXTENDED_CONFIG;       
    int _ADNS3080_DATA_OUT_LOWER;        
    int _ADNS3080_DATA_OUT_UPPER;        
    int _ADNS3080_SHUTTER_LOWER;         
    int _ADNS3080_SHUTTER_UPPER;         
    int _ADNS3080_FRAME_PERIOD_LOWER;    
    int _ADNS3080_FRAME_PERIOD_UPPER;    
    int _ADNS3080_MOTION_CLEAR;          
    int _ADNS3080_FRAME_CAPTURE;         
    int _ADNS3080_SROM_ENABLE;           
    int _ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER;      
    int _ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER;      
    int _ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER;      
    int _ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER;      
    int _ADNS3080_SHUTTER_MAX_BOUND_LOWER;           
    int _ADNS3080_SHUTTER_MAX_BOUND_UPPER;           
    int _ADNS3080_SROM_ID;               
    int _ADNS3080_OBSERVATION;           
    int _ADNS3080_INVERSE_PRODUCT_ID;    
    int _ADNS3080_PIXEL_BURST;           
    int _ADNS3080_MOTION_BURST;          
    int _ADNS3080_SROM_LOAD;             
    
    int _ADNS3080_PRODUCT_ID_VAL;

  
  public:
  Mousecam(int IPIN_SS, int IPIN_MISO, int IPIN_MOSI, int IPIN_SCK, int IPIN_MOUSECAM_RESET, int IPIN_MOUSECAM_CS, int IADNS3080_PIXELS_X, int IADNS3080_PIXELS_Y, int IADNS3080_PRODUCT_ID, 
           int IADNS3080_REVISION_ID, int IADNS3080_MOTION, int IADNS3080_DELTA_X, int IADNS3080_DELTA_Y, int IADNS3080_SQUAL, int IADNS3080_PIXEL_SUM, int IADNS3080_MAXIMUM_PIXEL, 
           int IADNS3080_CONFIGURATION_BITS, int IADNS3080_EXTENDED_CONFIG, int IADNS3080_DATA_OUT_LOWER, int IADNS3080_DATA_OUT_UPPER, int IADNS3080_SHUTTER_LOWER, int IADNS3080_SHUTTER_UPPER,
           int IADNS3080_FRAME_PERIOD_LOWER, int IADNS3080_FRAME_PERIOD_UPPER, int IADNS3080_MOTION_CLEAR, int IADNS3080_FRAME_CAPTURE, int IADNS3080_SROM_ENABLE, int IADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER,
           int IADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER, int IADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER, int IADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER, int IADNS3080_SHUTTER_MAX_BOUND_LOWER, int IADNS3080_SHUTTER_MAX_BOUND_UPPER,
           int IADNS3080_SROM_ID, int IADNS3080_OBSERVATION, int IADNS3080_INVERSE_PRODUCT_ID, int IADNS3080_PIXEL_BURST, int IADNS3080_MOTION_BURST, int IADNS3080_SROM_LOAD, int IADNS3080_PRODUCT_ID_VAL);  
  
  void mousecam_reset();                                                                                          //Reset mouscam

  int mousecam_init();                                                                                            //Will initialise mouscam and return 1 if it fails

  void mousecam_write_reg(int reg, int val);                                                                      //Write to mouscam

  int mousecam_read_reg(int reg);                                                                                 //Read data from mousecam

  void mousecam_read_motion(byte *motion, char *dx, char *dy, byte *squal, word* shutter, byte *max_pix);         //Read motion data from mousecam
  
  int mousecam_frame_capture(byte *pdata);                                                                        //Captures a fram from the mousecam
};

#endif
