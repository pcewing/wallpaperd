#ifndef IMAGE_H
#define IMAGE_H

struct wpd_pixel_t
{
    float m_red, m_green, m_blue;
};

struct wpd_image_t
{
    unsigned long int width, height;
    struct wpd_pixel_t** pixels;
};

int
load_image(const char * path, struct wpd_image_t** result);

int
free_image(struct wpd_image_t** image);

#endif
