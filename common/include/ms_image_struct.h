#ifndef MS_IMAGE_STRUCT_H_
#define MS_IMAGE_STRUCT_H_

//-----------------------------------------------------------------------------
typedef struct ms_image
{
    double* image;

    unsigned int img_w;
    unsigned int img_h;

    bool use_img = true;
    bool invert_img = false;

    double weight = 0.5;

    double scale = 1.0;
    bool scale_img = true;

    double threshold = 75.0;

} ms_image;

//-----------------------------------------------------------------------------
inline ms_image init_ms_image(double* image, unsigned int img_w, unsigned int img_h, bool use_img, bool invert_img, double weight, double scale, bool scale_img, double threshold)
{
    ms_image tmp;
    tmp.image = image;
    tmp.img_w = img_w;
    tmp.img_h = img_h;
    tmp.use_img = use_img;
    tmp.invert_img = invert_img;
    tmp.weight = weight;
    tmp.scale = scale;
    tmp.scale_img = scale_img;
    tmp.threshold = threshold;
    return tmp;
}


#endif	// MS_IMAGE_STRUCT_H_
