#ifndef WAVELET_H
#define WAVELET_H
#include <cstdio>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <CL/cl.h>
#include "Debug.h"
#include "WaveletParam.h"

#define ALPHA (-1.58613434342059f)
#define BETA  (-0.0529801185729f)
#define GAMMA (0.8829110755309f)
#define DELTA (0.4435068520439f)
#define ZETA1 (1.1496043988602f)
#define ZETA2 (0.8698644516248f)

typedef struct s_uchar4
  {
    s_uchar4(unsigned char x = 0, unsigned char y = 0, unsigned char z = 0, unsigned char w = 0)
      {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
      }
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
  }uchar4;
/**
 * @brief Frame time and position counters
 */
typedef struct s_det_output
  {
    std::vector<double> pre; ///< time duration in preprocess kernels
    std::vector<double> proc; ///< time duration in detection kernels
    std::vector<double> post; ///< time duration in postprocess kernels
    /**
      * Default counters constructor - init data
      */
    s_det_output()
      {
        this->reset();
      }
    /**
      * Get total process time.
      * @return total process time
      */
    double getTotalTime()
      {
        return getTotalPreprocessTime() + getTotalProcessTime() + getTotalPostprocessTime();
      }
    /**
      * Get detection preprocess time.
      * @return detection preprocess time
      */
    double getTotalPreprocessTime()
      {
        return this->getTotalTime(this->pre);
      }
    /**
      * Get detection process time.
      * @return detection process time
      */
    double getTotalProcessTime()
      {
        return this->getTotalTime(this->proc);
      }
    /**
      * Get detection postprocess time.
      * @return detection postprocess time
      */
    double getTotalPostprocessTime()
      {
        return this->getTotalTime(this->post);
      }
    /**
      * Get detection time.
      * @param in_time vector of time to calculate
      * @return detection postprocess time
      */
    double getTotalTime(std::vector<double> &in_time)
      {
        double result = 0.0f;
        for(unsigned int i = 0 ; i < in_time.size(); i++)
          {
            result += in_time[i];
          }
        return result;
      }
    /**
      * Reset struct data.
      */
    void reset()
      {
        pre.clear();
        proc.clear();
        post.clear();
      };
  }det_output;

/**
 * Vector of frame time and position counters
 */
typedef std::vector<det_output> v_det_output;


/**
 * @brief Video time and position counters
 */
typedef struct s_det_output_stat
  {
    v_det_output output; ///< Vector of counters
    /**
     * Get avarage counters.
     * @return avarage counters
     */
    static bool getProcSort(det_output &in1, det_output &in2)
    {
      return in1.getTotalProcessTime() < in2.getTotalProcessTime();
    }

    det_output getMedianData()
      {
        det_output out;
        if(output.size() == 0) return out;
        std::sort(output.begin(), output.end(), s_det_output_stat::getProcSort);
        return output[output.size()/2];
      }
    det_output getAvgData()
      {
        size_t pre_size = 0;
        size_t proc_size = 0;
        size_t post_size = 0;
        det_output out;
        for(unsigned int i = 0; i < output.size(); i++)
          {
            for(unsigned int j = 0; j < output[i].pre.size(); j++)
              {
                if(output[i].pre.size() > pre_size)
                  {
                    pre_size = output[i].pre.size();
                  }
              }
            for(unsigned int j = 0; j < output[i].proc.size(); j++)
              {
                if(output[i].proc.size() > proc_size)
                  {
                    proc_size = output[i].proc.size();
                  }
              }
            for(unsigned int j = 0; j < output[i].post.size(); j++)
              {
                if(output[i].post.size() > post_size)
                  {
                    post_size = output[i].post.size();
                  }
              }
          }
        std::vector<double> pre(pre_size, 0.0);
        std::vector<double> proc(proc_size, 0.0);
        std::vector<double> post(post_size, 0.0);
        for(unsigned int i = 0; i < output.size(); i++)
          {
            for(unsigned int j = 0; j < output[i].pre.size(); j++)
              {
                pre[j] += output[i].pre[j];
              }
            for(unsigned int j = 0; j < output[i].proc.size(); j++)
              {
                proc[j] += output[i].proc[j];
              }
            for(unsigned int j = 0; j < output[i].post.size(); j++)
              {
                post[j] += output[i].post[j];
              }
          }
        for(unsigned int j = 0; j < pre_size; j++)
          {
            out.pre.push_back((float)(pre[j]/output.size()));
          }
        for(unsigned int j = 0; j < proc_size; j++)
          {
            out.proc.push_back((float)(proc[j]/output.size()));
          }
        for(unsigned int j = 0; j < post_size; j++)
          {
            out.post.push_back((float)(post[j]/output.size()));
          }
        return out;
      };
  }det_output_stat;

/**
 * @brief Wavelet interface
 */
class Wavelet
{
	public:
    virtual ~Wavelet(){ this->printError(); };
    /**
     * Constructor to init detector.
     */
    virtual bool isValid() = 0;
    /**
    * Transcode frame.
    * @param bgra_frame frame in BGRA format to transcode
    * @param out optional frame process timers
    * @param error optional string to write error
    */
	  virtual bool getFrame(uchar4 *bgra_frame, det_output *out = NULL) = 0;
    void printError(){if(this->error_msg.str().length() != 0){fprintf(stderr, "\nWavelet Info:\n%s\n", this->error_msg.str().c_str()); this->error_msg = std::ostringstream();}};
    void printDebug(){};
	protected:
    Wavelet(rect in_image_size, WaveletParam *in_param) : image_size(in_image_size), param(in_param){ this->depth = this->param->max_depth; if(!param->scaled_image_size.isValid()) param->scaled_image_size = image_size; };
    rect image_size;
	cl_uint depth;
    std::ostringstream error_msg;
	  WaveletParam *param;
};

#endif

