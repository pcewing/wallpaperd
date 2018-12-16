#include "image.h"

#include "log.h"

#include <wand/MagickWand.h>

int
create_image(size_t width, size_t height, struct wpd_image_t** result)
{
    (*result) = malloc(sizeof(struct wpd_image_t));
    (*result)->width = width;
    (*result)->height = height;

    (*result)->pixels = malloc((*result)->width * sizeof(struct wpd_pixel_t*));

    for (size_t x = 0; x < (*result)->width; x++)
    {
        (*result)->pixels[x] = malloc((*result)->height * sizeof(struct wpd_pixel_t));
    }

    return 0;
}

int
load_image(const char * path, struct wpd_image_t** result)
{
    LOGINFO("Loading image at path %s", path);
  
    MagickBooleanType status;
    MagickWand *magick_wand;
    PixelIterator *iterator;
    PixelWand **pixels;
    MagickPixelPacket pixel;
    register ssize_t x;
    ssize_t y;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, path);
    if (status == MagickFalse)
    {
        LOGERROR("Failed to read the image file");
        return -1;
    }

    iterator = NewPixelIterator(magick_wand);
    if (iterator == (PixelIterator *) NULL)
    {
        LOGERROR("Failed to create a pixel iterator");
        return -1;
    }

    ssize_t image_height;
    size_t image_width, row_width;

    image_height = (ssize_t) MagickGetImageHeight(magick_wand);
    pixels = PixelGetNextIteratorRow(iterator, &image_width);

    struct wpd_image_t* image;
    int create_image_result = create_image(image_width, image_height, &image);
    if (create_image_result != 0)
    {
        LOGERROR("TODO");
        return -1;
    }

    for (y = 1; y < image_height; y++)
    {
        pixels = PixelGetNextIteratorRow(iterator, &row_width);

        if (pixels == (PixelWand **) NULL)
        {
            LOGERROR("TODO");
            return -1;
        }

        if (row_width != image_width)
        {
            LOGERROR("Varied image row widths unsupported");
            return -1;
        }

        for (x = 0; x < (ssize_t) row_width; x++)
        {
            PixelGetMagickColor(pixels[x], &pixel);
            image->pixels[x][y].m_red = PixelGetRed(pixels[x]);
            image->pixels[x][y].m_green = PixelGetGreen(pixels[x]);
            image->pixels[x][y].m_blue = PixelGetBlue(pixels[x]);

            //LOGINFO("(%f,%f,%f) -> (%f,%f,%f) \n",
                //image->pixels[x][y].m_red, image->pixels[x][y].m_green, image->pixels[x][y].m_blue,
                //255 * image->pixels[x][y].m_red, 255 * image->pixels[x][y].m_green, 255 * image->pixels[x][y].m_blue);
        }
    }

    iterator = DestroyPixelIterator(iterator);
    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    (*result) = image;
    return 0;
}

int
free_image(struct wpd_image_t** image)
{
    for (size_t x = 0; x < (*image)->width; x++)
    {
        free((*image)->pixels[x]);
    }

    free((*image)->pixels);
    free(*image);

    return 0;
}

