#include "EmptyReader.h"

EmptyReader::EmptyReader(bool convert_to_gray) : FileReaderInterface(convert_to_gray)
{
	this->init();
}

EmptyReader::EmptyReader(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, bool convert_to_gray, std::ostream *error) : FileReaderInterface(convert_to_gray)
{
	this->init();
	this->openFile(in_file, width, height, fps, repeat, error);
}

EmptyReader::~EmptyReader(void)
{
	this->closeFile();
}

void EmptyReader::init()
{
	this->open = false;
}

bool EmptyReader::isOpened()
	{
		return this->open;
	}

bool EmptyReader::openFile(const std::string &in_file, unsigned int &width, unsigned int &height, double &fps, bool repeat, std::ostream *error)
{
  this->closeFile();
  this->open = true;
  width = EMPTY_READER_WIDTH;
  height = EMPTY_READER_HEIGHT;
  fps = 30;
  return true;
}

bool EmptyReader::getFrame(void *out_frame, std::ostream *error)
	{
    if(!this->isOpened())
      {
        return false;
      }
	std::memset(out_frame, 0, EMPTY_READER_WIDTH*EMPTY_READER_HEIGHT*(convert_to_gray ? 1 : 4));
    return true;
	}

bool EmptyReader::seekToFrame(unsigned int frame_pos, std::ostream *error)
  {
    return true;
  }

void EmptyReader::closeFile()
{
  this->open = false;
}

bool EmptyReader::isEof()
{
	return false;
}
