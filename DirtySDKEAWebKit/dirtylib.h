/*
Copyright (C) 2001-2002,2009 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*H*************************************************************************************************/
/*!
    \File    dirtylib.h

*/
/*************************************************************************************************H*/


#ifndef _dirtylib_h
#define _dirtylib_h


//@{

/*** Include files *********************************************************************/

#include "dirtydefs.h"

/*** Defines ***************************************************************************/

#ifndef NULL
 #ifdef __cplusplus
  #define NULL 0
 #else
  #define NULL ((void *)0)
 #endif
#endif

#ifndef NetPrintf
 #if DIRTYCODE_DEBUG
 #else
 #endif
#endif

#if DIRTYCODE_DEBUG
 #define NetPrintf(_x) NetPrintfCode _x
 #define NetPrintfVerbose(_x) NetPrintfVerboseCode _x
 #define NetPrintMem(_pMem, _iSize, _pTitle) NetPrintMemCode(_pMem, _iSize, _pTitle)
 #define NetPrintWrap(_pString, _iWrapCol) NetPrintWrapCode(_pString, _iWrapCol)
#else
// #define NetPrintf(_x) { }
// #define NetPrintfVerbose(_x) { }
// #define NetPrintMem(_pMem, _iSize, _pTitle) { }
// #define NetPrintWrap(_pString, _iWrapCol) { }

  #include <stdio.h>
  #define NetPrintf(_x) {printf _x;}
  #define NetPrintfVerbose(_x) {printf _x;}
  #define NetPrintMem(_pMem, _iSize, _pTitle) { }
  #define NetPrintWrap(_pString, _iWrapCol) { }
#endif

/*** Macros ****************************************************************************/

/*** Type Definitions ******************************************************************/

/*** Variables *************************************************************************/

/*** Functions *************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


void NetPrintMemCode(const void *pMem, int32_t iSize, const char *pTitle);

void NetPrintWrapCode(const char *pData, int32_t iWrapCol);

uint32_t NetTick(void);

#define NetTickDiff(_uNewTime, _uOldTime) ((signed)((_uNewTime) - (_uOldTime)))
int32_t NetPrintfCode(const char *fmt, ...);

void NetPrintfVerboseCode(int32_t iVerbosityLevel, int32_t iCheckLevel, const char *pFormat, ...);

#if DIRTYCODE_DEBUG
void NetPrintfHook(int32_t (*pPrintfDebugHook)(void *pParm, const char *pText), void *pParm);
#endif

#ifdef __cplusplus
}
#endif

//@}

#endif // _dirtylib_h
