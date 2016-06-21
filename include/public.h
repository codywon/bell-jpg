#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define UPDATE_INDEX_CLOCK_PARAM	0
#define UPDATE_INDEX_BASE_PARAM		1
#define UPDATE_INDEX_NET_PARAM		2
#define UPDATE_INDEX_VIDEO_PARAM	3
#define UPDATE_INDEX_AUDIO_PARAM	4
#define UPDATE_INDEX_SERIAL_PARAM	5
#define UPDATE_INDEX_MOTION_PARAM	6
#define UPDATE_INDEX_SENSOR_PARAM	7
#define UPDATE_INDEX_VLOSS_PARAM	8
#define UPDATE_INDEX_VMASK_PARAM	9
#define UPDATE_INDEX_RECORD_PARAM	10

int create_sem( key_t key, int members );

int open_sem( key_t key );

int lock_sem( int semid, int member );
int unlock_sem( int semid, int member );

int open_shm( key_t key, int size, char** ppszAddr );

int close_shm( char* pszAddr );

#endif //__PUBLIC_H__
