#include "fifo.h"
#include "string.h"

/**
 * @brief initialize an empty fifo.
 * @param pbuff: the buffer used to store data.
 * @param len: the buffer length
 * @return none.
*/
void fifo_init(fifo_def *pfifo, uint8_t *pbuff, uint32_t len)
{
  pfifo->pbuff = pbuff;
  pfifo->buff_size = len;
  pfifo->write_index = 0;
  pfifo->read_index = 0;
  pfifo->data_count = 0;
}

/**
 * @brief get current fifo status(data count in fifo)
 * @return count of data in fifo.
*/
uint32_t fifo_status(fifo_def *pfifo)
{
  return pfifo->data_count;
}

/**
 * @brief write one byte to fifo.
 * @param ch: the character to write.
 * @return fifo_err_ok if succeeded.
*/
fifo_err_def fifo_write1B(fifo_def *pfifo, uint8_t ch){
  if(pfifo->data_count >= pfifo->buff_size)
    return fifo_err_full; //fifo is already full
  //write one data to fifo and modify windex and data_count
  pfifo->pbuff[pfifo->write_index] = ch;
  //enter critical area
  FIFO_DIS_INT();
  pfifo->write_index ++;
  if(pfifo->write_index == pfifo->buff_size)
    pfifo->write_index = 0;
  pfifo->data_count ++;
  FIFO_EN_INT();
  //exit critical area
  return fifo_err_ok;
}

/**
 * @brief read one byte from fifo.
 * @param ch: the pointer to save data read.
 * @return return fifo_err_ok if succeeded
*/
fifo_err_def fifo_read1B(fifo_def *pfifo, uint8_t *ch){
  if(ch == 0) return fifo_err_nullp;
  if(pfifo->data_count){ //we have data to read.
    *ch = pfifo->pbuff[pfifo->read_index];
    //enter critical area.
    FIFO_DIS_INT();
    pfifo->read_index ++;
    if(pfifo->read_index == pfifo->buff_size)
      pfifo->read_index = 0;  //rewind to zero.
    pfifo->data_count --;
    FIFO_EN_INT();
    //exit critical area.
    return fifo_err_ok;
  }
  return fifo_err_empty;
}

/**
 * @brief write a buffer to fifo.
 * @param pbuff: the pointer to data to be stored to fifo.
 * @param plen: the pointer to store how many data is in buffer,
 *        and used to store the actual stored data count.
 * @return fifo_err_ok if succeeded. If not all data are written, also return fifo_err_ok,
 *        read plen to determine the actual written data count.
*/
fifo_err_def fifo_write(fifo_def *pfifo, uint8_t *pbuff, uint32_t *plen){
  uint32_t write_len = *plen;
  uint32_t temp;
  if(pfifo->data_count >= pfifo->buff_size)
    return fifo_err_full;
  /* step1, how many data we can write? */
  write_len = write_len > pfifo->buff_size-pfifo->data_count?\
              pfifo->buff_size-pfifo->data_count : write_len;
  /* step2, how many data can be written directly to the buffer. */
  temp = pfifo->buff_size - pfifo->write_index; //we write from 'write_index' to end of buffer.
  if(write_len < temp){ //good, we can write all data at once.
    memcpy(pfifo->pbuff+pfifo->write_index, pbuff, write_len);
    /* !modify critical pointers. */
    FIFO_DIS_INT();
    pfifo->write_index += write_len;
    pfifo->data_count += write_len;
    FIFO_EN_INT();
    /* exit critical area. */
  }
  else{ //we need two steps to write.
    //firstly, write data until to end of buffer.
    memcpy(pfifo->pbuff+pfifo->write_index, pbuff, temp);
    //then write from start of buffer.
    if(write_len != temp) //we do need the second write.
      memcpy(pfifo->pbuff, pbuff+temp, write_len-temp);
    /* !modify critical pointers. */
    FIFO_DIS_INT();
    pfifo->write_index = write_len-temp;
    pfifo->data_count += write_len;
    FIFO_EN_INT();
    /* exit critical area. */
  }
  *plen = write_len;
  return fifo_err_ok;
}

/**
 * @brief read from fifo to a buffer.
 * @param pbuff: the buffer used to store data read.
 * @param plen: the pointer to say how many data expected to read. It also stores the
 *        actual data count read to buffer.
 * @return fifo_err_ok if succeeded.
*/
fifo_err_def fifo_read(fifo_def *pfifo, uint8_t *pbuff, uint32_t *plen)
{
  uint32_t read_len, temp;
  if(pfifo->data_count == 0)
    return fifo_err_empty;
  read_len = pfifo->data_count > *plen?\
              *plen: pfifo->data_count;
  /* How many data we can read directly from buffer? */
  temp = pfifo->buff_size - pfifo->read_index;
  if(read_len < temp){  //good, we can read directly.
    memcpy(pbuff, pfifo->pbuff+pfifo->read_index, read_len);
    //enter critical area.
    FIFO_DIS_INT();
    pfifo->read_index += read_len;
    pfifo->data_count -= read_len;
    FIFO_EN_INT();
    //exit critical area.
  }
  else{//we need two steps to read all data.
    memcpy(pbuff, pfifo->pbuff + pfifo->read_index, temp);
    //then read from buffer start.
    if(read_len != temp)
      memcpy(pbuff+temp, pfifo->pbuff, read_len-temp);
    /* enter critical area */
    FIFO_DIS_INT();
    pfifo->read_index = read_len - temp;
    pfifo->data_count -= read_len;
    FIFO_EN_INT();
    //exit critical area.
  }
  *plen = read_len;
  return fifo_err_ok;
}
