#include "WaveletKernelGenerator.h"


WaveletKernelGeneratorArg::WaveletKernelGeneratorArg(std::array<int, 2> in_pos, std::array<int, 2> in_reg_pos, e_subband in_subband, unsigned int in_memory_block) : pos(in_pos), reg_pos(in_reg_pos), subband(in_subband), memory_block(in_memory_block)
{
}

std::string WaveletKernelGeneratorArg::getName(bool is_output, bool is_call) const
{
	std::string out = "data_" + intToVariable(reg_pos[0]) + "_" + intToVariable(reg_pos[1]) + "_" + subband_labels[subband];
	if (is_output)
	{
		if(is_call) out = out + "_output";
		else out = "*" + out + "_output";
	}
	else
	{
		if ((is_call) && (pos[0] == 0) && (pos[1] == 0)) out = "*" + out + "_output";
		else out = out + "_" + intToVariable(pos[0]) + "_" + intToVariable(pos[1]);
	}
	return out;
}

std::string WaveletKernelGeneratorArg::intToVariable(int val) const
{
	return ((val < 0) ? "n" : "") + std::to_string(abs(val));
}

WaveletKernelGeneratorArg WaveletKernelGeneratorArg::toOutputArg() const
{
	return WaveletKernelGeneratorArg({0,0}, reg_pos, subband, memory_block);
}

