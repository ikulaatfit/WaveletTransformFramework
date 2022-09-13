#ifndef WaveletKernelGeneratorStepGpu_H
#define WaveletKernelGeneratorStepGpu_H
#include <string>
#include <vector>
#include "WaveletKernelGeneratorStep.h"
#include "WaveletKernelGeneratorTypes.h"

class WaveletKernelGeneratorStepGpu : public WaveletKernelGeneratorStep
{
public:
	WaveletKernelGeneratorStepGpu(std::array<int, 2> comb_sizes, s_wavelet_type_info *wavelet_info, std::array<int, 2>  pairs_per_thread, bool optim_thread, wavelet_optim_warp optim_warp, bool read_once, std::string function_name, WaveletKernelGeneratorStep *parent = NULL);
	std::string add(const std::string &a, const std::string &b);
	std::string fma(const std::string &a, const std::string &b, const std::string &c);
	std::string mul(const std::string &a, const std::string &b);
	std::string set(const std::string &out, const std::string &value) const;

	std::string getValue(std::string reg, std::array<unsigned char, 4> coef_local_id, std::array<int, 2> pos, std::array<int, 2> reg_pos, e_subband subband_id, bool is_read);
	std::string getFloat(float val);

	std::string getBody() const;

protected:
	void appendStep();
	std::array<int, 2> comb_sizes;
	wavelet_optim_warp optim_warp;
};

#endif
