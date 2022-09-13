#include "WaveletKernelGeneratorStepGpu.h"

WaveletKernelGeneratorStepGpu::WaveletKernelGeneratorStepGpu(std::array<int, 2> comb_sizes, s_wavelet_type_info *wavelet_info, std::array<int, 2> pairs_per_thread, bool optim_thread, wavelet_optim_warp optim_warp, bool read_once, std::string function_name, WaveletKernelGeneratorStep *parent):WaveletKernelGeneratorStep(wavelet_info, pairs_per_thread, optim_thread, read_once, function_name, parent)
{
	this->comb_sizes = comb_sizes;
	this->optim_warp = optim_warp;
}

std::string WaveletKernelGeneratorStepGpu::fma(const std::string &a, const std::string &b, const std::string &c)
{
	this->op_count++;
	return a + " * " + b + " + " + c;
}

std::string WaveletKernelGeneratorStepGpu::mul(const std::string &a, const std::string &b)
{
	this->op_count++;
	return a + " * " + b;
}

std::string WaveletKernelGeneratorStepGpu::add(const std::string &a, const std::string &b)
{
	this->op_count++;
	return a + " + " + b;
}

std::string WaveletKernelGeneratorStepGpu::set(const std::string &out, const std::string &value) const
{
	return out + " = " + value + ";\n";
}

std::string WaveletKernelGeneratorStepGpu::getValue(std::string reg, std::array<unsigned char, 4> coef_local_id, std::array<int, 2> pos, std::array<int, 2> reg_pos, e_subband subband_id, bool is_read)
{
	int reg_id = reg_pos[1] * this->pairs_per_thread[0] + reg_pos[0];
	if ((pos[0] == 0) && (pos[1] == 0))
	{
		return reg + "[" + std::to_string(reg_id) + "].s" + std::to_string(subband_id);
	}
	else if ((pos[1] == 0) && (this->optim_warp == WAVELET_OPTIM_WARP_SHUFFLE))
	{
		this->shuffle_count++;
		if (pos[0] < 0) return std::string() + "__shfl_up_f(" + reg + "[" + std::to_string(reg_id) + "].s" + std::to_string(subband_id) + "," + std::to_string(abs(pos[0])) + ",0)";
		else return std::string() + "__shfl_down_f(" + reg + "[" + std::to_string(reg_id) + "].s" + std::to_string(subband_id) + "," + std::to_string(abs(pos[0])) + ",31)";
	}
	else
	{
		this->local_borders.resize(pos, coef_local_id[subband_id]);
#if DEBUG_LEVEL == DEBUG_ALL
		this->step_borders.resize(pos, subband_id);
#endif
		this->load_count++;
		int comb_sizes_count = comb_sizes[0] * comb_sizes[1];
		int pair_per_thread_count = pairs_per_thread[0] * pairs_per_thread[1];
		return std::string() + "temp_image[" + std::to_string((((int)(coef_local_id[subband_id] * comb_sizes_count * pair_per_thread_count)) + pos[0] + pos[1] * (int)(comb_sizes[0]) + reg_id * comb_sizes_count)) + "]";
	}
}

std::string WaveletKernelGeneratorStepGpu::getFloat(float val)
{
	return std::to_string(val) + "f";
}

std::string WaveletKernelGeneratorStepGpu::getBody() const
{
	std::string out;
	for (int i = 0; i < this->steps.size(); i++)
	{
		out += this->steps.at(i)->getBody();
	}
	return out + WaveletKernelGeneratorStep::getBody();
	//return body.str();
}

void WaveletKernelGeneratorStepGpu::appendStep()
{
	steps.push_back(std::shared_ptr<WaveletKernelGeneratorStep>(new WaveletKernelGeneratorStepGpu(this->comb_sizes, this->wavelet_info, pairs_per_thread, optim_thread, optim_warp, true, this->function_name + "_" + std::to_string(steps.size()))));
}
