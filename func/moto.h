#ifndef __MOTO_GPIO123_H__
#define __MOTO_GPIO123_H__
#include "debug.h"

#define CMD_PTZ_UP                      0
#define CMD_PTZ_UP_STOP                 1
#define CMD_PTZ_DOWN                    2
#define CMD_PTZ_DOWN_STOP               3
#define CMD_PTZ_LEFT                    4
#define CMD_PTZ_LEFT_STOP               5
#define CMD_PTZ_RIGHT                   6
#define CMD_PTZ_RIGHT_STOP             	7

#define CMD_PTZ_CENTER                  25
#define CMD_PTZ_UP_DOWN                 26
#define CMD_PTZ_UP_DOWN_STOP            27
#define CMD_PTZ_LEFT_RIGHT              28
#define CMD_PTZ_LEFT_RIGHT_STOP         29


#define CMD_PTZ_PREFAB_BIT_SET0         30
#define CMD_PTZ_PREFAB_BIT_SET1         32
#define CMD_PTZ_PREFAB_BIT_SET2         34
#define CMD_PTZ_PREFAB_BIT_SET3         36
#define CMD_PTZ_PREFAB_BIT_SET4         38
#define CMD_PTZ_PREFAB_BIT_SET5         40
#define CMD_PTZ_PREFAB_BIT_SET6         42
#define CMD_PTZ_PREFAB_BIT_SET7         44
#define CMD_PTZ_PREFAB_BIT_SET8         46
#define CMD_PTZ_PREFAB_BIT_SET9         48
#define CMD_PTZ_PREFAB_BIT_SETA         50
#define CMD_PTZ_PREFAB_BIT_SETB         52
#define CMD_PTZ_PREFAB_BIT_SETC         54
#define CMD_PTZ_PREFAB_BIT_SETD         56
#define CMD_PTZ_PREFAB_BIT_SETE         58
#define CMD_PTZ_PREFAB_BIT_SETF         60

#define CMD_PTZ_PREFAB_BIT_RUN0         31
#define CMD_PTZ_PREFAB_BIT_RUN1         33
#define CMD_PTZ_PREFAB_BIT_RUN2         35
#define CMD_PTZ_PREFAB_BIT_RUN3         37
#define CMD_PTZ_PREFAB_BIT_RUN4         39
#define CMD_PTZ_PREFAB_BIT_RUN5         41
#define CMD_PTZ_PREFAB_BIT_RUN6         43
#define CMD_PTZ_PREFAB_BIT_RUN7         45
#define CMD_PTZ_PREFAB_BIT_RUN8         47
#define CMD_PTZ_PREFAB_BIT_RUN9         49
#define CMD_PTZ_PREFAB_BIT_RUNA         51
#define CMD_PTZ_PREFAB_BIT_RUNB         53
#define CMD_PTZ_PREFAB_BIT_RUNC         55
#define CMD_PTZ_PREFAB_BIT_RUND         57
#define CMD_PTZ_PREFAB_BIT_RUNE         59
#define CMD_PTZ_PREFAB_BIT_RUNF         61

#define CMD_PTZ_LEFT_UP                 90
#define CMD_PTZ_RIGHT_UP                91
#define CMD_PTZ_LEFT_DOWN               92
#define CMD_PTZ_RIGHT_DOWN              93

#define CMD_PTZ_IO_HIGH              	94
#define CMD_PTZ_IO_LOW              	95

/* BEGIN: Added by wupm, 2013/7/1 */
#define CMD_PTZ_IRCUT_HIGH              		96
#define CMD_PTZ_IRCUT_LOW              		97

#define CMD_PTZ_IO_SPEED              	100
#define CMD_PTZ_MOTO_TEST				255

#define GPIO_PTZ_STOP                        0x00
#define GPIO_PTZ_UP                          0x01
#define GPIO_PTZ_DOWN                        0x02
#define GPIO_PTZ_LEFT                        0x03
#define GPIO_PTZ_LEFT_UP                     0x04
#define GPIO_PTZ_LEFT_DOWN                   0x05
#define GPIO_PTZ_RIGHT                       0x06
#define GPIO_PTZ_RIGHT_UP                    0x07
#define GPIO_PTZ_RIGHT_DOWN                  0x08
#define GPIO_PTZ_AUTO                        0x09
#define GPIO_PTZ_LEFT_RIGHT                  0x0a
#define GPIO_PTZ_UP_DOWN                     0x0b
#define GPIO_PTZ_CENTER                      0x0c
#define GPIO_PTZ_PREFAB_BIT_SET              0x0d
#define GPIO_PTZ_PREFAB_BIT_DEL              0x0e
#define GPIO_PTZ_PREFAB_BIT_RUN              0x0f
#define GPIO_PTZ_LEFTRIGHT_STOP	16
#define GPIO_PTZ_UPDOWN_STOP	17
#define	GPIO_PTZ_MOTO_TEST		18

/* BEGIN: Added by wupm, 2013/7/1 */
//#ifdef	USE_NEW_MOTO_SOLUTION
#define MOTO_STOP                        	0
#define MOTO_UP                          	1
#define MOTO_DOWN                       	2
#define MOTO_LEFT                        	3
#define MOTO_LEFT_UP                   	4
#define MOTO_LEFT_DOWN              	5
#define MOTO_RIGHT                       	6
#define MOTO_RIGHT_UP                 	7
#define MOTO_RIGHT_DOWN            	8
#define MOTO_AUTO                        	9
#define MOTO_LEFT_RIGHT                10
#define MOTO_UP_DOWN                    11
#define MOTO_CENTER                      	 12
#define MOTO_PREFAB_BIT_SET         13
#define MOTO_PREFAB_BIT_DEL         14
#define MOTO_PREFAB_BIT_RUN        15
#define MOTO_LEFTRIGHT_STOP		 16
#define MOTO_UPDOWN_STOP		 17

/* BEGIN: Modified by wupm, 2013/7/1 */
//#define MOTO_TEST	 			255
#define MOTO_TEST	 			18
//#endif

#define PTZ_LEVEL_STOP_GPIO             0x00
#define PTZ_UP_GPIO                     0x01
#define PTZ_DOWN_GPIO                   0x02
#define PTZ_LEFT_GPIO                   0x03
#define PTZ_RIGHT_GPIO                  0x04
#define PTZ_VERT_STOP_GPIO              0x05

///////////////////////////////////////////////////////////////////////////
#define PTZ_VERT_MAX_TIMER              8000

#ifdef	REMOVE_UPDOWN_SWITCH
#undef	PTZ_VERT_MAX_TIMER
#define	PTZ_VERT_MAX_TIMER              160

#ifdef	PRODUCT_CATELOG_WHITE
#undef	PTZ_VERT_MAX_TIMER
#define	PTZ_VERT_MAX_TIMER				200
#endif

#ifdef	PRODUCT_CATELOG_HULU
#undef	PTZ_VERT_MAX_TIMER
#define	PTZ_VERT_MAX_TIMER				160
#endif

#ifdef	PRODUCT_CATELOG_CIRCLE
#undef	PTZ_VERT_MAX_TIMER
#define	PTZ_VERT_MAX_TIMER              560
#endif

#ifdef	PRODUCT_CATELOG_PSD
#undef	PTZ_VERT_MAX_TIMER
#define	PTZ_VERT_MAX_TIMER              100
#endif

#ifdef PRODUCT_CATELOG_COMMON_PSD
#undef PTZ_VERT_MAX_TIMER
#define PTZ_VERT_MAX_TIMER              85  //150
#endif

#ifdef	PRODUCT_CATELOG_GOSBELL
#undef	PTZ_VERT_MAX_TIMER
#define	PTZ_VERT_MAX_TIMER              140
#endif


#endif


///////////////////////////////////////////////////////////////////////////
#define PTZ_LEVEL_MAX_TIMER				8000

#ifdef	REMOVE_LEFTRIGHT_SWITCH
#undef	PTZ_LEVEL_MAX_TIMER
#define	PTZ_LEVEL_MAX_TIMER				530

#ifdef	PRODUCT_CATELOG_WHITE
#undef	PTZ_LEVEL_MAX_TIMER
#define	PTZ_LEVEL_MAX_TIMER				640
#endif

#ifdef	PRODUCT_CATELOG_HULU
#undef	PTZ_LEVEL_MAX_TIMER
#define	PTZ_LEVEL_MAX_TIMER				1060
#endif

#ifdef	PRODUCT_CATELOG_CIRCLE
#undef	PTZ_LEVEL_MAX_TIMER
#define	PTZ_LEVEL_MAX_TIMER             2300
#endif

#ifdef	PRODUCT_CATELOG_PSD
#undef	PTZ_LEVEL_MAX_TIMER
#define	PTZ_LEVEL_MAX_TIMER             400
#endif

#ifdef PRODUCT_CATELOG_COMMON_PSD
#undef PTZ_LEVEL_MAX_TIMER
#define PTZ_LEVEL_MAX_TIMER             385 //380
#endif

#ifdef PRODUCT_CATELOG_GOSBELL
#undef	PTZ_LEVEL_MAX_TIMER
#define	PTZ_LEVEL_MAX_TIMER             530
#endif

#endif


/* BEGIN: Added by wupm, 2013/7/1 */
#ifdef	USE_NEW_MOTO_SOLUTION
//#ifdef	REMOVE_LEFTRIGHT_SWITCH
#undef	MOTO_LEFTRIGHT_MAX_TIMES
#define	MOTO_LEFTRIGHT_MAX_TIMES	(PTZ_LEVEL_MAX_TIMER * 8)
//#endif

//#ifdef	REMOVE_UPDOWN_SWITCH
#undef	MOTO_UPDOWN_MAX_TIMES
#define	MOTO_UPDOWN_MAX_TIMES		(PTZ_VERT_MAX_TIMER	* 8 )
//#endif
#endif

typedef struct _MOTOSIT
{
    unsigned short 	leveltime;
    unsigned short 	verttime;
    unsigned char	levelruntime;
    unsigned char	vertruntime;
    unsigned char	levelruntime1;
    unsigned char	vertruntime1;

    /* BEGIN: Added by wupm, 2013/3/29 */
    unsigned char 	Motocmd;
    unsigned char 	Motospeed;

    unsigned short  mototimer;
    unsigned char	motoflag;
    unsigned char	onstart;

    unsigned char 	leveldir;
    unsigned char 	vertdir;
    unsigned char 	centflag;
    unsigned char 	dirflag;
    unsigned short 	dircnt;
    unsigned char 	stepflag;
    unsigned char   onestepvalue;
    unsigned short 	callpresettime;
    unsigned short 	callpresettime1;
    unsigned char 	callflag;

    unsigned short	overtime0;
    unsigned short  overtime1;
    unsigned short	overtime2;
    unsigned short	overtime3;

    unsigned short 	maxlevel;
    unsigned short 	maxvert;

    unsigned char 	levelflag;
    unsigned char 	vertflag;

    unsigned char 	levelcirr;
    unsigned char 	vertcirr;
    /* END:   Added by wupm, 2013/3/29 */
} MOTOSIT, *PMOTOSIT;

void MotoCmdAction( unsigned char Motocmd );

#endif
