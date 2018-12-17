#include "wallpaper.h"
#include "log.h"

#include <stdlib.h>
#include <X11/Xlib.h>

#include <string.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

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

struct x11_context_t {
    Display *display;
    Visual *visual;
    Screen *screen;
    Colormap cm;
    int depth;
    XContext xid_context;
    Window root;
};

int
create_x11_context(struct x11_context_t** result)
{
    struct x11_context_t* x11;

    x11 = malloc(sizeof(struct x11_context_t));

	x11->display = XOpenDisplay(NULL);
	if (!x11->display)
    {
        LOGERROR("Failed to open X11 display");
        free(x11);
        return -1;
    }

	x11->visual = DefaultVisual(x11->display, DefaultScreen(x11->display));
	x11->depth = DefaultDepth(x11->display, DefaultScreen(x11->display));
	x11->cm = DefaultColormap(x11->display, DefaultScreen(x11->display));
	x11->root = RootWindow(x11->display, DefaultScreen(x11->display));
	x11->screen = ScreenOfDisplay(x11->display, DefaultScreen(x11->display));
	x11->xid_context = XUniqueContext();

    (*result) = x11;
}

void
destroy_x11_context(struct x11_context_t** result)
{
    free(result);
}

void
initialize_imlib(struct x11_context_t* x11)
{
	imlib_context_set_display(x11->display);
	imlib_context_set_visual(x11->visual);
	imlib_context_set_colormap(x11->cm);
	imlib_context_set_color_modifier(NULL);
	imlib_context_set_progress_function(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);

	imlib_set_cache_size(32 * 1024 * 1024);
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

void
wpd_set_wallpaper(const char * path)
{
    struct x11_context_t* x11 = NULL;
    create_x11_context(&x11);
    initialize_imlib(x11);
    int w, h;

    Pixmap pmap_d1;

    /* an image handle */
    Imlib_Image image;
  
    /* load the image */
    image = imlib_load_image(path);
    /* if the load was successful */
    if (image)
    {
        char *tmp;
        /* set the image we loaded as the current context image to work on */
        imlib_context_set_image(image);

        /* set the image format to be the format of the extension of our last */
        /* argument - i.e. .png = png, .tif = tiff etc. */
        tmp = strrchr(path, '.');

        if(tmp)
           imlib_image_set_format(tmp + 1);

        w = gib_imlib_image_get_width(image);
        h = gib_imlib_image_get_height(image);
        pmap_d1 = XCreatePixmap(x11->display, x11->root, w, h, x11->depth);

		gib_imlib_render_image_on_drawable(pmap_d1, image, 0, 0, 1, 1, 0);
    }

	Pixmap pmap_d2;
	XGCValues gcvalues;
	XGCValues gcval;
	GC gc;
	char bgname[20];
	int num = (int) random();
	char bgfil[4096];
	char sendbuf[4096];
	Display *disp2;
	Window root2;
	int depth2;

	Atom prop_root, prop_esetroot, type;
	int format, i;
	unsigned long length, after;
	unsigned char *data_root = NULL, *data_esetroot = NULL;

	/* create new display, copy pixmap to new display */
	disp2 = XOpenDisplay(NULL);
	//if (!disp2)
		//eprintf("Can't reopen X display.");
	root2 = RootWindow(disp2, DefaultScreen(disp2));
	depth2 = DefaultDepth(disp2, DefaultScreen(disp2));
	XSync(x11->display, False);
	pmap_d2 = XCreatePixmap(disp2, root2, x11->screen->width, x11->screen->height, depth2);
	gcvalues.fill_style = FillTiled;
	gcvalues.tile = pmap_d1;
	gc = XCreateGC(disp2, pmap_d2, GCFillStyle | GCTile, &gcvalues);
	XFillRectangle(disp2, pmap_d2, gc, 0, 0, x11->screen->width, x11->screen->height);
	XFreeGC(disp2, gc);
	XSync(disp2, False);
	XSync(x11->display, False);
	XFreePixmap(x11->display, pmap_d1);

	prop_root = XInternAtom(disp2, "_XROOTPMAP_ID", True);
	prop_esetroot = XInternAtom(disp2, "ESETROOT_PMAP_ID", True);

#define XA_PIXMAP 20

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

	//if (prop_root == None || prop_esetroot == None)
		//eprintf("creation of pixmap property failed.");

	XChangeProperty(disp2, root2, prop_root, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pmap_d2, 1);
	XChangeProperty(disp2, root2, prop_esetroot, XA_PIXMAP, 32,
			PropModeReplace, (unsigned char *) &pmap_d2, 1);

	XSetWindowBackgroundPixmap(disp2, root2, pmap_d2);
	XClearWindow(disp2, root2);
	XFlush(disp2);
	XSetCloseDownMode(disp2, RetainPermanent);
	XCloseDisplay(disp2);
}

