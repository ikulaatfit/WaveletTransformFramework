#ifndef WaveletKernelGeneratorArg_H
#define WaveletKernelGeneratorArg_H
#include <string>
#include <array>

enum e_subband
{
	SUB_LL = 0,
	SUB_HL = 1,
	SUB_LH = 2,
	SUB_HH = 3
};

class WaveletKernelGeneratorArg
{
public:
	WaveletKernelGeneratorArg(std::array<int, 2> in_pos, std::array<int, 2> in_reg_pos, e_subband subband, unsigned int memory_block);

	std::array<int, 2> pos;
	std::array<int, 2> reg_pos;
	e_subband subband;
	unsigned int memory_block;

	std::string getName(bool is_output = false, bool is_call = false) const;
	std::string intToVariable(int val) const;
	WaveletKernelGeneratorArg toOutputArg() const;
};

#endif
