/* Note that only ToAscii and KF_REPEAT are not defined. Some of the
 * translations may be questionable, marked with ????. */

#ifndef OPEN32_WIN_H
#define OPEN32_WIN_H

#define __OPEN32__

#ifndef NO_WIN_H
#ifdef VOID
#undef VOID
#undef NULL
#define CHAR CHARmy
#define LONG LONGmy
#define SHORT SHORTmy
#define wchar_t wchar_t_my
#define WINGDIAPI
#define _System
#endif /* def VOID */

#ifdef WORD
#  undef WORD
#endif /* def VOID */

#include <os2win.h>

#endif /* ndef NO_WIN_H */

#define max(a,b) ((a) > (b) ? (a) : (b))

/* Is in not-executed branch for win32s */
#define HKEY_CLASSES_ROOT      HKEY_LOCAL_MACHINE

/* result = ToAscii(keycode, scancode, keys, (LPWORD) buf, 0); */
#define ToAscii(keycode, scancode, keys, buf, a) \
	(*buf = StashedKey, (StashedKey != 0))

 /* Obviously wrong ???? */

extern char StashedKey;

/* 	    if (tm.tmPitchAndFamily & TMPF_TRUETYPE) { */
#define TMPF_TRUETYPE 0

/* SetMessageQueue(64); */
#define SetMessageQueue()

/* 		if (RealizePalette(dc)) { */
/* 		    UpdateColors(dc); */
/* 		} */
#define UpdateColors(dc)	/* ????? */

/* What follows is the example how it is used: */
/* #define EX_OVERRIDE_STYLE (WS_EX_TOOLWINDOW); */
/* #define EX_TRANSIENT_STYLE (WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME); */
#define WS_EX_DLGMODALFRAME 0	/* ????? */
#define WS_EX_TOOLWINDOW 0	/* ????? */

/* This means win32s: */
/* 	if ((GetVersion() & 0x80000000)) { */

#define GetVersion() 0

/*     MessageBox(NULL, buf, "Fatal Error in Wish", */
/* 	    MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND); */


#define MB_TASKMODAL 0		/* ????? */

/* prevState = HIWORD(lParam) & KF_REPEAT; */
#define KF_REPEAT 0		/* Obviously wrong ?????? */

typedef struct {
    char dwOSVersionInfoSize;
    char dwPlatformId;
    char dwMajorVersion;
    char dwMinorVersion;
    char dwBuildNumber;
} OSVERSIONINFO;
/* Note that we do not initialize the version. ????? */
#define GetVersionEx(pinfo) ((pinfo)->dwPlatformId = VER_PLATFORM_WIN32s + 1)
#define VER_PLATFORM_WIN32s 45
#define TclWinFlushEvents()		/* ????? */
/*    ignoreEvents = 1;
    while (Tcl_DoOneEvent(TCL_DONT_WAIT|TCL_WINDOW_EVENTS|TCL_IDLE_EVENTS)) {
    }
    ignoreEvents = 0; */
/* Apparently bug: ReleaseDc(NULL,dc) dies. */

/* #define ReleaseDC(hwnd,dc) (hwnd ? ReleaseDC(hwnd,dc) : 0) */
#define ReleaseDC MyReleaseDC
#define GetDC MyGetDC

/* Unknown messages: */
#define WM_ENTERSIZEMOVE 30000
#define WM_EXITSIZEMOVE 30001

HCURSOR os2LoadCursor(HMODULE module, char *name);
HICON os2LoadIcon(HMODULE module, char *name);
HDC MyGetDC( HWND h);
int MyReleaseDC( HWND h, HDC dc);

/* To use in ifdef(__OPEN32__) */
extern HWND tmpParent;
extern unsigned long dllHandle;

/* In file dialogs: */

struct dummy_LPOFNOTIFY {struct {int code;} hdr;};

#define LPOFNOTIFY struct dummy_LPOFNOTIFY* /* ????? */

#define WM_NOTIFY 0xFFFF /* ????? */
#define OFN_EXPLORER 0 /* Not used */
#define CDN_SELCHANGE		0 /* Not used */
#define CDN_FOLDERCHANGE	1 /* Not used */
#define CDN_HELP		2 /* Not used */
#define CDN_FILEOK		3 /* Not used */
#define CDN_SHAREVIOLATION	4 /* Not used */
#define MB_ICONERROR		MB_ICONSTOP
#define MB_ICONWARNING		MB_ICONEXCLAMATION
#define MB_DEFBUTTON4		MB_DEFBUTTON3

#define OpenIcon(arg)

#endif /* OPEN32_WIN_H */
