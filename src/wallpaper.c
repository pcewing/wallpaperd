#include "wallpaper.h"
#include "log.h"

#include <stdlib.h>
#include <X11/Xlib.h>

#include <string.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

void 
set_wallpaper(const struct wpd_image_t * image)
{
    LOGINFO("Setting wallpaper...");

    Display* display;
    Window root;
	int depth;
	Screen* screen;
	Visual* visual;
    Pixmap pixmap;
    GC gc;
	XGCValues gcval;

    LOGINFO("Depth: %i\n");

    display = XOpenDisplay(NULL);
	if (!display)
    {
		LOGERROR("Can't open X display");
    }

    root = XRootWindow(display, DefaultScreen(display));
	depth = DefaultDepth(display, DefaultScreen(display));
	screen = ScreenOfDisplay(display, DefaultScreen(display));
	visual = DefaultVisual(display, DefaultScreen(display));

	pixmap = XCreatePixmap(display, root, screen->width, screen->height, depth);

    /*
    XVisualInfo *XGetVisualInfo(
            Display *display,
            long vinfo_mask,
            XVisualInfo *vinfo_template,
            int *nitems_return);
    */

    char* data = malloc(image->width * image->height * 3);
    for (size_t x = 0; x < image->width; x++)
    {
        for (size_t y = 0; y < image->height; y++)
        {
            data[4 * x + (y * image->width) + 0] = (char)(image->pixels[x][y].m_red * 255);
            data[4 * x + (y * image->width) + 1] = (char)(image->pixels[x][y].m_green * 255);
            data[4 * x + (y * image->width) + 2] = (char)(image->pixels[x][y].m_blue * 255);
        }
    }

    // Create the XImage
    XImage *ximage = XCreateImage( display, visual, depth, XYPixmap, 0,
        /* TODO */ data, image->width, image->height, 32, 0);

	//gc = XCreateGC(display, root, GCForeground, &gcval);
    gc = XCreateGC(display, pixmap, 0, &gcval);

    // Put the XImage on the Pixmap
    XPutImage(display, pixmap, gc, ximage, 0, 0, 0, 0, image->width,
        image->height);

    // Draw the pixmap on the window
    XCopyArea(display, pixmap, root, gc, 0, 0, image->width, image->height, 0, 0);

    /*
    XCopyArea(
        Display *display,
        Drawablesrc,
        dest,
        GC gc,
        intsrc_x,
        src_y,
        unsignedintwidth,
        height,
        intdest_x,
        dest_y);
    */

    /*
     * XPutImage(
     *     Display *display,
     *     Drawable d,
     *     GC gc,
     *     XImage *image,
     *     intsrc_x,
     *     src_y,
     *     intdest_x,
     *     dest_y,
     *     unsignedintwidth,
     *     height);
    */

    /*
    XImage *XCreateImage(
            Display *display,
            Visual *visual,
            unsignedint depth,
            int format,
            int offset,
            char *data,
            unsignedint width,
            unsignedint height,
            int bitmap_pad,
            int bytes_per_line);
    */

    XCloseDisplay(display);
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

Display *disp = NULL;
Visual *vis = NULL;
Screen *scr = NULL;
Colormap cm;
int depth;
Atom wmDeleteWindow;
XContext xid_context = 0;
Window root = 0;


void init_x_and_imlib(void)
{
	disp = XOpenDisplay(NULL);
	//if (!disp)
		//eprintf("Can't open X display. It *is* running, yeah?");
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

	imlib_set_cache_size(32 * 1024 * 1024);

	return;
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
  init_x_and_imlib();
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
        pmap_d1 = XCreatePixmap(disp, root, w, h, depth);

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
	return;
}

