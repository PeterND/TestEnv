#include "DataStream_pack.h"
#include <string>
#include <algorithm>
namespace c3pack_down
{
	DataStream::DataStream() :
	mStream(0),
	mSize((size_t) - 1)
	{
	}

	DataStream::DataStream(std::istream* _stream) :
	mStream(_stream),
	mSize((size_t) - 1)
	{
	}

	DataStream::~DataStream()
	{
	}

	size_t DataStream::size()
	{
		if (mStream == 0) return 0;
		if (mSize == (size_t) - 1)
		{
			mStream->seekg (0, std::ios::end);
			mSize = (size_t)mStream->tellg();
			mStream->seekg (0, std::ios::beg);
		}
		return mSize;
	}

	bool DataStream::eof()
	{
		return mStream == 0 ? true : mStream->eof();
	}

	void DataStream::readline(std::string& _source, unsigned int _delim)
	{
		if (mStream == 0) return;
		std::getline(*mStream, _source, (char)_delim);
	}

	size_t DataStream::read(void* _buf, size_t _count)
	{
		if (mStream == 0) return 0;
		size_t count = std::min(size(), _count);
		mStream->read((char*)_buf, count);
		return count;
	}
}