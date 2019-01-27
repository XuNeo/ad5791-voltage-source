/**
 * Serial frame uses the similar method in HDLC protocol.
 * high level data link control protocol
 * 0x7d as frame start and 0x7c as end marker(not same as HDLC)
 * 0x7e is the escape character, both data 0x7d and 0x7e should be
 * escaped with 0x7e + data^0x20.
 * So, data 0x7e will be encoded to 0x7e+0x5e; data 0x7d will be 
 * encoded to 0x7e+0x5d
 * */
#include "serial_frame.h"

/**
 * @brief init sframe with given buffer(and size) and function pointer.
 * @param pbuff: the buffer used to store frame received.
 * @param buffsz: buffer size.
 * @param callback: the function pointer that will be called when valid frame is received.
 * @return none.
*/
void sframe_init(sframe_def *psframe, uint8_t *pbuff, uint32_t buffsz, sframe_callback callback){
  if(psframe == 0) return;
  psframe->pbuff = pbuff;
  psframe->buffsz = buffsz;
  psframe->state = sframe_state_start;
  psframe->frame_len = 0;
  psframe->windex = 0;
  psframe->callback = callback;
}

/**
 * @brief decode the input data in buffer.
 * @return return 0.
*/
int32_t sframe_decode(sframe_def *psframe, uint8_t *pinput, uint32_t len){
  while(len--){
    if(psframe->state != sframe_state_end)
      if(*pinput == SFRAME_STOP)  //if we received unexpected STOP flag, restart the parser.
        psframe->state = sframe_state_start;
    switch(psframe->state){
      case sframe_state_start:  //waiting for start flag.
        if(*pinput == SFRAME_START)
          psframe->state = sframe_state_framelen; //next byte is frame length
        break;
      case sframe_state_framelen:  //frame length
        psframe->frame_len = *pinput;
        psframe->state = sframe_state_payload;
        break;
      case sframe_state_payload:
        if(*pinput == SFRAME_ESCAPE)
          psframe->state = sframe_state_escaping; //this is not a data.
        else
          psframe->pbuff[psframe->windex++] = *pinput;
        if(psframe->windex == psframe->frame_len) 
          //we got enough data, check the end flag.
          psframe->state = sframe_state_end;
        break;
      case sframe_state_escaping: //this is an escaping data.
        psframe->pbuff[psframe->windex++] = *pinput ^ 0x20;
        psframe->state = sframe_state_payload;
        if(psframe->windex == psframe->frame_len) 
          //we got enough data, check the end flag.
          psframe->state = sframe_state_end;
        break;
      case sframe_state_end:
        if(*pinput == SFRAME_STOP){
          if(psframe->callback)
            psframe->callback(psframe->pbuff, psframe->frame_len);
          psframe->state = sframe_state_start;
          psframe->windex = 0;
        }
        break;
    }
    pinput ++;
  }
  return 0;
}

/**
 * @brief encode the data and output it through function pfunc.
 * @param pfunc: the function used to output one character.
 * @param pdata: pointer to the data.
 * @param len: data length.
 * @return none.
*/
int32_t sframe_encode(sframe_outfunc pfunc, uint8_t *pdata, uint32_t len){
  if(pdata == 0) return 0;
  if(pfunc == 0) return 0;
  if(len == 0) return 0;
  /**
   * send out frame start and frame length.
  */
  pfunc((uint8_t)SFRAME_START);
  pfunc((uint8_t)len);
  //sending out data
  while(len){
    switch (*pdata){
      case SFRAME_START:
      case SFRAME_STOP:
      case SFRAME_ESCAPE:
      pfunc((uint8_t)SFRAME_ESCAPE);
      pfunc(*pdata++ ^ 0x20);
      len --;
      continue;
    }
    pfunc(*pdata++);
    len--;
  }
  pfunc((uint8_t)SFRAME_STOP);
  pfunc((uint8_t)SFRAME_STOP);
  return 0;
}

