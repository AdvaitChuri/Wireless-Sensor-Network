/*---------------FILE: Prog1.c-----------------*/

/*
by:   Advait Churi
      ID: 01738336

PURPOSE
Prog1.c display message according to packet received.
Shows error for unknown message type and
if destination address is not matched shows info message.

*/

/*Include micrium and STM headers*/
#include "includes.h"
/*Include PktParser module header*/
#include "PktParser.h"
/*Include Error module header*/
#include "Error.h"

/*Message type definition*/

#define TempMsg                  0x01
#define PresMsg 		 0x02
#define HumMsg			 0x03
#define WindMsg			 0x04
#define RadMsg	 		 0x05
#define DateTimeMsg		 0x06
#define PrecMsg			 0x07
#define IDMsg                    0x08

#define MaxId                    10     /*ID Array Size*/
#define TwoBytes                 2      /*Speed and Precipitation Array Size*/

/*Shift definitions for swapping bytes of 32 bit number*/
#define ShiftThreeBytes          24
#define ShiftByte                8
#define ShiftNibble              4

/*Nibble Mask*/
#define UpperNibbleMask          0xF0
#define LowerNibbleMask          0x0F

/*Masks and constant definitions for date*/
#define LSBMask32                0x000000FF
#define ThirdMSBMask32           0x0000FF00
#define SecondMSBMask32          0x00FF0000
#define MSBMask32                0xFF000000
#define DayMask                  0x1F
#define HourShift                27
#define MinuteShiftLeft          5 
#define MinuteShiftRight         26
#define YearShiftLeft            11
#define YearShiftRight           20
#define MonthShiftLeft           23
#define MonthShiftRight          28

/*Array bytes*/
#define FirstByte                0
#define SecondByte               1

#define HeaderLength             4      /*Header size*/
#define InitialState             0      
#define dstAdd                   1      /*Adress of Eval-STM32F107 */
#define BaudRate                 9600   /*Baud rate for RS232*/

CPU_INT08U                       c;     /* 8 bit buffer to store each byte*/



/*Define global data types for entire payload*/
/*Payload structure data type*/
#pragma pack(1)
typedef struct
{
  CPU_INT08U payloadLen;                /*Payload length */
  CPU_INT08U dstAddr;                   /*Destination address*/
  CPU_INT08U srcAddr;                   /*Source address*/
  CPU_INT08U msgType;                   /*Message type*/
  union                                 /*One of the 8 possibilities that comes after payload header*/
  {
    CPU_INT08S temp;                    /*Temperature data*/
    CPU_INT16U pres;                    /*Pressure data*/
    struct                              /*Humidity data type*/
      {
      CPU_INT08S dewPt;                 /*Dew point*/
      CPU_INT08U hum;                   /*Humidity*/
      }hum;
    struct                              /*Wind data type*/
      {
      CPU_INT08U speed[TwoBytes];       /*Speed*/
      CPU_INT16U dir;                   /*Direction */
      }wind;
   CPU_INT16U rad;                      /*Solar radiation Intensity*/
   CPU_INT32U dateTime;                 /*Date & Time*/
   CPU_INT08U depth[TwoBytes];          /*Precipitation depth*/
   CPU_INT08U id[MaxId];                /*ID message*/
  }dataPart;
}Payload;

/******************** Function "process()" : Prints each Message ********************/
void process(){
  Payload payloadBfr;                                 /*Payload buffer*/
  for(;;){
      ParsePkt(&payloadBfr);                          /*send address of payloadBfr to ParsePkt()*/
      CPU_INT08U IDByte = InitialState ;
      if(payloadBfr.dstAddr != dstAdd)                /*If destination address is not matched print error*/
        BSP_Ser_Printf(" *** INFO: Not My Address");  
      else{                                           /*Else print the case */
        switch(payloadBfr.msgType){
          case TempMsg:                               /*Display temperature message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: TEMPERATURE MESSAGE\n   Temperature = %d\n", payloadBfr.srcAddr, payloadBfr.dataPart.temp);
            break;
          case PresMsg:                               /*Display pressure message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: BAROMETRIC PRESSURE MESSAGE\n   Pressure = %d\n", payloadBfr.srcAddr,
                          (((payloadBfr.dataPart.pres << ShiftByte) & ThirdMSBMask32) | (payloadBfr.dataPart.pres >> ShiftByte)));
            break;
          case HumMsg:                                /*Display Humidity message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: HUMIDITY MESSAGE\n   Dew Point = %d Humidity = %d\n",payloadBfr.srcAddr,
                           payloadBfr.dataPart.hum.dewPt,payloadBfr.dataPart.hum.hum);
            break;
          case WindMsg:                               /*Display wind message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: WIND MESSAGE \n   Speed = %d%d%d.%d Wind Direction = %d\n",payloadBfr.srcAddr,
                          (payloadBfr.dataPart.wind.speed[FirstByte] & UpperNibbleMask) >> ShiftNibble,  
                          (payloadBfr.dataPart.wind.speed[FirstByte] & LowerNibbleMask),
                          (payloadBfr.dataPart.wind.speed[SecondByte] & UpperNibbleMask) >> ShiftNibble, 
                          (payloadBfr.dataPart.wind.speed[SecondByte] & LowerNibbleMask),
                           payloadBfr.dataPart.wind.dir); 
            break;      
          case RadMsg:                                /*Display radiation message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: SOLAR RADIATION MESSAGE\n   Solar Radiation Intensity = %d\n",payloadBfr.srcAddr,
                          ((payloadBfr.dataPart.rad << ShiftByte) & ThirdMSBMask32) | (payloadBfr.dataPart.rad >> ShiftByte));
            break;          
          case DateTimeMsg:                           /*Display date-time message*/
            payloadBfr.dataPart.dateTime =((payloadBfr.dataPart.dateTime & LSBMask32) << ShiftThreeBytes |     /*Rearrange date-time bytes*/
                                           (payloadBfr.dataPart.dateTime & ThirdMSBMask32) << ShiftByte | 
                                           (payloadBfr.dataPart.dateTime & SecondMSBMask32) >> ShiftByte | 
                                           (payloadBfr.dataPart.dateTime & MSBMask32) >> ShiftThreeBytes) ;           
            BSP_Ser_Printf("\n SOURCE NODE %d: DATE/TIME STAMP MESSAGE\n   Time Stamp = %d/%d/%d %d:%d\n",payloadBfr.srcAddr,
                          (payloadBfr.dataPart.dateTime << MonthShiftLeft) >> MonthShiftRight, 
                          (payloadBfr.dataPart.dateTime & DayMask),
                          (payloadBfr.dataPart.dateTime << YearShiftLeft) >> YearShiftRight,  
                          (payloadBfr.dataPart.dateTime >> HourShift),
                          (payloadBfr.dataPart.dateTime << MinuteShiftLeft) >>  MinuteShiftRight );
            break;
          case PrecMsg:                               /*Display prcipitation message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: PRECIPITATION MESSAGE\n   Precipitation Depth = %d%d.%d%d\n",payloadBfr.srcAddr,
                          (payloadBfr.dataPart.depth[FirstByte] & UpperNibbleMask) >> ShiftNibble,  
                          (payloadBfr.dataPart.depth[FirstByte] & LowerNibbleMask),
                          (payloadBfr.dataPart.depth[SecondByte] & UpperNibbleMask) >> ShiftNibble,  
                          (payloadBfr.dataPart.depth[SecondByte] & LowerNibbleMask));
            break;
          case IDMsg:                                 /*Display ID message*/
            BSP_Ser_Printf("\n SOURCE NODE %d: SENSOR ID MESSAGE\n   Node ID = ",payloadBfr.srcAddr);
              for( IDByte = InitialState ; IDByte < payloadBfr.payloadLen - HeaderLength ; IDByte++ )
              BSP_Ser_Printf("%c",payloadBfr.dataPart.id[IDByte]);
              BSP_Ser_Printf("\n");
            break;
          default:
            Err("Unknown Message Type");
        }}}}
/************************** Function "AppMain()" : main application function**********************/

void AppMain()
{
process(); 
}

/************************** Function "main()" : main function **********************/
CPU_INT32S main()
{
  /*Initialize the STM32F107 eval board*/
  BSP_IntDisAll();              /*Disable all interrupts*/
  
  BSP_Init();                   /*Initialize BSP*/
  
  BSP_Ser_Init(BaudRate);       /*Initialize RS232 interface*/
  
/* Run Application */
  AppMain();                    
  return 0;
}
