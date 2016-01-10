#include "MemoryReader.h"

MemoryReader::MemoryReader(char const *ptr, qint64 len)
	: offset(-1)
{
	setData(ptr, len);
}

void MemoryReader::setData(char const *ptr, qint64 len)
{
	begin = ptr;
	end = begin + len;
}

bool MemoryReader::isSequential() const
{
	return false;
}

bool MemoryReader::open(OpenMode mode)
{
	mode |= QIODevice::Unbuffered;
	if (mode == (QIODevice::ReadOnly | QIODevice::Unbuffered)) {
		if (begin && begin < end && offset == -1) {
			if (QIODevice::open(mode)) {
				offset = 0;
				return true;
			}
		}
	}
	return false;
}

qint64 MemoryReader::pos() const
{
	return offset;
}

qint64 MemoryReader::size() const
{
	if (begin && begin < end) {
		return end - begin;
	}
	return 0;
}

bool MemoryReader::seek(qint64 pos)
{
	if (begin && begin < end) {
		if (begin + pos <= end) {
			offset = pos;
			return true;
		}
	}
	return false;
}

bool MemoryReader::atEnd() const
{
	if (begin && begin < end && offset != -1 && begin + offset < end) {
		return false;
	}
	return true;
}

bool MemoryReader::reset()
{
	if (begin && begin < end) {
		offset = 0;
		return true;
	}
	return false;
}

qint64 MemoryReader::bytesAvailable() const
{
	if (begin && begin < end && offset != -1 && begin + offset < end) {
		return end - (begin + offset);
	}
	return 0;
}

qint64 MemoryReader::bytesToWrite() const
{
	return 0;
}

bool MemoryReader::canReadLine() const
{
	return bytesAvailable() > 0;
}

bool MemoryReader::waitForReadyRead(int /*msecs*/)
{
	return bytesAvailable() > 0;
}

bool MemoryReader::waitForBytesWritten(int /*msecs*/)
{
	return false;
}

qint64 MemoryReader::readData(char *data, qint64 maxlen)
{
	qint64 n = bytesAvailable();
	if (n > 0) {
		if (n > maxlen) {
			n = maxlen;
		}
		if (data) {
			memcpy(data, begin + offset, n);
		}
		offset += n;
	}
	return n;
}

qint64 MemoryReader::readLineData(char *data, qint64 maxlen)
{
	if (begin && begin < end && offset != -1 && begin + offset < end) {
		char const *start = begin + offset;
		char const *ptr = start;
		while (ptr < end) {
			if (*ptr == '\n') {
				ptr++;
				break;
			}
			if (*ptr == '\r') {
				ptr++;
				if (ptr < end && *ptr == '\n') {
					ptr++;
				}
				break;
			}
			ptr++;
		}
		qint64 n = ptr - start;
		if (n > maxlen) {
			n = maxlen;
		}
		if (data) {
			memcpy(data, start, n);
		}
		offset += n;
		return n;
	}
	return 0;
}

qint64 MemoryReader::writeData(const char * /*data*/, qint64 /*len*/)
{
	return 0;
}
