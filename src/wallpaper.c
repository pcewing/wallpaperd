#include "wallpaper.h"
#include "log.h"

#include <stdlib.h>
#include <X11/Xlib.h>

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
