/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "wio.h"
#include "global.h"
#include "dbtable.h"
#include "write.h"
#include "rcrtns.h"
#include "clibext.h"
#include "rccore.h"


typedef struct {
    DBTableHeader       header;
    char                begchars[256];
    DBIndexEntry        *index;
    uint_16             *entries;
}DBCharInfo;

static DBCharInfo       charInfo;

static RcStatus readDBHeader( WResFileID fp )
{
    int                 ret;

    ret = RCREAD( fp, &charInfo.header, sizeof( DBTableHeader ) );
    if( ret != sizeof( DBTableHeader ) ) {
        if( ret == -1 ) {
            return( RS_READ_ERROR );
        } else {
            return( RS_READ_INCMPLT );
        }
    }
    if( charInfo.header.sig[0] != DB_TABLE_SIG_1 ||
        charInfo.header.sig[1] != DB_TABLE_SIG_2 ) {
        return( RS_BAD_FILE_FMT );
    }
    if( charInfo.header.ver != DB_TABLE_VER ) {
        return( RS_WRONG_VER );
    }
    return( RS_OK );
}

static RcStatus readDBRanges( WResFileID fp )
{
    int                 ret;

    ret = RCREAD( fp, &charInfo.begchars, 256 );
    if( ret != 256 ) {
        if( ret == -1 ) {
            return( RS_READ_ERROR );
        } else {
            return( RS_READ_INCMPLT );
        }
    }
    return( RS_OK );
}

static RcStatus readDBIndex( WResFileID fp )
{
    int                 ret;
    int                 size;

    size = charInfo.header.num_indices * sizeof( DBIndexEntry );
    charInfo.index = RCALLOC( size );
    ret = RCREAD( fp, charInfo.index, size );
    if( ret != size ) {
        if( ret == -1 ) {
            return( RS_READ_ERROR );
        } else {
            return( RS_READ_INCMPLT );
        }
    }
    return( RS_OK );
}

static RcStatus readDBTable( WResFileID fp )
{
    int                 ret;
    int                 size;

    size = charInfo.header.num_entries * sizeof( uint_16 );
    charInfo.entries = RCALLOC( size );
    ret = RCREAD( fp, charInfo.entries, size );
    if( ret != size ) {
        if( ret == -1 ) {
            return( RS_READ_ERROR );
        } else {
            return( RS_READ_INCMPLT );
        }
    }
    return( RS_OK );
}

RcStatus OpenTable( char *fname, char *path )
{
    WResFileID  fp;
    RcStatus    status;

    status = RS_OK;
    _searchenv( fname, "PATH", path );
    if( path[0] == '\0' )
        return( RS_FILE_NOT_FOUND );
    fp = RCOPEN( path, O_RDONLY | O_BINARY );
    if( fp == NIL_HANDLE ) {
        status = RS_OPEN_ERROR;
    }
    if( status == RS_OK )
        status = readDBHeader( fp );
    if( status == RS_OK )
        status = readDBRanges( fp );
    if( status == RS_OK )
        status = readDBIndex( fp );
    if( status == RS_OK )
        status = readDBTable( fp );
    if( status != RS_OPEN_ERROR )
        RCCLOSE( fp );
    if( status == RS_OK ) {
        ConvToUnicode = DBStringToUnicode;
    }
    return( status );
}

static uint_16 lookUpDBChar( uint_16 ch ) {

    int         i;
    int         index;

    for( i=0; i < charInfo.header.num_indices; i++ ) {
        if( ch >= charInfo.index[i].min && ch <= charInfo.index[i].max ) {
            index = charInfo.index[i].base + ch - charInfo.index[i].min;
            return( charInfo.entries[ index ] );
        }
    }
    return( -1 );
}

const char *GetLeadBytes( void ) {
    return( charInfo.begchars );
}

int DBStringToUnicode( int len, const char *str, char *buf ) {

    const uint_8    *ptr;
    uint_16         *ubuf;
    uint_16         dbchar;
    int             ret;

    ptr = (uint_8 *)str;
    ret = 0;
    ubuf = (uint_16 *)buf;
    while( (char *)ptr < str + len ) {
        if( charInfo.begchars[ *ptr ] == DB_BEG_CHAR ) {
            dbchar = *ptr << 8;
            ptr++;
            dbchar |= *ptr;
        } else {
            dbchar = *ptr;
        }
        if( ubuf != NULL ) {
            *ubuf = lookUpDBChar( dbchar );
            ubuf++;
        }
        ptr ++;
        ret +=2;
    }
    return( ret );
}

void FiniTable( void ) {
/***********************/
    if( charInfo.index != NULL ) {
        RCFREE( charInfo.index );
        charInfo.index = NULL;
    }
    if( charInfo.entries != NULL ) {
        RCFREE( charInfo.entries );
        charInfo.entries = NULL;
    }
}

extern void DbtableInitStatics( void )
/************************************/
{
    memset( &charInfo, 0, sizeof( DBCharInfo ) );
}

