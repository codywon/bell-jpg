#ifndef __UPDATE_H__
#define __UPDATE_H__
typedef struct __file_index
{
    char		filename[64];
    unsigned int	filelen;
} FILEINDEX, *PFILEINDEX;

typedef struct __file_head
{
    char	sys[3];
    char	factory;
    char	version[4];
} FILEHEAD, *PFILEHEAD;

int Upgradesystem( char* path );

#endif

