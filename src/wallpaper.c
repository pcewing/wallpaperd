#include "wallpaper.h"
#include "log.h"

#include <stdlib.h>
#include <X11/Xlib.h>

#include <string.h>
#include <X11/Xutil.h>
#include <Imlib2.h>

#define IMLIB_CACHE_SIZE 32 * 1024 * 1024

#define ERROR_TODO -1
#define TODO_SUCCESS 0

// TODO I shouldn't have to define this?
#define XA_PIXMAP 20

struct x11_context_t {
    Display *display;
    Visual *visual;
    Screen *screen;
    Colormap cm;
    int depth;
    XContext xid_context;
    Window root;
};

void
draw_image_on_pixmap(Drawable d, Imlib_Image im, int x, int y,
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
create_x11_context(struct x11_context_t** result)
{
    struct x11_context_t* x11;

    x11 = malloc(sizeof(struct x11_context_t));

	x11->display = XOpenDisplay(NULL);
	if (!x11->display)
    {
        LOGERROR("Failed to open X11 display");
        free(x11);
        return ERROR_TODO;
    }

	x11->visual = DefaultVisual(x11->display, DefaultScreen(x11->display));
	x11->depth = DefaultDepth(x11->display, DefaultScreen(x11->display));
	x11->cm = DefaultColormap(x11->display, DefaultScreen(x11->display));
	x11->root = RootWindow(x11->display, DefaultScreen(x11->display));
	x11->screen = ScreenOfDisplay(x11->display, DefaultScreen(x11->display));
	x11->xid_context = XUniqueContext();

    (*result) = x11;

    return TODO_SUCCESS;
}

void
destroy_x11_context(struct x11_context_t** result)
{
    free(result);
}

void
initialize_imlib(const struct x11_context_t* x11)
{
	imlib_context_set_display(x11->display);
	imlib_context_set_visual(x11->visual);
	imlib_context_set_colormap(x11->cm);
	imlib_context_set_color_modifier(NULL);
	imlib_context_set_progress_function(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);

	imlib_set_cache_size(IMLIB_CACHE_SIZE);
}

Pixmap create_pixmap_from_image(Imlib_Image* image, const struct x11_context_t* x11)
{
    int w, h;
    Pixmap pixmap;

    // Get image dimensions
    imlib_context_set_image(*image);
    w = imlib_image_get_width();
    h = imlib_image_get_height();

    // Create a pixmap and draw the image onto it
    pixmap = XCreatePixmap(x11->display, x11->root, w, h, x11->depth);
	draw_image_on_pixmap(pixmap, image, 0, 0, 1, 1, 0);

    return pixmap;
}

int
update_props(const struct x11_context_t* x11, const Pixmap pmap_d2)
{
	Atom prop_root, prop_esetroot, type;
	int format;
	unsigned long length, after;
	unsigned char *data_root = NULL, *data_esetroot = NULL;

	prop_root = XInternAtom(x11->display, "_XROOTPMAP_ID", True);
    {
        if (prop_root == None)
        {
            LOGERROR("Couldn't find the _XROOTPMAP_ID atom");
            return ERROR_TODO;
        }

        // Get the _XROOTPMAP_ID property from the root window
        XGetWindowProperty(x11->display, x11->root, prop_root, 0L, 1L,
            False, AnyPropertyType, &type, &format, &length, &after, &data_root);

        // Make sure the property is a pixmap
        if (type != XA_PIXMAP)
        {
            LOGERROR("_XROOTPMAP_ID property is not of type pixmap");
            return ERROR_TODO;
        }
    }

	prop_esetroot = XInternAtom(x11->display, "ESETROOT_PMAP_ID", True);
    {
        if (prop_esetroot == None)
        {
            LOGERROR("Couldn't find the ESETROOT_PMAP_ID atom");
            return ERROR_TODO;
        }

        // Get the ESETROOT_PMAP_ID property from the root window
        XGetWindowProperty(x11->display, x11->root, prop_esetroot, 0L, 1L,
            False, AnyPropertyType, &type, &format, &length, &after,
            &data_esetroot);

        // Make sure the property is a pixmap
        if (type != XA_PIXMAP)
        {
            LOGERROR("ESETROOT_PMAP_ID property is not of type pixmap");
            return ERROR_TODO;
        }
    }

    // If both properties are the same pixmap
    if (*((Pixmap *) data_root) == *((Pixmap *) data_esetroot))
    {
        XKillClient(x11->display, *((Pixmap *) data_root));
    }

	if (data_root)
		XFree(data_root);
	
	if (data_esetroot)
		XFree(data_esetroot);

	/* This will locate the property, creating it if it doesn't exist */
	prop_root = XInternAtom(x11->display, "_XROOTPMAP_ID", False);
	prop_esetroot = XInternAtom(x11->display, "ESETROOT_PMAP_ID", False);

	if (prop_root == None || prop_esetroot == None)
    {
		LOGERROR("creation of pixmap property failed.");
        return ERROR_TODO;
    }

	XChangeProperty(x11->display, x11->root, prop_root, XA_PIXMAP, 32, PropModeReplace, (unsigned char *) &pmap_d2, 1);
	XChangeProperty(x11->display, x11->root, prop_esetroot, XA_PIXMAP, 32,
			PropModeReplace, (unsigned char *) &pmap_d2, 1);

    return 0;
}

int
apply_pixmap(const struct x11_context_t* x11, Imlib_Image image)
{
	Pixmap pmap_d2;
	XGCValues gcvalues;
	GC gc;
    struct x11_context_t* x11_temp = NULL;
    Pixmap image_pixmap;

    int error;

	Atom prop_root, prop_esetroot, type;
	int format;
	unsigned long length, after;
	unsigned char *data_root = NULL, *data_esetroot = NULL;

    x11_temp = malloc(sizeof(struct x11_context_t));
    /* local display to set closedownmode on */
    Display *disp2;
    Window root2;
    int depth2;
    int w, h;

    LOGDEBUG("Falling back to XSetRootWindowPixmap\n");

    XColor color;
    Colormap cmap = DefaultColormap(x11->display, DefaultScreen(x11->display));
    XAllocNamedColor(x11->display, cmap, "black", &color, &color);

    image_pixmap = create_pixmap_from_image(&image, x11);
	draw_image_on_pixmap(image_pixmap, image, 0, 0, 1, 1, 0);

    /* create new display, copy pixmap to new display */
    disp2 = XOpenDisplay(NULL);
    if (!disp2)
        LOGERROR("Can't reopen X display.");
    root2 = RootWindow(disp2, DefaultScreen(disp2));
    depth2 = DefaultDepth(disp2, DefaultScreen(disp2));

    x11_temp->display = disp2;
    x11_temp->root = root2;
    x11_temp->depth = depth2;

    XSync(x11->display, False);
    pmap_d2 = XCreatePixmap(disp2, root2, x11->screen->width, x11->screen->height, depth2);
    gcvalues.fill_style = FillTiled;
    gcvalues.tile = image_pixmap;
    gc = XCreateGC(disp2, pmap_d2, GCFillStyle | GCTile, &gcvalues);
    XFillRectangle(disp2, pmap_d2, gc, 0, 0, x11->screen->width, x11->screen->height);
    XFreeGC(disp2, gc);
    XSync(disp2, False);
    XSync(x11->display, False);

    error = update_props(x11_temp, pmap_d2);
    if (error != 0)
    {
        return error;
    }

    XSetWindowBackgroundPixmap(x11_temp->display, x11_temp->root, pmap_d2);
    XClearWindow(x11_temp->display, x11_temp->root);
    XFlush(x11_temp->display);
    XSetCloseDownMode(x11_temp->display, RetainPermanent);
    XCloseDisplay(x11_temp->display);
}

int
wpd_set_wallpaper(const struct x11_context_t* x11, const char * path)
{
    Pixmap image_pixmap;
    Imlib_Image image;

    // Load the image from a file
    image = imlib_load_image(path);
    if (!image)
    {
        LOGERROR("TODO");
        return ERROR_TODO;
    }

    apply_pixmap(x11, image);
}

