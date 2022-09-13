#ifndef WAVELET_KERNEL_GENERATOR_H
#define WAVELET_KERNEL_GENERATOR_H

#include <sstream>
#include <iostream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "WaveletKernelGeneratorStep.h"
#include "WaveletKernelGeneratorTypes.h"

struct s_proc_type
{
	std::array<unsigned char, 4> *double_proc;
	std::array<unsigned char, 4> *single_proc;
	unsigned int count;
};

/**
 * @brief Program parameters structure
 */
class WaveletKernelGenerator
{
public:
	/**
   * Default constructor for set initial undefined value.
   */
	WaveletKernelGenerator(e_kernel_type kernel_type, s_wavelet_type_info *wavelet_info, bool double_buffering, bool optim_thread, std::array<int, 2> pairs_per_thread);
	virtual ~WaveletKernelGenerator(){};
	virtual std::string getBody() = 0;

	
	e_kernel_type kernel_type;
	bool optim_thread;
	s_local_borders local_borders;
	v_step_borders step_borders;
	int op_count;
	int shuffle_count;
	int store_count;
	int load_count;
	int barrier_count;

	std::array<size_t, 2> scaled_image_size;
	std::array<int, 2> pairs_per_thread;
	s_wavelet_type_info *wavelet_info;
	wavelet_optim_warp optim_warp;
	bool double_buffering;

	std::vector<std::shared_ptr<WaveletKernelGeneratorStep>> steps;
protected:
	/**
	* Print debug information
	*/
	
	bool read_once;

	void getFirData2D(unsigned int stages, unsigned int iter_id, std::vector<float> &fir_ll, std::vector<float> &fir_hl, std::vector<float> &fir_lh, std::vector<float> &fir_hh, unsigned int &subband_row_size);
	void getFirData1D(unsigned int stages, unsigned int iter_id, std::vector<float> &fir_l, std::vector<float> &fir_h);
	void createFirFilter2D(unsigned int stages, unsigned int iter_id, cv::Mat &fir_ll_m, cv::Mat &fir_hl_m, cv::Mat &fir_lh_m, cv::Mat &fir_hh_m);
	void createFirFilter1D(unsigned int stages, unsigned int iter_id, cv::Mat &fir_l_m, cv::Mat &fir_h_m, bool improved);

	void appendRegDeclareBody(std::ostream &body, std::string type, std::string name);
	void appendRegVectDeclareBody(std::ostream &body, std::string type, std::string name, size_t vect_size);
	void appendRegSetBody(std::ostream &body, std::string name, std::string value);

	
	virtual void appendUpdateLocalBarrierBody(std::ostream &body, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> proc_mask, std::array<unsigned char, 4> &act_coef, bool start_barrier = true, bool end_barrier = true) = 0;

	void getSweldensBody(std::ostream &body);
	void getIwahashiBody(std::ostream &body);
	void getExplosiveBody(std::ostream &body);
	void getMonolithicBody(std::ostream &body);
	void getPolyphaseBody(std::ostream &body);
	void getConvolutionBody(std::ostream &body);
	void getPolyphaseSepNoneBody(std::ostream &body);
	void getPolyphaseSepHorBody(std::ostream &body);
	void getPolyphaseSepVertBody(std::ostream &body);
	void getPolyphaseSepAllBody(std::ostream &body);
	void getConvolutionSepNoneBody(std::ostream &body);
	void getConvolutionSepHorBody(std::ostream &body);
	void getConvolutionSepVertBody(std::ostream &body);
	void getConvolutionSepAllBody(std::ostream &body);

	virtual void appendStep() = 0;

	void setOptimizeThread(bool optim_thread);

	void getFirStepBody(std::ostream &body, unsigned int stages, unsigned int iter_id, std::array<unsigned char, 4> *coef_local_id, std::array<unsigned char, 4> *mask, std::array<unsigned char, 4> &act_coef);
	std::string getFilterBody();

	bool isRegGenNeeded(const std::vector<float> &fir_l, const std::vector<float> &fir_h, bool horizontal, bool vertical) const;
	bool isRegGenNeeded(const std::vector<float> &fir_ll, const std::vector<float> &fir_hl, const std::vector<float> &fir_lh, const std::vector<float> &fir_hh, unsigned int subband_row_size) const;
	bool isRegTempGenNeeded(const std::vector<float> &fir_l, const std::vector<float> &fir_h) const;
	void findBorders();
	virtual bool isHorOpAtomic() = 0;
};

#endif
