/*
* Copyright 2021 Amy Wagoner, NSWC Crane
* 
* TODO: Add licensing statement. 
*/

#ifndef INCLUDED_TARGET_H
#define INCLUDED_TARGET_H

#include <cstdint>

class target_rect
{

public:
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    
    target_rect() : x(0), y(0), w(0), h(0)
    {}
    
    target_rect(int32_t x_, int32_t y_, int32_t w_, int32_t h_) : x(x_), y(y_), w(w_), h(h_)
    {}
    
    // ----------------------------------------------------------------------------
    target_rect intersect(const target_rect&r)
    {
        int32_t x1 = std::max(x, r.x);
        int32_t y1 = std::max(y, r.y);
        int32_t x2 = std::min((x + w), (r.x + r.w));
        int32_t y2 = std::min((y + h), (r.y + r.h));

        return target_rect(x1, y1, x2 - x1, y2 - y1);

    }
    
    // ----------------------------------------------------------------------------
    int64_t area()
    {
        return w * h;
    }
    
    // ----------------------------------------------------------------------------
    bool is_empty()
    { 
        return (h > 0 || w > 0); 
    }
    
    // ----------------------------------------------------------------------------
    double get_iou(target_rect&r)
    {
        
        target_rect r_int = intersect(r);
        double inner = (double)(r_int.area());
        
        double outer = (double)(area()) + (double)(r.area()) - inner;
        
        if(outer > 0.0)
        {
            return inner/outer;
        }
        else
        {
            return 0.0;
        }
        
    }   // end of get_iou

    // ----------------------------------------------------------------------------
    void get_center(int32_t &x_, int32_t &y_)
    {
        x_ = x + (w >> 1);
        y_ = y + (h >> 1);
    }

    // ----------------------------------------------------------------------------
    inline friend std::ostream& operator<< (
        std::ostream& out,
        const target_rect& item
        )
    {
        out << "x=" << item.x;
        out << ", y=" << item.y;
        out << ", h=" << item.h;
        out << ", w=" << item.w << std::endl;
        return out;
    }

private:

};



// ----------------------------------------------------------------------------
class target
{
 
public: 


    //cv::KeyPoint kp;
    std::string id = "";
    
    //bool illuminated;
    //double pixel_size;
    //double angular_size;
    //double x, y; // , z_loc;
    //double kp_size; // size of the blob diameter
    //uint32_t x, y, h, w;
    //uint32_t center_x, center_y;

    //cv::Rect roi;
    target_rect roi;

    // Target confidence is currently determined by the keypoint overlap score
    double confidence = 0.0;
        
    // Intensity for all bands  
    //double vis_intensity;
    //double ir_intensity;
    //double swir_intensity; 

    // ----------------------------------------------------------------------------
    target() = default;

    target(int32_t x_, int32_t y_, int32_t w_, int32_t h_, std::string id_ = "") : id(id_)
    {
        confidence = 0.0;

//        roi = cv::Rect(std::min(std::max(x_, min_x), max_x), std::min(std::max(y_, min_y), max_y), w_, h_);
        roi = target_rect(std::min(std::max(x_, min_x), max_x), std::min(std::max(y_, min_y), max_y), w_, h_);
        //center_x = x_ + w_ >> 1;
        //center_y = y_ + h_ >> 1;
    }

//    target(cv::Rect r) : roi(r)
    target(target_rect r, std::string id_ = "") : roi(r), id(id_)
    {
        confidence = 0.0;
    }

    // target(cv::KeyPoint kp)
    // {
        // confidence = 0.0;
        // id = "";

        // int32_t x = std::min(std::max((int32_t)(kp.pt.x - kp.size), min_x), max_x);
        // int32_t y = std::min(std::max((int32_t)(kp.pt.y - kp.size), min_y), max_y);
        // int32_t w = (int32_t)(2 * kp.size);
        // int32_t h = (int32_t)(2 * kp.size);

        // roi = cv::Rect(x, y, w, h);
    // }

    ~target() {}
        
    // ----------------------------------------------------------------------------
    double get_iou(target t)
    {
        // get the intersection
        //cv::Rect i = roi & t.roi;

        // get the union
        //cv::Rect u = roi | t.roi;

        //return (double)i.area() / (double)u.area();

        return roi.get_iou(t.roi);
        
    }   // end  of get_iou

//    double get_iou(cv::Rect r)
    double get_iou(target_rect r)
    {
        // get the intersection
        //cv::Rect i = roi & r;

        // get the union
        //cv::Rect u = roi | r;

        //return (double)i.area() / (double)u.area();
        
        return roi.get_iou(r);
        
        
    }   // end  of get_iou

    // ----------------------------------------------------------------------------
    void get_center(uint32_t &x, uint32_t &y)
    {
        x = roi.x + (roi.w >> 1);
        y = roi.y + (roi.h >> 1);
    }   // end of get_center

    // ----------------------------------------------------------------------------
    void set_limits(int32_t min_x_, int32_t max_x_, int32_t min_y_, int32_t max_y_)
    {
        min_x = min_x_;
        max_x = max_x_;
        min_y = min_y_;
        max_y = max_y_;
    }   // end of set_limits

    // ----------------------------------------------------------------------------

    // TODO: Add additional features 
    // TODO: Add methods that will read in white/black list and compare target features
        

    inline friend std::ostream& operator<< (
        std::ostream& out,
        const target& item
        )
    {
        out << "id=" << item.id << std::endl;
        out << item.roi;
        out << "confidence=" << item.confidence << std::endl;
        return out;
    }

// ----------------------------------------------------------------------------
private:

    // target bounds
    int32_t min_x = 0;
    int32_t max_x = 640;
    int32_t min_y = 0;
    int32_t max_y = 480;
        
};

#endif
