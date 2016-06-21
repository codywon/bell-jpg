//
//
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

static char card[64] = "default";
//
int main( int argc, char** argv )
{
    int err;
    int orig_volume           = 0;
    int uVolume;
    static snd_ctl_t*  handle = NULL;
    snd_ctl_elem_info_t*  info;
    snd_ctl_elem_id_t*    id;
    snd_ctl_elem_value_t* control;
    unsigned int count;
    snd_ctl_elem_type_t type;
    snd_ctl_elem_info_alloca( &info );
    snd_ctl_elem_id_alloca( &id );
    snd_ctl_elem_value_alloca( &control );
    unsigned int idx;
    snd_ctl_elem_id_set_interface( id, SND_CTL_ELEM_IFACE_MIXER ); /* default */

    if ( !strcmp( argv[1], "play-vol" ) )
    {
        uVolume = atoi( argv[2] );

        if ( uVolume < 0 || uVolume > 63 )
        {
            printf( "Audio only support 0-63 level volume\n" );
            exit( 1 );
        }

        snd_ctl_elem_id_set_name( id, "Master Playback Volume" );

        if ( ( err = snd_ctl_open( &handle, card, 0 ) ) < 0 )
        {
            fprintf( stderr, "Control %s open error: %s\n", card, snd_strerror( err ) );
            exit( 1 );
        }

        snd_ctl_elem_info_set_id( info, id );

        if ( ( err = snd_ctl_elem_info( handle, info ) ) < 0 )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        type = snd_ctl_elem_info_get_type( info );
        count = snd_ctl_elem_info_get_count( info );

        if ( type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        snd_ctl_elem_value_set_id( control, id );

        if ( !snd_ctl_elem_read( handle, control ) )
        {
            orig_volume = snd_ctl_elem_value_get_integer( control, 0 );
        }

        if ( uVolume != orig_volume )
        {
            fprintf( stderr, "uVolume != orig_volume ##################### new_value(%d) orgin_value(%d)\n", uVolume, orig_volume );
            snd_ctl_elem_value_set_integer( control, 0, uVolume );

            if ( ( err = snd_ctl_elem_write( handle, control ) ) < 0 )
            {
                fprintf( stderr, "Control %s element write error: %s\n", card, snd_strerror( err ) );
                snd_ctl_close( handle );
                handle = NULL;
                exit( 1 );
            }
        }
    }

    if ( !strcmp( argv[1], "mic1-vol" ) )
    {
        uVolume = atoi( argv[2] );

        if ( uVolume < 0 || uVolume > 3 )
        {
            printf( "MIc1 only support 0-3 level volume\n" );
            exit( 1 );
        }

        snd_ctl_elem_id_set_name( id, "MicL Volume" );

        if ( ( err = snd_ctl_open( &handle, card, 0 ) ) < 0 )
        {
            fprintf( stderr, "Control %s open error: %s\n", card, snd_strerror( err ) );
            exit( 1 );
        }

        snd_ctl_elem_info_set_id( info, id );

        if ( ( err = snd_ctl_elem_info( handle, info ) ) < 0 )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        type = snd_ctl_elem_info_get_type( info );
        count = snd_ctl_elem_info_get_count( info );

        if ( type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        snd_ctl_elem_value_set_id( control, id );

        if ( !snd_ctl_elem_read( handle, control ) )
        {
            orig_volume = snd_ctl_elem_value_get_integer( control, 0 );
        }

        if ( uVolume != orig_volume )
        {
            fprintf( stderr, "uVolume != orig_volume ##################### new_value(%d) orgin_value(%d)\n", uVolume, orig_volume );
            snd_ctl_elem_value_set_integer( control, 0, uVolume );

            if ( ( err = snd_ctl_elem_write( handle, control ) ) < 0 )
            {
                fprintf( stderr, "Control %s element write error: %s\n", card, snd_strerror( err ) );
                snd_ctl_close( handle );
                handle = NULL;
                exit( 1 );
            }
        }
    }

    if ( !strcmp( argv[1], "mic2-vol" ) )
    {
        uVolume = atoi( argv[2] );

        if ( uVolume < 0 || uVolume > 3 )
        {
            printf( "MIc2 only support 0-3 level volume\n" );
            exit( 1 );
        }

        snd_ctl_elem_id_set_name( id, "MicR Volume" );

        if ( ( err = snd_ctl_open( &handle, card, 0 ) ) < 0 )
        {
            fprintf( stderr, "Control %s open error: %s\n", card, snd_strerror( err ) );
            exit( 1 );
        }

        snd_ctl_elem_info_set_id( info, id );

        if ( ( err = snd_ctl_elem_info( handle, info ) ) < 0 )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        type = snd_ctl_elem_info_get_type( info );
        count = snd_ctl_elem_info_get_count( info );

        if ( type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        snd_ctl_elem_value_set_id( control, id );

        if ( !snd_ctl_elem_read( handle, control ) )
        {
            orig_volume = snd_ctl_elem_value_get_integer( control, 0 );
        }

        if ( uVolume != orig_volume )
        {
            fprintf( stderr, "uVolume != orig_volume ##################### new_value(%d) orgin_value(%d)\n", uVolume, orig_volume );
            snd_ctl_elem_value_set_integer( control, 0, uVolume );

            if ( ( err = snd_ctl_elem_write( handle, control ) ) < 0 )
            {
                fprintf( stderr, "Control %s element write error: %s\n", card, snd_strerror( err ) );
                snd_ctl_close( handle );
                handle = NULL;
                exit( 1 );
            }
        }
    }

    if ( !strcmp( argv[1], "rec-adc-vol" ) )
    {
        uVolume = atoi( argv[2] );

        if ( uVolume < 0 || uVolume > 7 )
        {
            printf( "Rec Adc Volume only support 0-7 level volume\n" );
            exit( 1 );
        }

        snd_ctl_elem_id_set_name( id, "Capture Volume" );

        if ( ( err = snd_ctl_open( &handle, card, 0 ) ) < 0 )
        {
            fprintf( stderr, "Control %s open error: %s\n", card, snd_strerror( err ) );
            exit( 1 );
        }

        snd_ctl_elem_info_set_id( info, id );

        if ( ( err = snd_ctl_elem_info( handle, info ) ) < 0 )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        type = snd_ctl_elem_info_get_type( info );
        count = snd_ctl_elem_info_get_count( info );

        if ( type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        snd_ctl_elem_value_set_id( control, id );

        if ( !snd_ctl_elem_read( handle, control ) )
        {
            orig_volume = snd_ctl_elem_value_get_integer( control, 0 );
        }

        if ( uVolume != orig_volume )
        {
            fprintf( stderr, "uVolume != orig_volume ##################### new_value(%d) orgin_value(%d)\n", uVolume, orig_volume );
            snd_ctl_elem_value_set_integer( control, 0, uVolume );

            if ( ( err = snd_ctl_elem_write( handle, control ) ) < 0 )
            {
                fprintf( stderr, "Control %s element write error: %s\n", card, snd_strerror( err ) );
                snd_ctl_close( handle );
                handle = NULL;
                exit( 1 );
            }
        }
    }

    if ( !strcmp( argv[1], "fm-vol" ) )
    {
        uVolume = atoi( argv[2] );

        if ( uVolume < 0 || uVolume > 7 )
        {
            printf( "Rec Fm Volume only support 0-7 level volume\n" );
            exit( 1 );
        }

        snd_ctl_elem_id_set_name( id, "Fm Volume" );

        if ( ( err = snd_ctl_open( &handle, card, 0 ) ) < 0 )
        {
            fprintf( stderr, "Control %s open error: %s\n", card, snd_strerror( err ) );
            exit( 1 );
        }

        snd_ctl_elem_info_set_id( info, id );

        if ( ( err = snd_ctl_elem_info( handle, info ) ) < 0 )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        type = snd_ctl_elem_info_get_type( info );
        count = snd_ctl_elem_info_get_count( info );

        if ( type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        snd_ctl_elem_value_set_id( control, id );

        if ( !snd_ctl_elem_read( handle, control ) )
        {
            orig_volume = snd_ctl_elem_value_get_integer( control, 0 );
        }

        if ( uVolume != orig_volume )
        {
            fprintf( stderr, "uVolume != orig_volume ##################### new_value(%d) orgin_value(%d)\n", uVolume, orig_volume );
            snd_ctl_elem_value_set_integer( control, 0, uVolume );

            if ( ( err = snd_ctl_elem_write( handle, control ) ) < 0 )
            {
                fprintf( stderr, "Control %s element write error: %s\n", card, snd_strerror( err ) );
                snd_ctl_close( handle );
                handle = NULL;
                exit( 1 );
            }
        }
    }

    if ( !strcmp( argv[1], "line-vol" ) )
    {
        uVolume = atoi( argv[2] );

        if ( uVolume < 0 || uVolume > 7 )
        {
            printf( "Line Volume only support 0-7 level volume\n" );
            exit( 1 );
        }

        snd_ctl_elem_id_set_name( id, "Line Volume" );

        if ( ( err = snd_ctl_open( &handle, card, 0 ) ) < 0 )
        {
            fprintf( stderr, "Control %s open error: %s\n", card, snd_strerror( err ) );
            exit( 1 );
        }

        snd_ctl_elem_info_set_id( info, id );

        if ( ( err = snd_ctl_elem_info( handle, info ) ) < 0 )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        type = snd_ctl_elem_info_get_type( info );
        count = snd_ctl_elem_info_get_count( info );

        if ( type != SND_CTL_ELEM_TYPE_INTEGER || 1 != count )
        {
            fprintf( stderr, "Cannot find the given element from control %s\n", card );
            snd_ctl_close( handle );
            handle = NULL;
            exit( 1 );
        }

        snd_ctl_elem_value_set_id( control, id );

        if ( !snd_ctl_elem_read( handle, control ) )
        {
            orig_volume = snd_ctl_elem_value_get_integer( control, 0 );
        }

        if ( uVolume != orig_volume )
        {
            fprintf( stderr, "uVolume != orig_volume ##################### new_value(%d) orgin_value(%d)\n", uVolume, orig_volume );
            snd_ctl_elem_value_set_integer( control, 0, uVolume );

            if ( ( err = snd_ctl_elem_write( handle, control ) ) < 0 )
            {
                fprintf( stderr, "Control %s element write error: %s\n", card, snd_strerror( err ) );
                snd_ctl_close( handle );
                handle = NULL;
                exit( 1 );
            }
        }
    }

    snd_ctl_close( handle );
    handle = NULL;
    return 0;
}
