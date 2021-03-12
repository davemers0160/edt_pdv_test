/*
* Copyright 2021 David Emerson, NSWC Crane
*
* TODO: Add licensing statement.
*/

// Stores values for each camera

#ifndef MS_TRACKER_INCLUDE_H
#define MS_TRACKER_INCLUDE_H

#include <cstdint>
#include <list>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

//#include "camera.h"
//#include "target.h"

// ----------------------------------------------------------------------------
/**
Tracker variables

These are the IDs that can be used to select various tracker types.
*/
enum tracker_types
{
    BOOSTING,
    MIL,
    KCF,
    TLD,
    MEDIANFLOW,
    GOTURN,
    MOSSE,
    CSRT
};


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

// ----------------------------------------------------------------------------
/** @brief Multispectral Tracker Class

This class 
*/
class ms_tracker
{
public:

    //template <typename tracker_type>
    //tracker_type tracker;

    cv::Ptr<cv::Tracker> tracker;   /**<  */


    int32_t lost_track;             /**< how many frames camera has lost track */

    int32_t max_lost;               /**< how many frames the tracker is unable to track an object before starting over with detect */

    //Camera cam;                     /**< this is the camera that supplies data to the tracker */

    //bool has_detection;
    bool tracking;

    // ----------------------------------------------------------------------------
    ms_tracker() = default;

	ms_tracker(int32_t track_type) 
	{
        max_lost = 10;
        lost_track = 0;
        tracking = false;

        create(track_type);
	}

    ~ms_tracker()
    {
        //targets.clear();
        tracker.release();
    }

	// ----------------------------------------------------------------------------
    //void add_camera(Camera &c)
    //{
    //    cams.push_back(c);    // std::move(c);
    //}

    // ----------------------------------------------------------------------------
    void add_target(target_rect &t)
    {
        //targets.push_front(t);
        target = t;
        tracking = true;
    }

    // ----------------------------------------------------------------------------
    //std::list<target>::iterator get_targets()
    target_rect get_target()
    {
        //return targets.begin();
        return target;
    }

    //// ----------------------------------------------------------------------------
    //void remove_last_target()
    //{
    //    targets.pop_back();
    //}

    //// ----------------------------------------------------------------------------
    //void remove_first_target()
    //{
    //    targets.pop_front();
    //}

    // ----------------------------------------------------------------------------
    void remove_target()
    {
        //std::list<target>::iterator it = targets.begin();
        //std::advance(it, index);

        target = target_rect();
        tracking = false;
    }

    // ----------------------------------------------------------------------------
    //uint32_t get_num_targets()
    //{
    //    return (uint32_t)targets.size();
    //}

    // ----------------------------------------------------------------------------
    // create the tracker object
    // For explanation of trackers, see https://learnopencv.com/object-tracking-using-opencv-cpp-python/
    void create(int32_t tracker_type)
    {
        switch (tracker_type)
        {
        case BOOSTING:
            tracker = cv::TrackerBoosting::create();
            break;
        case MIL:
            tracker = cv::TrackerMIL::create();
            break;
        case KCF:
            tracker = cv::TrackerKCF::create();
            break;
        case TLD:
            tracker = cv::TrackerTLD::create();
            break;
        case MEDIANFLOW:
            tracker = cv::TrackerMedianFlow::create();
            break;
        case GOTURN:
            tracker = cv::TrackerGOTURN::create();
            break;
        case MOSSE:
            tracker = cv::TrackerMOSSE::create();
            break;
        case CSRT:
            tracker = cv::TrackerCSRT::create();
            break;
        default:
            tracker = cv::TrackerKCF::create();
            break;
        }
    }
        
    // ----------------------------------------------------------------------------
    bool init(cv::Mat &img, target_rect &roi)
    {
        cv::Rect2d r(roi.x, roi.y, roi.w, roi.h);
        tracking = tracker->init(img, r);
        roi = target_rect((int32_t)r.x, (int32_t)r.y, (int32_t)r.width, (int32_t)r.height);
        return tracking;
    }   // end of init

    bool init(uint8_t* d, int32_t h, int32_t w, int64_t* rx, int64_t* ry, int64_t* rw, int64_t* rh)
    {
        cv::Rect2d r((double)(*rx), (double)(*ry), (double)(*rw), (double)(*rh));

        cv::Mat img = cv::Mat(h, w, CV_8UC1, d, w * sizeof(*d));

        bool tracking = tracker->init(img, r);

        *rx = (int64_t)std::floor(r.x + 0.5);
        *ry = (int64_t)std::floor(r.y + 0.5);
        *rw = (int64_t)std::floor(r.width + 0.5);
        *rh = (int64_t)std::floor(r.height + 0.5);

        return tracking;
    }   // end of init

    // ----------------------------------------------------------------------------
    bool update(cv::Mat& img, target_rect& roi)
    {
        cv::Rect2d r(roi.x, roi.y, roi.w, roi.h);
        bool tracking = tracker->update(img, r);
        roi = target_rect((int32_t)r.x, (int32_t)r.y, (int32_t)r.width, (int32_t)r.height);
        return tracking;
    }   // end of update


    bool update(uint8_t *d, int32_t h, int32_t w, int64_t *rx, int64_t *ry, int64_t *rw, int64_t *rh)
    {
        cv::Rect2d r((double)(*rx), (double)(*ry), (double)(*rw), (double)(*rh));

        cv::Mat img = cv::Mat(h, w, CV_8UC1, d, w * sizeof(*d));

        bool tracking = tracker->update(img, r);

        *rx = (int64_t)std::floor(r.x + 0.5);
        *ry = (int64_t)std::floor(r.y + 0.5);
        *rw = (int64_t)std::floor(r.width + 0.5);
        *rh = (int64_t)std::floor(r.height + 0.5);

        return tracking;
    }   // end of update

    // ----------------------------------------------------------------------------
    // TODO: lots of stuff to work out on the find object side
    bool find_object(cv::Mat& img)
    {
        int32_t idx, jdx;
        bool result = true;
//        std::list<target> tg;
        target_rect new_tgt;
        int32_t l, t, b, r;

        lost_track = 0;

        // run a simple blob detector to get an initial detection
        blob_detection(img, new_tgt, 40.0);

        // going to assume that there is no target detections
        //if (!cam.keypoints.empty()) //{

        // there are no current targets
        //if (targets.empty() && !tg.empty())
        if (tracking == false)
        {
            // copy t into targets if t is not empty
            //targets.assign(tg.begin(), tg.end());
            target = new_tgt;
            tracking = true;
        }
        else
        {
            // if t is not empty then run a check for overlaps between t and targets
            // condense any targets that have an iou of min_matching_iou
            if (!target.is_empty())
            {

                // create an array of bools to track if a member of targets has been used
                //std::vector<bool> used(tg.size(), false);
                //bool used = false;



                //auto tgt = std::next(targets.begin(), idx);
                //target new_tgt = std::next(tg.begin(), jdx);

                if (target.get_iou(new_tgt) >= min_matching_iou)
                {
                    //used = true;

                    l = std::min(target.x, new_tgt.x);
                    t = std::min(target.y, new_tgt.y);

                    r = std::max(target.x, new_tgt.x);
                    b = std::max(target.y, new_tgt.y);

                    target.x = l;
                    target.y = t;
                    target.w = r - l;
                    target.h = t - b;
                }

/*
                for (idx = 0; idx < targets.size(); ++idx)
                {
                    // run the t list backwards and check the overlap between the existing 
                    // targets and the targets found by the detector
                    for (jdx = tg.size() - 1; jdx >= 0; --jdx)
                    {
                    
                        // assuming that the detection/tracking algorithm removes all duplicate/overlaping detects
                        if (used[jdx] == false)
                        {
                            auto tgt = std::next(targets.begin(), idx);
                            auto new_tgt = std::next(tg.begin(), jdx);

                            if ((*tgt).get_iou(*new_tgt) >= min_matching_iou)
                            {
                                used[idx] = true;

                                l = std::min((*tgt).roi.x, (*new_tgt).roi.x);
                                t = std::min((*tgt).roi.y, (*new_tgt).roi.y);

                                r = std::max((*tgt).roi.x, (*new_tgt).roi.x);
                                b = std::max((*tgt).roi.y, (*new_tgt).roi.y);

                                (*tgt).roi.x = l;
                                (*tgt).roi.y = t;
                                (*tgt).roi.w = r - l;
                                (*tgt).roi.h = t - b;
                            }

                        }
                    }

                }
*/
                // cycle through the new targets and add the ones that didn't overlap
                // existing targets to the existing list
                //for (idx = 0; idx < tg.size(); ++idx)
                //{
                //    if (used[idx] == false)
                //    {
                //        auto new_tgt = std::next(tg.begin(), idx);
                //        targets.push_front(*new_tgt);
                //    }
                //}

                tracking = true;
            }
        }

        //c.display_image("Keypoints", img);
/*
        // Initialize tracker - choose a keypoint to track 
        // If other cameras exist with keypoints, see if any keypoints overlap 
        if (!targets.empty())
        {
            for (KeyPoint i : c.keypoints)
            {
                target tgt;
                // Check existing targets for overlap
                for (auto it = std::begin(targets); it != std::end(targets); ++it)
                {
                    tgt = *it;
                    //tgt.confidence = KeyPoint::overlap(i, tgt.kp);
                    //tgt.confidence = tgt.get_iou()

                    // Initialize tracker with object if same keypoint location is detected with multiple cameras - preferred.

                    if (tgt.confidence > 0)
                    {
                        // Center the ROI over the object 
                        c.roi = setROI(&i, c.roi_margin);
                        c.has_target = true;
                    }
                }
            }
        }
        if (!c.has_target)
        {
            // TODO: Find better solution or only track if multiple cameras find object
            // Assume the first keypoint is the object of interest - not great 
            // Center the ROI over the object
            c.roi = setROI(&c.keypoints[0], c.roi_margin);
            // Create new target 
            target new_target(c.roi);
            std::cout << "New target Key point: " << c.keypoints[0].pt << std::endl;
            c.has_target = true;
            // Add to targets list
            targets.push_back(new_target);
        }

        c.start_tracker(img);
        //c.display_image("Tracking", img);
    //}
        */
        return result;

    }   // end of find_object

    bool find_object(uint8_t* src, int32_t h, int32_t w)
    {
        cv::Mat img = cv::Mat(h, w, CV_8UC1, src, w * sizeof(*src));

        bool result = find_object(img);

        return result;
    }


    // ----------------------------------------------------------------------------
    void track_object(cv::Mat& img)
    {
        //auto tgt = targets.begin();

        target_rect prev_tgt = target;

        if (update(img, target)) // Tracking successful
        {
            //c.display_image("Tracking", img);
            lost_track = 0; // reset lost track counter

            // check to see if the detect matches the previous detect
            double iou = target.get_iou(prev_tgt);
            if (iou >= min_matching_iou)
            {
                target.id = prev_tgt.id;
                target.confidence = iou;
            }
        }
        else
        {
            ++lost_track;
            std::cout <<  "lost track: " << lost_track << std::endl;

            // If camera loses track for max_lost (default: 5) consecutive frames, resets camera to detection mode
            if (lost_track >= max_lost)
            {
                tracking = false;
            }
        }

    }   // end of track_object

    void track_object(uint8_t* d, int32_t h, int32_t w)
    {
        cv::Mat img = cv::Mat(h, w, CV_8UC1, d, w * sizeof(*d));

        track_object(img);

    }   // end of track_object

    // ----------------------------------------------------------------------------
    void merge_targets()
    {
        


    }   // end of merge_targets

// ----------------------------------------------------------------------------
private:

    double min_matching_iou = 0.8;
    //std::list<target> targets;      /**< std::list of the targets.  This allows the tracker to have more than one target */
    //target tgt;                     /**< single target for this tracker */
    target_rect target;


    

    // For more info, see https://learnopencv.com/blob-detection-using-opencv-python-c/
//    void blob_detection(cv::Mat& img, std::list<target>& t, float min_area = 20.0)
    void blob_detection(cv::Mat& img, target_rect& t, float min_area = 20.0)
    {
        std::vector<cv::KeyPoint> keypoints;
        cv::SimpleBlobDetector::Params params;

        double mean = cv::mean(img)[0];

        // Filter by Area.
        params.filterByArea = true;
        params.filterByColor = true;
        params.filterByInertia = true;
        params.filterByConvexity = false;

        params.minDistBetweenBlobs = 50.0;

        params.minInertiaRatio = 0.8f;
        params.maxInertiaRatio = 2.0;

        params.minArea = min_area;

        if (mean >= 128.0)
        {
            params.minThreshold = 0; // inclusive
            params.maxThreshold = 75; //exclusive
            params.blobColor = 0;
        }
        else // dark background, light blob
        {
            params.minThreshold = 150; // inclusive
            params.maxThreshold = 230; //exclusive
            params.blobColor = 255;
        }

        cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

        // Detect blobs.
        detector->detect(img, keypoints);

        // Draw detected blobs as red circles.
        // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
        //drawKeypoints(img, keypoints, im_with_keypoints, cv::Scalar(0, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        // return the keypoints in terms of a target class
        cv::Mat kp_display = img.clone();
        drawKeypoints(kp_display, keypoints, kp_display, cv::Scalar(0, 0, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::imshow("keypoints", kp_display);
        cv::waitKey(0);

        // TODO: need to add some logic to pick the largest blob and use that as the detection
        if (keypoints.size() > 0)
        {
            target_rect r((int32_t)(keypoints[0].pt.x - keypoints[0].size), (int32_t)(keypoints[0].pt.y - keypoints[0].size), (int32_t)(keypoints[0].pt.x + keypoints[0].size), (int32_t)(keypoints[0].pt.x + keypoints[0].size));
            t = r;
        }

        //for (cv::KeyPoint kp : keypoints)
        //{
        //    target_rect r((int32_t)(kp.pt.x - kp.size), (int32_t)(kp.pt.y - kp.size), (int32_t)(kp.pt.x + kp.size), (int32_t)(kp.pt.x + kp.size));

        //    t = r;
        //}

    }   // end of blob_detection

//    void blob_detection(uint8_t* d, int32_t h, int32_t w, std::list<target> &t, float min_area = 20.0)
    void blob_detection(uint8_t* d, int32_t h, int32_t w, target_rect& t, float min_area = 20.0)
    {
        cv::Mat img = cv::Mat(h, w, CV_8UC1, d, w * sizeof(*d));

        blob_detection(img, t, min_area);

    }   // end of blob_detection

};

#endif	// MS_TRACKER_INCLUDE_H
