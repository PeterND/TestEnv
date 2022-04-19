#ifndef __C3PACK_DATA_STREAM_PACK_H__
#define __C3PACK_DATA_STREAM_PACK_H__
#include <iostream>

namespace c3pack_down
{
	class DataStream
	{
	public:
		DataStream();
		DataStream(std::istream* _stream);
		virtual ~DataStream();

		virtual bool eof();
		virtual size_t size();
		virtual void readline(std::string& _source, unsigned int _delim);
		virtual size_t read(void* _buf, size_t _count);

	protected:
		std::istream* mStream;
		size_t mSize;
	};
}
#endif // __MYGUI_DATA_STREAM_H__
