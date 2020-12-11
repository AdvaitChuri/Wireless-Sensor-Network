/*---------------FILE: PktParser.c-----------------*/

/*
by:   Advait Churi
      ID: 01738336

PURPOSE
PktParser.c checks three preamble bytes and if they are correct
stores upcoming bytes or display respective error.
At the end check for packet error by comparing checksum.
 
*/

/*Include micrium and STM headers*/
#include "includes.h"
/*Include PktParser module header*/
#include "PktParser.h"
/*Include Error module header*/
#include "Error.h"


#define HeaderLength    4        /*Header size*/

#define P1Char          0x03     /*Preamble byte 1*/
#define P2Char          0xEF     /*Preamble byte 1*/
#define P3Char          0xAF     /*Preamble byte 1*/

#define MinLength       8        /*Minimum packet length*/
#define MaxLength       14       /*Maximum payload length*/
#define InitialState    0

/*Structure to save payloadlength and data bytes*/
typedef struct
{
  CPU_INT08U payloadLen;
  CPU_INT08U data[1];
}PktBfr;


typedef enum{ P1, P2, P3, K, D, CS, ER} ParserState;

void ParsePkt(void *payloadBfr){
  ParserState parseState = P1;                          /*Initial state*/
  CPU_INT08U               c;                           /*variable to store byte*/
  CPU_INT08U               checksumVal = InitialState ; /*Initial checksum value*/
  CPU_INT08U               i           = InitialState ; /*counter to represent databyte */
  PktBfr *pktBfr = payloadBfr;                          /*payloadBfr look like pktBfr*/
  for(;;){
      c = GetByte();                                    /*Get next byte of the packet*/         
      //BSP_Ser_Printf("Byte is %d \n",c); 
      if((parseState != CS))                           /*If state is CS do not calculate checksum*/
          checksumVal = checksumVal ^ c;
      switch(parseState){
        case P1:                                        /*Check preamble 1*/
          if(c==P1Char)
              parseState=P2;
          else{
              Err("Bad Preamble Byte 1");
              parseState=ER;}
          break;
        case P2:                                        /*Check preamble 2*/
          if(c==P2Char){parseState=P3;}
          else{
              Err("Bad Preamble Byte 2");
              parseState=ER;}
          break;
        case P3:                                        /*Check preamble 3*/            
          if(c==P3Char)
              parseState=K;
          else{
              Err("Bad Preamble Byte 3");
              parseState=ER;}
          break;          
        case K:                
          pktBfr->payloadLen = c-HeaderLength;          /*save payloadlength*/
          if((pktBfr->payloadLen < (MinLength-HeaderLength)) | (pktBfr->payloadLen > (MaxLength-HeaderLength))){
              Err("Bad Packet Size");
              parseState = ER;}
          else{parseState=D;}
          break;            
        case D:  
          pktBfr->data[i++] = c;                        /*save data bytes except payloadLength*/
          if(i>=(pktBfr->payloadLen - 1))
              parseState=CS;
          break;                      
        case CS:
          if(c == checksumVal){                         /*If checksum is correct set state to P1 and return*/
              parseState = P1;
              return;  }
          else{
              Err("Checksum error"); /*If checksum error occurs set state to error and reset checksum and counter*/
              parseState  = ER;
              checksumVal = InitialState ;
              i           = InitialState ; 
          break;}           
        case ER:                                        /*Stay in the loop until you received correct preamble bytes*/
          if (c == P1Char){                             /*If byte is equal to P1char check if next byte is P2char*/
            checksumVal = P1Char;  
            c = GetByte();  
            if(c == P2Char){
            checksumVal = checksumVal  ^ c;
            parseState = P3;}}        
          break;}}}