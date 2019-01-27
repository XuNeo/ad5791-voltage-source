#ifndef _SERIAL_FRAME_H_
#define _SERIAL_FRAME_H_
#include "stdint.h"

#define SFRAME_START  0x7d
#define SFRAME_STOP   0x7c  //i don't want to use same mark as start and stop.
#define SFRAME_ESCAPE 0x7e

typedef enum{
  sframe_state_start = 0,
  sframe_state_framelen,
  sframe_state_payload,
  sframe_state_escaping,
  sframe_state_crc,
  sframe_state_end,
}sframe_state_def;

typedef void (*sframe_callback)(uint8_t*, uint32_t);
typedef void (*sframe_outfunc)(uint8_t);

/**
 * @brief this structure is only used for decoder.
 * The encoder will send out data when encoding.
*/
typedef struct _sframe{
  uint8_t *pbuff;             /**< buffer used to store decoded frame. */
  uint32_t buffsz;            /**< buffer size */
  uint32_t frame_len;         /**< decoded frame length. */
  uint32_t windex;            /**< the index to current buff position */
  sframe_state_def state;     /**< current state. */
  sframe_callback callback;   /**< the function that will be called if a frame is successfully decode. */
}sframe_def;

void sframe_init(sframe_def *psframe, uint8_t *pbuff, uint32_t buffsz, sframe_callback callback);
int32_t sframe_decode(sframe_def *psframe, uint8_t *pinput, uint32_t len);
int32_t sframe_encode(sframe_outfunc pfunc, uint8_t *pdata, uint32_t len);

#endif
