#include "WaveletOpenmpSep.h"


void transformSepOpenmpCopy(float *out_image, float *in_image, float *temp_image_1, float *temp_image_2, size_t width, size_t height, double &time)
{
  size_t half_width = width / 2;
  
  double start = omp_get_wtime();
#pragma omp parallel default(none) shared(width, in_image, out_image, height, half_width, temp_image_1, temp_image_2)
  {
    #pragma omp for
      for(int j = 0; j < height; j++)
          {
            size_t act_start = j * width;
            size_t temp_start = width * omp_get_thread_num();

            for(size_t i = 0; i < half_width; i++)
              {
                temp_image_1[temp_start + i] = in_image[act_start + i * 2];
                temp_image_2[temp_start + i] = in_image[act_start + i * 2 + 1];
              }
            for(size_t i = 0; i < half_width - 1; i++)
              {
                temp_image_2[temp_start + i] += ALPHA * (temp_image_1[temp_start + i] + temp_image_1[temp_start + i + 1]);
              }
            temp_image_2[temp_start + half_width - 1] += 2 * ALPHA * temp_image_1[temp_start + half_width - 1];
        
            temp_image_1[temp_start] += 2 * BETA * temp_image_2[temp_start];
            for(size_t i = 1; i < half_width; i++)
              {
                temp_image_1[temp_start + i] += BETA * (temp_image_2[temp_start + i - 1] + temp_image_2[temp_start + i]);
              }
            for(size_t i = 0; i < half_width - 1; i++)
              {
                temp_image_2[temp_start + i] += GAMMA * (temp_image_1[temp_start + i] + temp_image_1[temp_start + i + 1]);
              }
            temp_image_2[temp_start + half_width - 1] += 2 * GAMMA * temp_image_1[temp_start + half_width - 1];
        
            temp_image_1[temp_start] += 2 * DELTA * temp_image_2[temp_start];

            for(size_t i = 1; i < half_width; i++)
              {
                temp_image_1[temp_start + i] += DELTA * (temp_image_2[temp_start + i - 1] + temp_image_2[temp_start + i]);
              }

            for(size_t i = 0; i < half_width; i++)
              {
                out_image[i * height + j] = ZETA1 * temp_image_1[temp_start + i];
                out_image[(i + half_width) * height + j] = ZETA2 * temp_image_2[temp_start + i];
              }
          }
      }
  double end = omp_get_wtime();
  time = (end - start) * 1000.0;
}

void transformSepOpenmpSimd(float *out_image, float *in_image, size_t width, size_t height, double &time)
{
	size_t half_width = width / 2;
	double start = omp_get_wtime();
#pragma omp parallel default(none) shared(half_width, width, in_image, out_image, height)
	{
#pragma omp for
	  for (int j = 0; j < height; j++)
		{
			size_t act_start = j * width;
      #ifndef _WIN32
      #pragma omp simd
      #endif
			for (size_t i = act_start; i < act_start + width - 2; i += 2)
			{
				in_image[i + 1] += ALPHA * (in_image[i] + in_image[i + 2]);
			}
			in_image[act_start + width - 1] += 2 * ALPHA * in_image[act_start + width - 2];

			in_image[act_start] += 2 * BETA * in_image[act_start + 1];
      #ifndef _WIN32
      #pragma omp simd
      #endif
			for (size_t i = act_start + 2; i < act_start + width; i += 2)
			{
				in_image[i] += BETA * (in_image[i - 1] + in_image[i + 1]);
			}
      #ifndef _WIN32
      #pragma omp simd
      #endif
			for (size_t i = act_start; i < act_start + width - 2; i += 2)
			{
				in_image[i + 1] += GAMMA * (in_image[i] + in_image[i + 2]);
			}
			in_image[act_start + width - 1] += 2 * GAMMA * in_image[act_start + width - 2];

			in_image[act_start] += 2 * DELTA * in_image[act_start + 1];
      #ifndef _WIN32
      #pragma omp simd
      #endif
			for (size_t i = act_start + 2; i < act_start + width; i += 2)
			{
				in_image[i] += DELTA * (in_image[i - 1] + in_image[i + 1]);
			}
      #ifndef _WIN32
      #pragma omp simd
      #endif
			for (size_t i = 0; i < half_width; i++)
			{
				out_image[i * height + j] = ZETA1 * in_image[act_start + i * 2];
				out_image[(i + half_width) * height + j] = ZETA2 * in_image[act_start + i * 2 + 1];
			}
		}
	}
  double end = omp_get_wtime();
  time = (end - start) * 1000.0;
}

void transformSepOpenmpCopySimd(float *out_image, float *in_image, float *temp_image_1, float *temp_image_2, size_t width, size_t height, double &time)
{
  size_t half_width = width / 2;
  
  double start = omp_get_wtime();
#pragma omp parallel default(none) shared(width, in_image, out_image, height, half_width, temp_image_1, temp_image_2)
  {
    #pragma omp for
      for(int j = 0; j < height; j++)
          {
            size_t act_start = j * width;
            size_t temp_start = width * omp_get_thread_num();
            for(size_t i = 0; i < half_width; i++)
              {
                temp_image_1[temp_start + i] = in_image[act_start + i * 2];
                temp_image_2[temp_start + i] = in_image[act_start + i * 2 + 1];
              }
            #ifndef _WIN32
            #pragma omp simd
            #endif
            for(size_t i = 0; i < half_width - 1; i++)
              {
                temp_image_2[temp_start + i] += ALPHA * (temp_image_1[temp_start + i] + temp_image_1[temp_start + i + 1]);
              }
            temp_image_2[temp_start + half_width - 1] += 2 * ALPHA * temp_image_1[temp_start + half_width - 1];
        
            temp_image_1[temp_start] += 2 * BETA * temp_image_2[temp_start];
            #ifndef _WIN32
            #pragma omp simd
            #endif
            for(size_t i = 1; i < half_width; i++)
              {
                temp_image_1[temp_start + i] += BETA * (temp_image_2[temp_start + i - 1] + temp_image_2[temp_start + i]);
              }
            #ifndef _WIN32
            #pragma omp simd
            #endif
            for(size_t i = 0; i < half_width - 1; i++)
              {
                temp_image_2[temp_start + i] += GAMMA * (temp_image_1[temp_start + i] + temp_image_1[temp_start + i + 1]);
              }
            temp_image_2[temp_start + half_width - 1] += 2 * GAMMA * temp_image_1[temp_start + half_width - 1];
        
            temp_image_1[temp_start] += 2 * DELTA * temp_image_2[temp_start];
            #ifndef _WIN32
            #pragma omp simd
            #endif
            for(size_t i = 1; i < half_width; i++)
              {
                temp_image_1[temp_start + i] += DELTA * (temp_image_2[temp_start + i - 1] + temp_image_2[temp_start + i]);
              }

            for(size_t i = 0; i < half_width; i++)
              {
                out_image[i * height + j] = ZETA1 * temp_image_1[temp_start + i];
                out_image[(i + half_width) * height + j] = ZETA2 * temp_image_2[temp_start + i];
              }
          }
      }
  double end = omp_get_wtime();
  time = (end - start) * 1000.0;
}

void transformSepOpenmp(float *out_image, float *in_image, size_t width, size_t height, double &time)
{
	size_t half_width = width / 2;
	double start = omp_get_wtime();
#pragma omp parallel default(none) shared(half_width, width, in_image, out_image, height)
	{
#pragma omp for
	  for (int j = 0; j < height; j++)
		{
			size_t act_start = j * width;
			for (size_t i = act_start; i < act_start + width - 2; i += 2)
			{
				in_image[i + 1] += ALPHA * (in_image[i] + in_image[i + 2]);
			}
			in_image[act_start + width - 1] += 2 * ALPHA * in_image[act_start + width - 2];

			in_image[act_start] += 2 * BETA * in_image[act_start + 1];
			for (size_t i = act_start + 2; i < act_start + width; i += 2)
			{
				in_image[i] += BETA * (in_image[i - 1] + in_image[i + 1]);
			}

			for (size_t i = act_start; i < act_start + width - 2; i += 2)
			{
				in_image[i + 1] += GAMMA * (in_image[i] + in_image[i + 2]);
			}
			in_image[act_start + width - 1] += 2 * GAMMA * in_image[act_start + width - 2];

			in_image[act_start] += 2 * DELTA * in_image[act_start + 1];
			for (size_t i = act_start + 2; i < act_start + width; i += 2)
			{
				in_image[i] += DELTA * (in_image[i - 1] + in_image[i + 1]);
			}
			for (size_t i = 0; i < half_width; i++)
			{
				out_image[i * height + j] = ZETA1 * in_image[act_start + i * 2];
				out_image[(i + half_width) * height + j] = ZETA2 * in_image[act_start + i * 2 + 1];
			}
		}
	}
  double end = omp_get_wtime();
  time = (end - start) * 1000.0;
}


void transformSepSingle(float *out_image, float *in_image, size_t width, size_t height, double &time)
{
	size_t half_width = width / 2;
	double start = omp_get_wtime();
	for(int j = 0; j < height; j++)
		{
			size_t act_start = j * width;
			for (size_t i = act_start; i < act_start + width - 2; i += 2)
			{
				in_image[i + 1] += ALPHA * (in_image[i] + in_image[i + 2]);
			}
			in_image[act_start + width - 1] += 2 * ALPHA * in_image[act_start + width - 2];

			in_image[act_start] += 2 * BETA * in_image[act_start + 1];
			for (size_t i = act_start + 2; i < act_start + width; i += 2)
			{
				in_image[i] += BETA * (in_image[i - 1] + in_image[i + 1]);
			}

			for (size_t i = act_start; i < act_start + width - 2; i += 2)
			{
				in_image[i + 1] += GAMMA * (in_image[i] + in_image[i + 2]);
			}
			in_image[act_start + width - 1] += 2 * GAMMA * in_image[act_start + width - 2];

			in_image[act_start] += 2 * DELTA * in_image[act_start + 1];
			for (size_t i = act_start + 2; i < act_start + width; i += 2)
			{
				in_image[i] += DELTA * (in_image[i - 1] + in_image[i + 1]);
			}
			for (size_t i = 0; i < half_width; i++)
			{
				out_image[i * height + j] = ZETA1 * in_image[act_start + i * 2];
				out_image[(i + half_width) * height + j] = ZETA2 * in_image[act_start + i * 2 + 1];
			}
		}
  double end = omp_get_wtime();
  time = (end - start) * 1000.0;
}

inline int mirror_repeat(int id, int max_val)
{
  int out = (abs(id) % (2 * max_val - 2));
  return (out < max_val) ? out : 2 * max_val - out - 1;
}

inline void write_quad(float *coef, int x, int y, size_t width, float data)
{
  coef[x * 2 + y * 2 * width] = data;
  coef[x * 2 + 1 + y * 2 * width] = data;
  coef[x * 2 + (y * 2 + 1) * width] = data;
  coef[x * 2  + 1 + (y * 2 + 1) * width] = data;
}

void split_image(float *ll, float *hl, float *lh, float *hh, float *in_image, int width, int height, double &time)
{
  int half_width = width / 2;
  int half_height = height / 2;
  for(int y = 0; y < half_height; y++)
    {
      for(int x = 0; x < half_width; x++)
        {
          write_quad(ll, x, y, width, in_image[x + y * width]);
          write_quad(hl, x, y, width, in_image[x + half_width + y * width]);
          write_quad(lh, x, y, width, in_image[x + (y + half_height) * width]);
          write_quad(hh, x, y, width, in_image[x + half_width + (y + half_height) * width]);
        }
    }
}

void transformSepSingleMod(float *out_image, float *in_image, int width, int height, s_wavelet_type_info *wavelet_info, double &time)
{
	int half_width = width / 2;
	double start = omp_get_wtime();
  float norm_invert = 1.0f / wavelet_info->norm;
	for(int j = 0; j < height; j++)
		{
			int act_start = j * width;
      int act_end = act_start + width;
      for(int k = 0; k < (int)wavelet_info->steps; k++)
      {
        float *act_predict_coef = &(wavelet_info->coef[wavelet_info->width * 2 * k]);
        float *act_update_coef = &(wavelet_info->coef[wavelet_info->width*(2*k+1)]);
			  for(int i = act_start + 1; i < act_start + width; i += 2)
			    {
            for(int l = 0; l < (int)wavelet_info->width; l++)
              {
                in_image[i] += act_predict_coef[l] * (in_image[mirror_repeat(i - 1 - 2 * l, act_end)] + in_image[mirror_repeat(i + 1 + 2 * l, act_end)]);
              }
			    }

			  for(int i = act_start; i < act_start + width; i += 2)
			    {
            for(int l = 0; l < (int)wavelet_info->width; l++)
              {
                in_image[i] += act_update_coef[l] * (in_image[mirror_repeat(i - 1 - 2 * l, act_end)] + in_image[mirror_repeat(i + 1 + 2 * l, act_end)]);
              }
			    }
      }
      for(int i = 0; i < half_width; i++)
			{
        out_image[i * height + j] = wavelet_info->norm * in_image[act_start + i * 2];
        out_image[(i + half_width) * height + j] = in_image[act_start + i * 2 + 1] / norm_invert;
			}
		}
  double end = omp_get_wtime();
  time = (end - start) * 1000.0;
}


WaveletOpenmpSep::WaveletOpenmpSep(rect image_size, WaveletOpenmpParamSep *param) : WaveletOpenmp(image_size, param),temp_image_1(NULL),temp_image_2(NULL),param(param)
{
  omp_set_num_threads(4);
  this->createResources();
}

WaveletOpenmpSep::~WaveletOpenmpSep()
{
  this->deleteResources();
}

void WaveletOpenmpSep::deleteBuffers()
{
  if(!this->isValid()) return;
  delete[] temp_image_1;
  delete[] temp_image_2;
}

void WaveletOpenmpSep::deleteResources()
{
  this->deleteBuffers();
}

bool WaveletOpenmpSep::createBuffers()
{
  if(!this->isValid())
    {
      this->error_msg << "Error: Detector isnt in valid state.\n";
      return false;
    }
  temp_image_1 = new float[image_size.w * 4];
  temp_image_2 = new float[image_size.w * 4];
  return true;
}

bool WaveletOpenmpSep::createTimers()
{
  if(!this->isValid())
    {
      this->error_msg << "Error: Detector isnt in valid state.\n";
      return false;
    }
  this->proc_time.resize(2);
  return true;
}

bool WaveletOpenmpSep::createResources()
{
  if(!this->image_size.isValid() || 
     !this->createBuffers() ||
     !this->createTimers())
    {
      this->deleteResources();
      return false;
    }
  #if DEBUG_LEVEL == DEBUG_ALL
  this->param->printDebug();
  #endif
  return true;
}

void wavelet_transform_split(float *ll, float *hl, float *lh, float *hh, float *in_data, int width, int height, s_wavelet_type_info *wavelet_info)
{
  double time;
  float * temp_data_1 = (float *)malloc(sizeof(float) * width * height);
  float * temp_data_2 = (float *)malloc(sizeof(float) * width * height);
  transformSepSingleMod(temp_data_1, in_data, width, height, wavelet_info, time);
  transformSepSingleMod(temp_data_2, temp_data_1, height, width, wavelet_info, time);
  split_image(ll, hl, lh, hh, temp_data_2, width, height, time);
  free(temp_data_1);
  free(temp_data_2);
}

bool WaveletOpenmpSep::getFrame(uchar4 *bgra_frame, det_output *out)
	{
		if(!this->isValid())
			{
        this->error_msg << "Error: Detector isnt in valid state.\n";
				return false;
			}
    float * temp_data_ll = (float *)malloc(sizeof(float)*this->image_size.w*this->image_size.h);
    float * temp_data_hl = (float *)malloc(sizeof(float)*this->image_size.w*this->image_size.h);
    float * temp_data_lh = (float *)malloc(sizeof(float)*this->image_size.w*this->image_size.h);
    float * temp_data_hh = (float *)malloc(sizeof(float)*this->image_size.w*this->image_size.h);
    if(!this->WaveletOpenmp::getFramePre(bgra_frame)) return false;
    //wavelet_transform_split(this->f_gray_image_1, temp_data_hl, temp_data_lh, temp_data_hh, this->f_gray_image_1, this->image_size.w, this->image_size.h, this->param->wavelet_info);
    //wavelet_transform_split(temp_data_hl, this->f_gray_image_1, temp_data_lh, temp_data_hh, this->f_gray_image_1, this->image_size.w, this->image_size.h, this->param->wavelet_info);
    //wavelet_transform_split(temp_data_hl, temp_data_lh, this->f_gray_image_1, temp_data_hh, this->f_gray_image_1, this->image_size.w, this->image_size.h, this->param->wavelet_info);
    //wavelet_transform_split(temp_data_hl, temp_data_lh, temp_data_hh, this->f_gray_image_1, this->f_gray_image_1, this->image_size.w, this->image_size.h, this->param->wavelet_info);
    this->transformHor(this->f_gray_image_2, this->f_gray_image_1, this->image_size.w, this->image_size.h, this->proc_time[0]);
    this->transformVert(this->f_gray_image_1, this->f_gray_image_2, this->image_size.h, this->image_size.w, this->proc_time[1]);
    if(!this->WaveletOpenmp::getFramePost(bgra_frame, this->f_gray_image_2, this->f_gray_image_1, out)) return false;
    free(temp_data_ll);
    free(temp_data_hl);
    free(temp_data_lh);
    free(temp_data_hh);
    return true;
	}

void WaveletOpenmpSep::transformVert(float *out_image, float *in_image, size_t width, size_t height, double &time)
{
  switch(this->param->vert_func)
    {
      case OPENMP_SEP_FUNC_SINGLE:
        //transformSepSingleMod(out_image, in_image, width, height, this->param->wavelet_info, time);
        transformSepSingle(out_image, in_image, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP:
        transformSepOpenmp(out_image, in_image, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP_COPY:
        transformSepOpenmpCopy(out_image, in_image, this->temp_image_1, this->temp_image_2, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP_SIMD:
        transformSepOpenmpSimd(out_image, in_image, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP_COPY_SIMD:
        transformSepOpenmpCopySimd(out_image, in_image, this->temp_image_1, this->temp_image_2, width, height, time);
      break;
    }
}

void WaveletOpenmpSep::transformHor(float *out_image, float *in_image, size_t width, size_t height, double &time)
{
  switch(this->param->hor_func)
    {
      case OPENMP_SEP_FUNC_SINGLE:
        //transformSepSingleMod(out_image, in_image, width, height, this->param->wavelet_info, time);
        transformSepSingle(out_image, in_image, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP:
        transformSepOpenmp(out_image, in_image, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP_COPY:
        transformSepOpenmpCopy(out_image, in_image, this->temp_image_1, this->temp_image_2, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP_SIMD:
        transformSepOpenmpSimd(out_image, in_image, width, height, time);
      break;
      case OPENMP_SEP_FUNC_OPENMP_COPY_SIMD:
        transformSepOpenmpCopySimd(out_image, in_image, this->temp_image_1, this->temp_image_2, width, height, time);
      break;
    }
}