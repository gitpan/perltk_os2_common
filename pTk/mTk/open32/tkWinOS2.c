/* PM code in this file was stolen from Illya Vaes's code */

#ifdef __WIN32__
#  include "tkWinInt.h"
/* We keep a char here to emulate ToAscii later. Note that ToAscii
   is used before any WM_CHAR is recieved to init the map. We hope
   for the best here. */
HWND tmpParent;
#else
#  define __PM_WIN__
#  include "tkOS2Int.h"
static int ignoreEvents = 0;
#endif

extern char StashedKey;
char StashedKey = 0;	/* fake for PM */
/*
 *----------------------------------------------------------------------
 *
 * OS2Tcl_WaitForEvent --
 *
 *	This procedure does the lowest level wait for events in a
 *	platform-specific manner.  It uses information provided by
 *	previous calls to Tcl_WatchFile, plus the timePtr argument,
 *	to determine what to wait for and how long to wait.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	May put the process to sleep for a while, depending on timePtr.
 *	When this procedure returns, an event of interest to the application
 *	has probably, but not necessarily, occurred.
 *
 *----------------------------------------------------------------------
 */

int
OS2Tcl_WaitForEvent(n, maskPtr, timePtr)
    int n;
    long *maskPtr;		/* file part, not used so far. */
    Tcl_Time *timePtr;		/* Specifies the maximum amount of time
				 * that this procedure should block before
				 * returning.  The time is given as an
				 * interval, not an absolute wakeup time.
				 * NULL means block forever. */
{
#ifdef __WIN32__
    MSG msg;
#else
    QMSG msg;
#endif
    int foundEvent = 1;

    /*
     * If we are ignoring events from the system, just return immediately.
     */
#ifndef __WIN32__
    if (ignoreEvents) {
	return 0;
    }
#endif
    /*
     * Set up the asynchronous select handlers for any sockets we
     * are watching.
     */

    /* TclWinNotifySocket(); */

    /*
     * Look for an event, setting a timer so we don't block forever.
     */

    if (timePtr != NULL) {
	UINT ms;
	ms = timePtr->sec * 1000;
	ms += timePtr->usec / 1000;

	if (ms > 0) {
#ifdef __WIN32__
	    UINT timerHandle = SetTimer(tmpParent, 0, ms, NULL);
	    GetMessage(&msg, NULL, 0, 0);
	    KillTimer(tmpParent, timerHandle);
#else
	    ULONG timerHandle = WinStartTimer(hab, NULLHANDLE, 1, ms);
	    WinGetMsg(hab, &msg, NULLHANDLE, 0, 0);
	    WinStopTimer(hab, NULLHANDLE, timerHandle);
#endif
	} else {

	    /*
	     * If the timeout is too small, we just poll.
	     */

#ifdef __WIN32__
	    foundEvent = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
#else
	    foundEvent = WinPeekMsg(hab, &msg, NULLHANDLE, 0, 0, PM_REMOVE);
#endif
	}
    } else {
#ifdef __WIN32__
	GetMessage(&msg, NULL, 0, 0);
#else
	WinGetMsg(hab, &msg, NULLHANDLE, 0, 0);
#endif
    }

    /*
     * Dispatch the message, if we found one.  If we are exiting, be
     * sure to inform Tcl so we can clean up properly.
     */

    if (foundEvent) {
#ifdef __WIN32__
	if (msg.message == WM_QUIT) {
	    Tcl_Exit(0);
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
#else
	if (msg.msg == WM_QUIT) {
	    Tcl_Exit(0);
	}
	WinDispatchMsg(hab, &msg);
#endif
    }
    return 1;
}

/* Here we put some stubs for functions called from TKGEN. */

#undef XFlush
#undef XFree
#undef XGrabServer
#undef XNoOp
#undef XUngrabServer
#undef XSynchronize
#undef XSync
#undef XVisualIDFromVisual

#ifndef SvREFCNT

void XFlush(Display *display) {}
void XFree(void *data) {if ((data) != NULL) ckfree((char *) (data));}
void XGrabServer(Display *display) {}
void XNoOp(Display *display) {display->request++;}
void XUngrabServer(Display *display) {}
int (*XSynchronize(Display *display, int i))() {display->request++; return NULL;}
void XSync(Display *display, int i) {display->request++;}
VisualID XVisualIDFromVisual(Visual *visual) {return visual->visualid;}

#endif

#ifdef __WIN32__

#undef ReleaseDC
#undef GetDC

#ifdef 0 /* Paints black on black? */

static HWND hwndDesktop;

int        
MyReleaseDC( HWND h, HDC dc)
{ 
  if (h) return ReleaseDC(h,dc);
  return ReleaseDC(hwndDesktop,dc);
}


HDC        
MyGetDC( HWND h)
{ 
  static HDC dc;

  if (h) return GetDC(h);
  if (!hwndDesktop) hwndDesktop = GetDesktopWindow();
  return GetDC(hwndDesktop);
}

#else /* ! 0 */

int        
MyReleaseDC( HWND h, HDC dc)
{ 
  if (h) return ReleaseDC(h,dc);
  return 0;
}


HDC        
MyGetDC( HWND h)
{ 
  static HDC dc;

  if (h) return GetDC(h);
  if (!dc) dc = GetDC(h);
  return dc;
}


#endif /* !0 */

/* Cursor stuff */

#include <rc/cursors.h>

HCURSOR
os2LoadCursor(module, name)
    HMODULE module;
    char*   name;
{
    myCursor *curPtr = cursors;
    while (curPtr->name) {
	if (strcmp(curPtr->name, name) == 0) {
	    break;
	}
	curPtr++;
    }
    if (curPtr->name) {
	return LoadCursor(module, curPtr->id);
    } else {
	return NULL;
    }
}

HICON
os2LoadIcon(module, name)
    HMODULE module;
    char*   name;
{
    myCursor *curPtr = cursors;
    while (curPtr->name) {
	if (strcmp(curPtr->name, name) == 0) {
	    break;
	}
	curPtr++;
    }
    if (curPtr->name) {
	return LoadIcon(module, curPtr->id);
    } else {
	return NULL;
    }
}

#else /* !__WIN32__ */

/*
 *----------------------------------------------------------------------
 *
 * TclOS2FlushEvents --
 *
 *	This function is a special purpose hack to allow Tk to
 *	process queued Window events during a recursive event loop
 *	without looking for new events on the system event queue.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Services any pending Tcl events and calls idle handlers.
 *
 *----------------------------------------------------------------------
 */

void
TclOS2FlushEvents()
{
    ignoreEvents = 1;
    while (Tcl_DoOneEvent(TCL_DONT_WAIT|TCL_WINDOW_EVENTS|TCL_IDLE_EVENTS)) {
    }
    ignoreEvents = 0;
}


HBITMAP
CreateBitmap(int width, int height, int unknown, int depth, void* unknown1)
{
	HPS memPS;
	BITMAPINFOHEADER2 bmpInfo;
	HBITMAP bitmap;

#ifdef DEBUG
printf("XCopyPlane case3\n");
#endif

	if (unknown != 1 || unknown1 != NULL)
	    panic("Unknown parameters to OS2CreateBitmap");

	memPS = WinGetScreenPS(HWND_DESKTOP);
	bmpInfo.cbFix = sizeof(BITMAPINFOHEADER2);
	bmpInfo.cx = width;
	bmpInfo.cy = height;
	bmpInfo.cPlanes = 1;
	bmpInfo.cBitCount = depth;
	bitmap = GpiCreateBitmap(memPS, &bmpInfo, 0L, NULL, NULL);
#ifdef DEBUG
printf("    GpiCreateBitmap (%d,%d) returned %x\n", width, height, bitmap);
#endif
/*
*/
	return bitmap;
}

/* A copy from tixpWinXpm.c */
#include "tix.h"
#include "tixImgXpm.h"

typedef struct PixmapData {
    HDC bitmapDC;               /* Bitmap used on Windows platforms */
    HDC maskDC;                 /* Mask used on Windows platforms */
    HBITMAP bitmap, bitmapOld;
    HBITMAP maskBm, maskBmOld;
} PixmapData;

static void		CopyTransparent _ANSI_ARGS_((Display* display,
			    HDC srcDC, Drawable dest,
			    int src_x, int src_y, int width,
			    int height, int dest_x, int dest_y,
			    HDC maskDC));


/*----------------------------------------------------------------------
 * TixpXpmRealizePixmap --
 *
 *	On Unix: 	Create the pixmap from the buffer.
 *	On Windows:	Free the mask if there are no transparent pixels.
 *----------------------------------------------------------------------
 */
void
TixpXpmRealizePixmap(masterPtr, instancePtr, image, mask, isTransp)
    PixmapMaster * masterPtr;
    PixmapInstance * instancePtr;
    XImage * image;
    XImage * mask;
{
    Display *display = Tk_Display(instancePtr->tkwin);
    PixmapData *dataPtr = (PixmapData*)instancePtr->clientData;
    HPS hps, bitmapPS;
    HDC dc, dc1;
    TkOS2PSState psState;
    HBITMAP bitmap, bitmapOld;
    int w, h;
    BITMAPINFOHEADER2_2colors bmpInfo;
    PBITMAPINFOHEADER2 pbmpInfo = &(bmpInfo.header);
    SIZEL sizl = {0,0}; /* use same page size as device */
    POINTL aPoints[3]; /* Lower-left, upper-right, lower-left source */
    DEVOPENSTRUC dop = {0L, (PSZ)"DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};

    w = masterPtr->size[0];
    h = masterPtr->size[1];

    hps = TkOS2GetDrawablePS(display, instancePtr->pixmap, &psState);
    dc = DevOpenDC(hab, OD_MEMORY, (PSZ)"*", 5L, (PDEVOPENDATA)&dop,
		   ((TkOS2Drawable *)instancePtr->pixmap)->bitmap.dc);
    if (dc == DEV_ERROR) {
	printf("DevOpenDC ERROR\n");
    } else {
	printf("DevOpenDC(?) = %x\n", dc);	    
    }
    bitmapPS = GpiCreatePS(hab, dc, &sizl, PU_PELS | GPIT_NORMAL | GPIA_ASSOC);
    if (bitmapPS == GPI_ERROR) {
	printf("GpiCreatePS(%x, %x, {0,0}, ?) GPI_ERROR: %x\n",
	       hab, dc, WinGetLastError(hab));
    } else {
	printf("GpiCreatePS(?) = %x\n", bitmapPS);	    
    }

    pbmpInfo->cbFix = 16L;
    pbmpInfo->cx = w;
    pbmpInfo->cy = h;
    pbmpInfo->cPlanes = 1;
    pbmpInfo->cBitCount = 1;
    bitmap = GpiCreateBitmap(bitmapPS, pbmpInfo, 0L, NULL, NULL);
    if (bitmap == GPI_ERROR) {
	printf("GpiCreateBitmap(%x, %x, {?,?}, 0,0,0) GPI_ERROR: %x\n",
	       hab, bitmapPS, WinGetLastError(hab));
    }
    bitmapOld = GpiSetBitmap(bitmapPS, bitmap);
    
    aPoints[0].x = 0; /* dest_ll = 0 */
    aPoints[0].y = 0;
    aPoints[1].x = w; /* other corner: ur */
    aPoints[1].y = h;
    aPoints[2].x = 0; /* source ll */
    aPoints[2].y = 0;
    GpiBitBlt(bitmapPS, hps, 3, aPoints, ROP_SRCCOPY, BBO_IGNORE);

    if (isTransp) {
	HPS maskPS;
	HBITMAP maskBm, maskBmOld;
	BITMAPINFOHEADER2 bmpInfo2;	/* Description of mask->data. */

	/*
	 * There are transparent pixels. We need a mask.
	 */
	dc1 = DevOpenDC(hab, OD_MEMORY, (PSZ)"*", 5L, (PDEVOPENDATA)&dop,
		       ((TkOS2Drawable *)instancePtr->pixmap)->bitmap.dc);
	if (dc1 == DEV_ERROR) {
	    printf("DevOpenDC ERROR\n");
	    return;
	} else {
	    printf("DevOpenDC(?) = %x\n", dc1);	    
	}
	maskPS = GpiCreatePS(hab, dc1, &sizl, PU_PELS | GPIT_NORMAL | GPIA_ASSOC);
	if (maskPS == GPI_ERROR) {
	    printf("GpiCreatePS(%x, %x, {0,0}, ?) GPI_ERROR: %x\n",
		   hab, dc1, WinGetLastError(hab));
	} else {
	    printf("GpiCreatePS(?) = %x\n", maskPS);	    
	}
	bmpInfo.colors[0].bBlue = bmpInfo.colors[0].bGreen =
	    bmpInfo.colors[0].bRed = 0;
	bmpInfo.colors[1].bBlue = bmpInfo.colors[1].bGreen =
	    bmpInfo.colors[1].bRed = 255;
	
	maskBm = GpiCreateBitmap(bitmapPS, pbmpInfo, CBM_INIT, mask->data, (PBITMAPINFO2)pbmpInfo);
	if (maskBm == GPI_ERROR) {
	    printf("GpiCreateBitmap(%x, %x, {?,?}, CBM_INIT,?,?) GPI_ERROR: %x\n",
		   hab, bitmapPS, WinGetLastError(hab));
	}
	maskBmOld = GpiSetBitmap(maskPS, maskBm);

	GpiBitBlt(bitmapPS, maskPS, 3, aPoints, ROP_SRCCOPY, BBO_IGNORE);
	GpiBitBlt(maskPS, maskPS, 3, aPoints, ROP_NOTSRCCOPY, BBO_IGNORE);

	TkOS2ReleaseDrawablePS(instancePtr->pixmap, hps, &psState);
	dataPtr->maskDC = maskPS;
	dataPtr->maskBm = maskBm;
	dataPtr->maskBmOld = maskBmOld;
    } else {
	dataPtr->maskDC = NULL;
    }
    dataPtr->bitmapDC = bitmapPS;
    dataPtr->bitmap = bitmap;
    dataPtr->bitmapOld = bitmapOld;
}

void
TixpXpmFreeInstanceData(instancePtr, delete)
    PixmapInstance *instancePtr;	/* Pixmap instance. */
    int delete;				/* Should the instance data structure
					 * be deleted as well? */
{
    PixmapData *dataPtr = (PixmapData*)instancePtr->clientData;    
    HDC dc, dc1;

    if (dataPtr->maskDC != NULL) {
	GpiDeleteBitmap(GpiSetBitmap(dataPtr->maskDC,
				     dataPtr->maskBmOld));
	dc = GpiQueryDevice(dataPtr->maskDC);
	if (dc == HDC_ERROR) {
	    printf("GpiQueryDevice(%x) ERROR = %x\n", dataPtr->maskDC,
		   WinGetLastError(hab));
	} else {
	    printf("GpiQueryDevice(%x) = %x\n", dataPtr->maskDC, dc);	    
	}
	if (!GpiSetBitmap(dataPtr->maskDC, NULL)) {
	    printf("GpiSetBitmap(%x,0) ERROR = %x\n", dataPtr->maskDC, WinGetLastError(hab));	    
	}
	WinReleasePS(dataPtr->maskDC);
	if (!WinReleasePS(dataPtr->maskDC)) {
	    printf("WinReleasePS(%x) ERROR = %x\n", dataPtr->maskDC, WinGetLastError(hab));
	}
	dc1 = DevCloseDC(dc);
	if (dc1 == DEV_ERROR) {
	    printf("DevCloseDC(%x) ERROR = %x\n", dc, WinGetLastError(hab));
	}
	dataPtr->maskDC = NULL;
    }
    if (dataPtr->bitmapDC != NULL) {
	GpiDeleteBitmap(GpiSetBitmap(dataPtr->bitmapDC,
				     dataPtr->bitmapOld));
	dc = GpiQueryDevice(dataPtr->bitmapDC);
	if (dc == HDC_ERROR) {
	    printf("GpiQueryDevice(%x) ERROR = %x\n", dataPtr->bitmapDC, WinGetLastError(hab));
	} else {
	    printf("GpiQueryDevice(%x) = %x\n", dataPtr->bitmapDC, dc);	    
	}
	if (!GpiSetBitmap(dataPtr->bitmapDC, NULL)) {
	    printf("GpiSetBitmap(%x,0) ERROR = %x\n", dataPtr->bitmapDC, WinGetLastError(hab));	    
	}
	if (!WinReleasePS(dataPtr->bitmapDC)) {
	    printf("WinReleasePS(%x) ERROR = %x\n", dataPtr->bitmapDC, WinGetLastError(hab));
	}
	dc1 = DevCloseDC(dc);
	if (dc1 == DEV_ERROR) {
	    printf("DevCloseDC(%x) ERROR = %x\n", dc, WinGetLastError(hab));
	}
	dataPtr->bitmapDC = NULL;
    }
    if (delete) {
	ckfree((char*)dataPtr);
	instancePtr->clientData = NULL;
    }
}

void
TixpXpmDisplay(clientData, display, drawable, imageX, imageY, width,
	height, drawableX, drawableY)
    ClientData clientData;	/* Pointer to PixmapInstance structure for
				 * for instance to be displayed. */
    Display *display;		/* Display on which to draw image. */
    Drawable drawable;		/* Pixmap or window in which to draw image. */
    int imageX, imageY;		/* Upper-left corner of region within image
				 * to draw. */
    int width, height;		/* Dimensions of region within image to draw.*/
    int drawableX, drawableY;	/* Coordinates within drawable that
				 * correspond to imageX and imageY. */
{
    PixmapInstance *instancePtr = (PixmapInstance *) clientData;
    PixmapData *dataPtr = (PixmapData*)instancePtr->clientData;

    CopyTransparent(display, dataPtr->bitmapDC, drawable,
	imageX, imageY, width, height,
	drawableX, drawableY, dataPtr->maskDC);
}

static void
CopyTransparent(display, srcDC, dest, src_x, src_y, width, height, dest_x,
        dest_y,	maskDC)
    Display* display;
    HPS srcDC;
    Drawable dest;
    int src_x;
    int src_y;
    int width;
    int height;
    int dest_x;
    int dest_y;
    HPS maskDC;
{
    HPS destPS;
    TkOS2PSState destState;
    POINTL aPoints[4]; /* Lower-left, upper-right, lower-left source */

    destPS = TkOS2GetDrawablePS(display, dest, &destState);
    /* Make this upside-down: */
    aPoints[0].x = 0;			/* dest_ll = 0 */
    aPoints[0].y = height - 1;
    aPoints[1].x = width;		/* other corner: ur */
    aPoints[1].y = -1;
    aPoints[2].x = 0;			/* source ll */
    aPoints[2].y = 0;
    aPoints[3].x = width;		/* source ll */
    aPoints[3].y = height;
    if (maskDC) {
	GpiBitBlt(destPS, maskDC, 4, aPoints, ROP_SRCAND, BBO_IGNORE);
	GpiBitBlt(destPS, srcDC, 4, aPoints, ROP_SRCPAINT, BBO_IGNORE);
    } else {
	GpiBitBlt(destPS, srcDC, 4, aPoints, ROP_SRCCOPY, BBO_IGNORE);
    }
    TkOS2ReleaseDrawablePS(dest, destPS, &destState);
}


#endif /* ! __Win32__ */
