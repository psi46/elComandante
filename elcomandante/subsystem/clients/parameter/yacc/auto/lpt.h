/**
 * \file lpt.h
 * \brief control parport datalines
 *
 * \author Dennis Terhorst
 * \date 06/2006
 */
#ifndef LPT_H
#define LPT_H

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief initialize parport device
 * \param dev /dev/parport0 or sth. the like...
 * \return parfd or -1 fail (see errno)
 */ 
int lpt_open(const char* dev);	// init

/**
 * \brief Write a bit pattern to the data lines
 * 
 * \return 0 on success
 */
int lpt_setdata(int parfd, char bitpat);

/**
 * \brief close parport device and zero all lines
 * \return 0, -1 fail
 */
int lpt_close();//int parfd);	// close

#ifdef __cplusplus
};
#endif

#endif //ndef LPT_H
