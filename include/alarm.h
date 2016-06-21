#ifndef __IEALARM_H__
#define __IEALARM_H__

#define FAILED 		-1
#define SUCCESS		0

#define ALARM_OUT_PORT 	0
#define ALARM_OUT0  	11
#define ALARM_ON	1
#define ALARM_OFF	0

#define ALARM_IN_PORT   5
#define ALARM_IN0	6
#define ALARM_IN1       7
#define ALARM_IN2       8
#define ALARM_IN3       9

#define BYENABLE	1
#define BYDISABLE	0
#define MOTO_PORT       0

#define PTZ_STOP		0x00
#define PTZ_UP			0x01
#define PTZ_DOWN		0x02

#define	PTZ_LEFT		0x03
#define PTZ_LEFT_UP		0x04
#define	PTZ_LEFT_DOWN		0x05

#define	PTZ_RIGHT		0x06
#define	PTZ_RIGHT_UP		0x07
#define	PTZ_RIGHT_DOWN		0x08

#define	PTZ_AUTO		0x09
#define PTZ_LEFT_RIGHT		0x0a
#define PTZ_UP_DOWN		0x0b
#define	PTZ_CENTER		0x0c
#define	PTZ_PREFAB_BIT_SET	0x0d
#define PTZ_PREFAB_BIT_DEL	0x0e
#define	PTZ_PREFAB_BIT_RUN	0x0f

#define UART_START		0xe2
#define UART_END		0x23

#define MOTION_ALARM		0x01
#define GPIO_ALARM		0x02
#define NOTE_CLIENT_CAP_PIC 		0x07
#define NOTE_CLIENT_RECORD  		0x08

#define LIVE_MOTION			0x20
#define LIVE_GPIO			0x21

#define LIVE_MOTION_CLR		0x22
#define LIVE_GPIO_CLR		0x23

#define MOTION_ALARM_OFF      	0x22
#define GPIO_ALARM_OFF         	0x23


#define PTZ_UP_MJ               0
#define PTZ_STOP_UP_MJ          1

#define PTZ_DOWN_MJ             2
#define PTZ_STOP_DOWN_MJ        3

#define PTZ_LEFT_MJ             4
#define PTZ_STOP_LEFT_MJ	5

#define PTZ_RIGHT_MJ          	6
#define PTZ_RIGHT_STOP_MJ       7

#define PTZ_CENTER_MJ           25

#define PTZ_UP_DOWN_MJ          26
#define PTZ_UP_DOWN_STOP_MJ     27

#define PTZ_LEFT_RIGHT_MJ       28
#define PTZ_LEFT_RIGHT_STOP_MJ  29

#define PTZ_PREFAB_BIT_SET0     30
#define PTZ_PREFAB_BIT_RUN0     31

#define PTZ_PREFAB_BIT_SET1     32
#define PTZ_PREFAB_BIT_RUN1     33

#define PTZ_PREFAB_BIT_SET2     34
#define PTZ_PREFAB_BIT_RUN2     35

#define PTZ_PREFAB_BIT_SET3     36
#define PTZ_PREFAB_BIT_RUN3     37

#define PTZ_PREFAB_BIT_SET4     38
#define PTZ_PREFAB_BIT_RUN4     39

#define PTZ_PREFAB_BIT_SET5     40
#define PTZ_PREFAB_BIT_RUN5     41

#define PTZ_PREFAB_BIT_SET6     42
#define PTZ_PREFAB_BIT_RUN6     43

#define PTZ_PREFAB_BIT_SET7     44
#define PTZ_PREFAB_BIT_RUN7     45

#define PTZ_PREFAB_BIT_SET8     46
#define PTZ_PREFAB_BIT_RUN8     47

#define PTZ_PREFAB_BIT_SET9     48
#define PTZ_PREFAB_BIT_RUN9     49

#define PTZ_PREFAB_BIT_SETA     50
#define PTZ_PREFAB_BIT_RUNA     51

#define PTZ_PREFAB_BIT_SETB     52
#define PTZ_PREFAB_BIT_RUNB     53

#define PTZ_PREFAB_BIT_SETC     54
#define PTZ_PREFAB_BIT_RUNC     55

#define PTZ_PREFAB_BIT_SETD     56
#define PTZ_PREFAB_BIT_RUND     57

#define PTZ_PREFAB_BIT_SETE     58
#define PTZ_PREFAB_BIT_RUNE     59

#define PTZ_PREFAB_BIT_SETF     60
#define PTZ_PREFAB_BIT_RUNF     61

#define PTZ_LEFT_UP_MJ         	90
#define PTZ_RIGHT_UP_MJ         91
#define PTZ_LEFT_DOWN_MJ        92
#define PTZ_RIGHT_DOWN_MJ       93

#define MOTION_TYPE		0x01
#define SENSOR_TYPE		0x02

#define LED0			21
#define LED_PORT		4

#define DEF_KEY                 0
#define DEF_PORT                0


#define ALARM_TIMER_OVER	30
#define RECORD_TIMER		0x00
#define RECORD_ALARM		0x01
#define ALARM_MOTION		0x00
#define ALARM_GPIO		0x01

typedef struct _ALARMACTION
{
    char	flag;		//1->happen
    char	ftptime;	//ftp upload jpeg time
    char    mailcnt;	//mail jpeg
    char    time;		//happen time
} ALARMACTION, *PALARMACTION;

extern char 	presetsit;
//extern char	alarminhappen;
//extern char     defkeyhappen;
#endif //__SENSOR_H__
