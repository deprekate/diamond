/****
DIAMOND protein aligner
Copyright (C) 2016-2020 Max Planck Society for the Advancement of Science e.V.
                        Benjamin Buchfink
						
Code developed by Benjamin Buchfink <benjamin.buchfink@tue.mpg.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
****/


#pragma once
#include <vector>
#include <utility>
#include <iterator>
#include <string.h>
#include "stream_entity.h"
#include "../algo/varint.h"
#include "../system/endianness.h"

struct DynamicRecordReader;

struct Deserializer
{

	enum { VARINT = 1 };

	Deserializer(StreamEntity* buffer);
	void rewind();
	Deserializer& seek(size_t pos);
	void seek_forward(size_t n);
	bool seek_forward(char delimiter);
	void close();

	Deserializer(const char *begin, const char *end, int flags = 0):
		varint(flags & VARINT),
		buffer_(NULL),
		begin_(begin),
		end_(end)
	{}

	Deserializer& operator>>(unsigned &x)
	{
		if (varint)
			read_varint(*this, x);
		else {
			read(x);
			x = big_endian_byteswap(x);
		}
		return *this;
	}

	Deserializer& operator>>(int &x)
	{
		read(x);
		return *this;
	}

	Deserializer& operator>>(unsigned long &x)
	{
		read(x);
		x = big_endian_byteswap(x);
		return *this;
	}

	Deserializer& operator>>(unsigned long long &x)
	{
		read(x);
		x = big_endian_byteswap(x);
		return *this;
	}

	Deserializer& operator>>(double &x)
	{
		read(x);
		return *this;
	}

	Deserializer& operator>>(std::string &s)
	{
		s.clear();
		if (!read_to(std::back_inserter(s), '\0'))
			throw EndOfStream();
		return *this;
	}

	Deserializer& operator>>(std::vector<std::string> &v)
	{
		uint32_t n;
		*this >> n;
		v.clear();
		v.reserve(n);
		std::string s;
		for (uint32_t i = 0; i < n; ++i) {
			*this >> s;
			v.push_back(std::move(s));
		}
		return *this;
	}

	Deserializer& operator>>(std::vector<uint32_t> &v)
	{
		uint32_t n, x;
		*this >> n;
		v.clear();
		v.reserve(n);
		for (unsigned i = 0; i < n; ++i) {
			*this >> x;
			v.push_back(x);
		}
		return *this;
	}

	template<typename _t>
	void read(_t &x)
	{
		if (avail() >= sizeof(_t)) {
			x = *(const _t*)begin_;
			begin_ += sizeof(_t);
		}
		else if (read_raw((char*)&x, sizeof(_t)) != sizeof(_t))
			throw EndOfStream();
	}

	template<class _t>
	size_t read(_t *ptr, size_t count)
	{
		return read_raw((char*)ptr, count*sizeof(_t)) / sizeof(_t);
	}

	const char* data() const
	{
		return begin_;
	}

	template<typename _it>
	bool read_to(_it dst, char delimiter)
	{
		int d = delimiter;
		do {
			const char* p = (const char*)memchr((void*)begin_, d, avail());
			if (p == 0) {
				std::copy(begin_, end_, dst);
			}
			else {
				const size_t n = p - begin_;
				std::copy(begin_, begin_ + n, dst);
				begin_ += n + 1;
				return true;
			}
		} while (fetch());
		return false;
	}

	size_t read_raw(char *ptr, size_t count);
	DynamicRecordReader read_record();
	~Deserializer();

	bool varint;

protected:
	
	void pop(char *dst, size_t n);
	bool fetch();

	size_t avail() const
	{
		return end_ - begin_;
	}
	
	StreamEntity *buffer_;
	const char *begin_, *end_;
};
