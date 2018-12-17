#include "wallpaper.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <string.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

#define ERROR_TODO -1

// GLOBALS

Display *disp = NULL;
Visual *vis = NULL;
Screen *scr = NULL;
Colormap cm;
int depth;
Atom wmDeleteWindow;
XContext xid_context = 0;
Window root = 0;

struct options
{
    int cache_size;
	char *image_bg;
};

struct options opt = { 4, NULL };

void init_x_and_imlib(void)
{
	disp = XOpenDisplay(NULL);
	if (!disp)
		LOGERROR("Can't open X display. It *is* running, yeah?");
	vis = DefaultVisual(disp, DefaultScreen(disp));
	depth = DefaultDepth(disp, DefaultScreen(disp));
	cm = DefaultColormap(disp, DefaultScreen(disp));
	root = RootWindow(disp, DefaultScreen(disp));
	scr = ScreenOfDisplay(disp, DefaultScreen(disp));
	xid_context = XUniqueContext();

#ifdef HAVE_LIBXINERAMA
	init_xinerama();
#endif				/* HAVE_LIBXINERAMA */

	imlib_context_set_display(disp);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);
	imlib_context_set_color_modifier(NULL);
	imlib_context_set_progress_function(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);
	wmDeleteWindow = XInternAtom(disp, "WM_DELETE_WINDOW", False);

	imlib_set_cache_size(opt.cache_size * 1024 * 1024);

	return;
}

void
gib_imlib_render_image_on_drawable(Drawable d, Imlib_Image im, int x, int y,
                                   char dither, char blend, char alias)
{
   imlib_context_set_image(im);
   imlib_context_set_drawable(d);
   imlib_context_set_anti_alias(alias);
   imlib_context_set_dither(dither);
   imlib_context_set_blend(blend);
   imlib_context_set_angle(0);
   imlib_render_image_on_drawable(x, y);
}


int
gib_imlib_image_get_width(Imlib_Image im)
{
   imlib_context_set_image(im);
   return imlib_image_get_width();
}

int
gib_imlib_image_get_height(Imlib_Image im)
{
   imlib_context_set_image(im);
   return imlib_image_get_height();
}


void feh_wm_set_bg(Imlib_Image im)	
{
	XGCValues gcvalues;
	XGCValues gcval;
	GC gc;
	char bgname[20];
	int num = (int) random();
	char bgfil[4096];
	char sendbuf[4096];

    Atom prop_root, prop_esetroot, type;
    int format, i;
    unsigned long length, after;
    unsigned char *data_root = NULL, *data_esetroot = NULL;
    Pixmap pmap_d1, pmap_d2;

    char *home;

    /* local display to set closedownmode on */
    Display *disp2;
    Window root2;
    int depth2;
    int w, h;

    LOGDEBUG("Falling back to XSetRootWindowPixmap\n");

    XColor color;
    Colormap cmap = DefaultColormap(disp, DefaultScreen(disp));
    if (opt.image_bg)
        XAllocNamedColor(disp, cmap, (char*) opt.image_bg, &color, &color);
    else
        XAllocNamedColor(disp, cmap, "black", &color, &color);

    w = gib_imlib_image_get_width(im);
    h = gib_imlib_image_get_height(im);
    pmap_d1 = XCreatePixmap(disp, root, w, h, depth);
    gib_imlib_render_image_on_drawable(pmap_d1, im, 0, 0, 1, 1, 0);

    /* create new display, copy pixmap to new display */
    disp2 = XOpenDisplay(NULL);
    if (!disp2)
        LOGERROR("Can't reopen X display.");
    root2 = RootWindow(disp2, DefaultScreen(disp2));
    depth2 = DefaultDepth(disp2, DefaultScreen(disp2));
    XSync(disp, False);
    pmap_d2 = XCreatePixmap(disp2, root2, scr->width, scr->height, depth2);
    gcvalues.fill_style = FillTiled;
    gcvalues.tile = pmap_d1;
    gc = XCreateGC(disp2, pmap_d2, GCFillStyle | GCTile, &gcvalues);
    XFillRectangle(disp2, pmap_d2, gc, 0, 0, scr->width, scr->height);
    XFreeGC(disp2, gc);
    XSync(disp2, False);
    XSync(disp, False);
    XFreePixmap(disp, pmap_d1);

    prop_root = XInternAtom(disp2, "_XROOTPMAP_ID", True);
    prop_esetroot = XInternAtom(disp2, "ESETROOT_PMAP_ID", True);

    if (prop_root != None && prop_esetroot != None) {
        XGetWindowProperty(disp2, root2, prop_root, 0L, 1L,
                   False, AnyPropertyType, &type, &format, &length, &after, &data_root);
        if (type == XA_PIXMAP) {
            XGetWindowProperty(disp2, root2,
                       prop_esetroot, 0L, 1L,
                       False, AnyPropertyType,
                       &type, &format, &length, &after, &data_esetroot);
            if (data_root && data_esetroot) {
                if (type == XA_PIXMAP && *((Pixmap *) data_root) == *((Pixmap *) data_esetroot)) {
                    XKillClient(disp2, *((Pixmap *)
                                 data_root));
                }
            }
        }
    }

    if (data_root)
        XFree(data_root);

    if (data_esetroot)
        XFree(data_esetroot);

    /* This will locate the property, creating it if it doesn't exist */
    prop_root = XInternAtom(disp2, "_XROOTPMAP_ID", False);
    prop_esetroot = XInternAtom(disp2, "ESETROOT_PMAP_ID", False);

    if (prop_root == None || prop_esetroot == None)
        LOGERROR("creation of pixmap property failed.");

    XChangeProperty(disp2, root2, prop_root, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pmap_d2, 1);
    XChangeProperty(disp2, root2, prop_esetroot, XA_PIXMAP, 32,
            PropModeReplace, (unsigned char *) &pmap_d2, 1);

    XSetWindowBackgroundPixmap(disp2, root2, pmap_d2);
    XClearWindow(disp2, root2);
    XFlush(disp2);
    XSetCloseDownMode(disp2, RetainPermanent);
    XCloseDisplay(disp2);
	return;
}

int
set_bg(const char * path)
{
    Pixmap image_pixmap;
    Imlib_Image image;
    static struct x11_context_t* x11 = NULL;
    int error;

    /* load the image */
    image = imlib_load_image(path);

    if (!image)
    {
        LOGERROR("TODO");
        return ERROR_TODO;
    }

    feh_wm_set_bg(image);
    return 0;
}
