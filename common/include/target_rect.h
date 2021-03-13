/*
* Copyright 2021 David Emerson
* 
* TODO: Add licensing statement. 
*/

#ifndef TARGET_RECT_H
#define TARGET_RECT_H

#include <cstdint>

// ----------------------------------------------------------------------------
class target_rect
{

public:
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    std::string id = "";

    double confidence = 0.0;

    target_rect() : x(0), y(0), w(0), h(0)
    {}

    target_rect(int32_t x_, int32_t y_, int32_t w_, int32_t h_) : x(x_), y(y_), w(w_), h(h_)
    {
        id = "";
    }

    target_rect(int32_t x_, int32_t y_, int32_t w_, int32_t h_, std::string id_) : x(x_), y(y_), w(w_), h(h_)
    {
        id = id_;
    }

    ~target_rect() {}

    // ----------------------------------------------------------------------------
    target_rect intersect(const target_rect& r)
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
        return !(h > 0 || w > 0);
    }

    // ----------------------------------------------------------------------------
    double get_iou(target_rect& r)
    {

        target_rect r_int = intersect(r);
        double inner = (double)(r_int.area());

        double outer = (double)(area()) + (double)(r.area()) - inner;

        if (outer > 0.0)
        {
            return inner / outer;
        }
        else
        {
            return 0.0;
        }

    }   // end of get_iou

    // ----------------------------------------------------------------------------
    void get_center(int32_t& x_, int32_t& y_)
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
#endif  // TARGET_RECT_H
