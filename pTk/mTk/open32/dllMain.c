/*****************************************************************************
 * Copyright:                                                                *
 *   Licensed Materials - Property of IBM                                    *
 *   (C) Copyright IBM Corp. 1995                                            *
 *   All Rights Reserved                                                     *
 *****************************************************************************
 * File: dllmain.c                                                           *
 *****************************************************************************
 * Description:                                                              *
 *   This file contains the DLL initialization/Termination function, which   *
 *   is called when a process gains or loses access to the DLL.  The DEF     *
 *   file used to build the DLL needs to specify INITINSTANCE and            *
 *   TERMINSTANCE, otherwise this function will only be called for the       *
 *   first process to gain access and the last process to free up the DLL.   *
 *                                                                           *
 *   This implementation is for IBM C/Set++ and assumes that the 'C'         *
 *   Runtime library is being statically linked to the DLL and that the      *
 *   library uses C++ classes.                                               *
 *                                                                           *
 * Restrictions:                                                             *
 *   DLL_THREAD_ATTACH/DETACH is not supported. DisableThreadLibraryCalls()  *
 *   is also not supported.                                                  *
 *                                                                           *
 *   lpvReserved always indicates that the DLL was loaded statically even    *
 *   when the DLL was dynamically loaded.                                    *
 *****************************************************************************/
#ifdef __WIN32__
#  define WINGDIAPI
#  include <os2win.h>
#else
#  define TRUE 1
#endif

#define _System
#include <stdio.h>

struct Tcl_Time;
extern int (*Tcl_WaitForEventProc)(int, long *, struct Tcl_Time *);
extern int OS2Tcl_WaitForEvent(int, long *, struct Tcl_Time *);

int _CRT_init(void);
void _CRT_term(void);
void __ctordtorInit(void);
void __ctordtorTerm(void);

static init;
unsigned long dllHandle;

unsigned long _System _DLL_InitTerm(unsigned long handle, unsigned long flag)
{
   unsigned long reserved;

   if ( flag )
   {
      /* Termination: A process is losing access to this DLL
       */

       if (!init) {
	   fputs("error: Open32 DLL Term called before Init.\n"
		 "       Is it called from PM program?\n",
		 stderr);
	   fflush(stderr);
       }

      /* Call Win32 Initialization/Termination function
       * (NOTE: this assumes the DLL entry point is a function called
       *        DllEntryPoint)
       */
#ifdef __WIN32__
      DllEntryPoint((HANDLE) handle, DLL_PROCESS_DETACH, &reserved);
#endif

      __ctordtorTerm();       /* Termination code for C++ Objects */
      _CRT_term();            /* Assumes static linking of C Runtime */
      return TRUE;
   }
   else
   {
      /* Initialization: A process is gaining access to this DLL
       */
       dllHandle = handle;
      if ( _CRT_init() == -1 ) /* Initialize C Runtime */
         return 0;             /* If initialization failed return 0 */

      __ctordtorInit();        /* Initialization code for C++ Objects */

      /* Call Win32 Initialization/Termination function
       * (NOTE: this assumes the DLL entry point is a function called
       *        DllEntryPoint)
       */
      Tcl_WaitForEventProc = &OS2Tcl_WaitForEvent;
      init = 1;
#ifdef __WIN32__
      return DllEntryPoint((HANDLE) handle, DLL_PROCESS_ATTACH, &reserved) != 0;
#else
      TkOS2InitPM();
      TkOS2XInit(TkOS2GetAppInstance());
      return TRUE;
#endif
   }
}

