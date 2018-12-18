#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include "wallpaper.h"
#include "log.h"
#include "core.h"

#define IMLIB_CACHE_SIZE 32 * 1024 * 1024

#define ERROR_TODO -1
#define TODO_SUCCESS 0

struct x11_context_t
{
    Display *display;
    Visual *visual;
    Screen *screen;
    Colormap cm;
    int depth;
    XContext xid_context;
    Window root;
};

void
wpd_draw_image_on_pixmap(Drawable d, Imlib_Image im, int x, int y,
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

wpd_error
wpd_create_x11_context(struct x11_context_t** result)
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

wpd_error
wpd_destroy_x11_context(struct x11_context_t** result)
{
    if (!result || !(*result))
    {
        return WPD_ERROR_NULL_PARAM;
    }

    free(*result);
    return WPD_ERROR_SUCCESS;
}

void
wpd_initialize_libs(const struct x11_context_t* x11)
{
	imlib_context_set_display(x11->display);
	imlib_context_set_visual(x11->visual);
	imlib_context_set_colormap(x11->cm);
	imlib_context_set_color_modifier(NULL);
	imlib_context_set_progress_function(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);

	imlib_set_cache_size(IMLIB_CACHE_SIZE);
}

Pixmap wpd_create_pixmap_from_image(Imlib_Image* image, const struct x11_context_t* x11)
{
    int w, h;
    Pixmap pixmap;

    // Get image dimensions
    imlib_context_set_image(*image);
    w = imlib_image_get_width();
    h = imlib_image_get_height();

    // Create a pixmap with the same dimensions as the image
    return XCreatePixmap(x11->display, x11->root, w, h, x11->depth);
}

wpd_error
wpd_get_pixmap_property(const struct x11_context_t* x11,
    const char * prop_name, unsigned char** result)
{
	Atom prop, type;
	int format;
	unsigned long length, after;
    unsigned char* data;
    
	prop = XInternAtom(x11->display, prop_name, True);
    if (prop == None)
    {
        LOGERROR("Couldn't find the %s atom", prop_name);
        return ERROR_TODO;
    }

    XGetWindowProperty(x11->display, x11->root, prop, 0L, 1L, False,
        AnyPropertyType, &type, &format, &length, &after, &data);

    if (type != XA_PIXMAP)
    {
        LOGERROR("%s property is not of type pixmap", prop_name);
        return ERROR_TODO;
    }

    (*result) = data;
    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_set_pixmap_property(const struct x11_context_t* x11,
    const char * prop_name, const Pixmap pixmap)
{
    Atom prop;

    prop = XInternAtom(x11->display, prop_name, False);

    if (prop == None)
    {
        LOGERROR("creation of pixmap property failed.");
        return ERROR_TODO;
    }

    XChangeProperty(x11->display, x11->root, prop, XA_PIXMAP, 32,
        PropModeReplace, (unsigned char *) &pixmap, 1);

    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_update_pixmap_properties(const struct x11_context_t* x11,
    const Pixmap background_pixmap)
{
	unsigned char *data_root = NULL, *data_esetroot = NULL;

    TRY(wpd_get_pixmap_property(x11, "_XROOTPMAP_ID", &data_root));
    TRY(wpd_get_pixmap_property(x11, "ESETROOT_PMAP_ID", &data_esetroot));

    // If both properties are the same pixmap
    if (*((Pixmap *) data_root) == *((Pixmap *) data_esetroot))
    {
        // TODO: Understand why we do this
        XKillClient(x11->display, *((Pixmap *) data_root));
    }

	if (data_root)
		XFree(data_root);
	
	if (data_esetroot)
		XFree(data_esetroot);

    TRY(wpd_set_pixmap_property(x11, "_XROOTPMAP_ID", background_pixmap));
    TRY(wpd_set_pixmap_property(x11, "ESETROOT_PMAP_ID", background_pixmap));

    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_set_image_as_wallpaper(const struct x11_context_t* x11, Imlib_Image image)
{
	Pixmap background_pixmap;
	XGCValues gcvalues;
	GC gc;
    struct x11_context_t* x11_temp = NULL;
    Pixmap image_pixmap;

    XColor color;
    Colormap cmap = DefaultColormap(x11->display, DefaultScreen(x11->display));
    XAllocNamedColor(x11->display, cmap, "black", &color, &color);

    image_pixmap = wpd_create_pixmap_from_image(&image, x11);
	wpd_draw_image_on_pixmap(image_pixmap, image, 0, 0, 1, 1, 0);

    // Create a separate temporary context that we can dispose of when we are
    // done
    TRY(wpd_create_x11_context(&x11_temp));

    XSync(x11->display, False);
    background_pixmap = XCreatePixmap(x11_temp->display, x11_temp->root,
            x11->screen->width, x11->screen->height, x11_temp->depth);

    // TODO: Understand this; Create the graphics context used to copy the pixmap?
    gcvalues.fill_style = FillTiled;
    gcvalues.tile = image_pixmap;
    gc = XCreateGC(x11_temp->display, background_pixmap, GCFillStyle | GCTile,
        &gcvalues);

    // Copy the pixmap
    XFillRectangle(x11_temp->display, background_pixmap, gc, 0, 0,
        x11->screen->width, x11->screen->height);

    // Free the graphics context
    XFreeGC(x11_temp->display, gc);

    // Sync the displays
    XSync(x11_temp->display, False);
    XSync(x11->display, False);

    // Update the root pixmap properties so Window Managers respect the updates
    TRY(wpd_update_pixmap_properties(x11_temp, background_pixmap));

    // Set the background pixmap of the root window
    XSetWindowBackgroundPixmap(x11_temp->display, x11_temp->root, background_pixmap);
    XClearWindow(x11_temp->display, x11_temp->root);
    XFlush(x11_temp->display);

    // Close the temporary display and retain the changes
    XSetCloseDownMode(x11_temp->display, RetainPermanent);
    XCloseDisplay(x11_temp->display);
}

wpd_error
wpd_load_image(const char * path, Imlib_Image* result)
{
    Imlib_Image image;

    image = imlib_load_image(path);
    if (!image)
    {
        LOGERROR("TODO");
        return ERROR_TODO;
    }

    (*result) = image;
    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_set_wallpaper(const struct x11_context_t* x11, const char * path)
{
    Imlib_Image image;

    TRY(wpd_load_image(path, &image));

    wpd_set_image_as_wallpaper(x11, image);
}

