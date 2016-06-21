/*
 * mygpio.h for Linux .
 *
 * History:
 *      10-April-2006 create this file
 *
 * Base register description
 */

#ifndef _MY_GPIO_H__
#define _MY_GPIO_H__

#define DEBUG_GPIO

#define SCTL_PHY_BASE		0x101E0000
#define SCTL_MAP_SIZE    	0x1000

#define GPIO_PHY_BASE		0x101E4000
#define GPIO_MAP_SIZE    	0x4000

#define GPIO_PHY_BASE1		0x101F7000


#define GPIO_SPACE_SIZE    	0x1000
#define GPIO_0             	0
#define GPIO_0_BASE        	GPIO_BASE

#define GPIO_1             	1
#define GPIO_1_BASE        	(GPIO_BASE + GPIO_1 * GPIO_SPACE_SIZE)

#define GPIO_2             	2
#define GPIO_2_BASE        	(GPIO_BASE + GPIO_2 * GPIO_SPACE_SIZE)

#define GPIO_3             	3
#define GPIO_3_BASE        	(GPIO_BASE + GPIO_3 * GPIO_SPACE_SIZE)

#define GPIO_4			4
#define GPIO_4_BASE        	(GPIO_BASE1 + 0 * GPIO_SPACE_SIZE)

#define GPIO_5			5
#define GPIO_5_BASE        	(GPIO_BASE1 + 1 * GPIO_SPACE_SIZE)

#define GPIO_6			6
#define GPIO_6_BASE        	(GPIO_BASE1 + 2 * GPIO_SPACE_SIZE)

#define GPIO_7			7
#define GPIO_7_BASE        	(GPIO_BASE1 + 3 * GPIO_SPACE_SIZE)

#define GPIO_GROUP_MID_NUMBER	4
#define GPIO_GROUP_NUMBER  	8

#define GPIO_DATA_OFFSET   	0          /* gpio data register offset */

#define GPIO_DIR           	0x400      /* gpio direction register */

#define DIRECTION_OUTPUT   	1
#define DIRECTION_INPUT    	0

#define GPIO_IS            	0x404      /* gpio interrupt sense register */
#define SENSE_EDGE         	0
#define SENSE_LEVEL        	1

#define GPIO_IBE           	0x408      /* gpio interrupt both edge register */
#define SENSE_SINGLE       	0
#define SENSE_BOTH         	1

#define GPIO_IEV           	0x40C      /* gpio interrupt event register  */
#define EVENT_RISING_EDGE  	1
#define EVENT_FALLING_EDGE 	0

#define EVENT_HIGH_LEVEL   	1
#define EVENT_LOW_LEVEL    	0

#define GPIO_IE            	0x410      /* gpio interrupt enable register  */
#define GPIO_RIS           	0x414      /* gpio raw interrupt status register */
#define GPIO_MIS           	0x418      /* gpio masked interrupt status register */
#define GPIO_IC            	0x41C      /* gpio interrupt clear register */
#define GPIO_AFSEL         	0x420      /* gpio mode control select register */
/* 1: enable hardware control; */
/* 0: enable software control. */


typedef struct _stMyGPIO_bit_info
{
    unsigned char	byPathNum;
    unsigned char	byBitNum;
    unsigned char	byBitValue;
    unsigned char	byReserved;

} MYGPIO_BIT_INFO, *PMYGPIO_BIT_INFO;

typedef struct _stMyGPIO_byte_info
{
    unsigned char	byPathNum;
    unsigned char	byByteValue;
    unsigned short	nReserved;

} MYGPIO_BYTE_INFO, *PMYGPIO_BYTE_INFO;

#define MYGPIO_DIR_GET_BIT		0x0
#define MYGPIO_DIR_SET_BIT 		0x1

#define MYGPIO_DIR_GET_BYTE		0x2
#define MYGPIO_DIR_SET_BYTE 		0x3

#define MYGPIO_DIR_GET_BIT_VALUE	0x4
#define MYGPIO_DIR_SET_BIT_VALUE 	0x5

#define MYGPIO_DIR_GET_BYTE_VALUE	0x6
#define MYGPIO_DIR_SET_BYTE_VALUE 	0x7


/*Free gpio virtual address*/
extern int gpio_unmap( void );

/*Get  virtual address for gpio*/
extern int gpio_remap( void );

extern int gpio_modeset( unsigned int gpio_pathnum, unsigned int mode );

/*Get the mode of the gpio path pins*/
extern int gpio_modeget( unsigned int gpio_pathnum, unsigned int* mode );

/* Set the direction of the single bit for the specific path */
/* dirbit=1 means output; */
/* dirbit=0 means input.  */
extern int gpio_dirsetbit( unsigned int gpio_pathnum, unsigned int bitx, unsigned int dirbit );

/* Set the input or output direction for the total path */
int gpio_dirsetbyte( unsigned int gpio_pathnum, unsigned int dirbyte );

/* Get the input or output direction for a specific pin of a port */
extern int gpio_dirgetbit( unsigned int gpio_pathnum, unsigned int bitx, unsigned int* dirbyte );

/* Get the input or output direction for the total path */
extern int gpio_dirgetbyte( unsigned int gpio_pathnum, unsigned int* dirbyte );


/*Bit write the value from a specific bit of a gpio path*/
extern int gpio_writebit( unsigned int gpio_pathnum, unsigned int bitx, unsigned int bitvalue );

/*byte write the value of a specific gpio path*/
extern int gpio_writebyte( unsigned int gpio_pathnum, unsigned int bytevalue );

/*Bit read the value from a specific bit of a gpio path*/
extern int gpio_readbit( unsigned int gpio_pathnum, unsigned int bitx, unsigned int* readbit );

/*byte read the value of a specific gpio path*/
extern int gpio_readbyte( unsigned int gpio_pathnum, unsigned int* readbyte );

/* Enable single interrupt bit*/
extern int gpio_interruptenable( unsigned int gpio_pathnum, unsigned int bitx );

/* Enable single interrupt byte*/
extern int gpio_interruptenable_byte( unsigned int gpio_pathnum, unsigned int byte );

/*Cleae interrupt bit*/
extern int gpio_interruptclear( unsigned int gpio_pathnum, unsigned int bitx );

/*Cleae interrupt byte*/
extern int gpio_interruptclear_byte( unsigned int gpio_pathnum, unsigned int byte );

/* Disable single interrupt bit*/
extern int gpio_interruptdisable( unsigned int gpio_pathnum, unsigned int bitx );

/* Disable single interrupt byte*/
extern int gpio_interruptdisable_byte( unsigned int gpio_pathnum, unsigned int byte );


/* Interrupt mode set:include bothnsingle,levelnedge,risingnfalling. */
extern int gpio_interruptset( unsigned int gpio_pathnum, unsigned int bitx, unsigned int bothnsingle, unsigned int levelnedge, unsigned int risingnfalling );

/* Interrupt mode set:include bothnsingle,levelnedge,risingnfalling in a byte. */
extern int gpio_interruptset( unsigned int gpio_pathnum, unsigned int byte, unsigned int bothnsingle, unsigned int levelnedge, unsigned int risingnfalling );

/* Interrupt mode set:include bothnsingle,levelnedge,risingnfalling in a byte */
extern int gpio_interruptset_byte( unsigned int gpio_pathnum, unsigned int byte, unsigned int bothnsingle, unsigned int levelnedge, unsigned int risingnfalling );

/* Here just one routine assigned to pin0 of port0,just as a demo */
/* Also this routine can be defined in the app programm. */

#endif
