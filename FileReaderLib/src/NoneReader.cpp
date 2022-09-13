#include "NoneReader.h"

NoneReader::NoneReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

NoneReader::NoneReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
	this->init();
	this->openFile(in_file, width, height, fps, repeat, error);
}

NoneReader::~NoneReader(void)
{
	this->closeFile();
}

void NoneReader::init()
{
	this->open = false;
}

bool NoneReader::isOpened()
	{
		return this->open;
	}

bool NoneReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  this->closeFile();
  this->open = true;
  width = 1;
  height = 1;
  fps = 30;
  return true;
}

bool NoneReader::getFrame(void *out_frame, std::ostream *error)
	{
    if(!this->isOpened())
      {
        return false;
      }
    return true;
	}

bool NoneReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
  {
    return true;
  }

void NoneReader::closeFile()
{
  this->open = false;
}

bool NoneReader::isEof()
{
	return false;
}




