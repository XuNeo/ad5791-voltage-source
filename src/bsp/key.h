#ifndef _KEY_H_
#define _KEY_H_

#define KEY_PRESS_L 0x80  //the key is pressed for a long time.
#define KEY_OK      0x02

void key_init(void);
uint8_t get_encoder(void);
uint8_t get_key(void);

#endif
