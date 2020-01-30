/****
DIAMOND protein aligner
Copyright (C) 2013-2017 Benjamin Buchfink <buchfink@gmail.com>

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

#ifndef LOG_STREAM_H_
#define LOG_STREAM_H_

#include <iostream>
#include <fstream>
#include <mutex>
#include <limits.h>
#include <chrono>

using std::endl;

struct Message_stream
{
	Message_stream(bool to_cout = true, bool to_file = false) :
		to_cout_(to_cout),
		to_file_(to_file)
	{}
	template<typename _t>
	Message_stream& operator<<(const _t& x)
	{
		if(to_cout_)
			std::cerr << x;
		if (to_file_) {
			std::ofstream f("diamond.log", std::ios_base::out | std::ios_base::app);
			f << x;
			f.close();
		}
		return *this;
	}
	//Message_stream& operator<<(std::ostream & (__cdecl *_Pfn)(std::ostream&))
	Message_stream& operator<<(std::ostream & (*_Pfn)(std::ostream&))
	{
		if(to_cout_)
			((*_Pfn)(std::cerr));
		if (to_file_) {
			mtx.lock();
			std::ofstream f("diamond.log", std::ios_base::out | std::ios_base::app);
			((*_Pfn)(f));
			f.close();
			mtx.unlock();
		}
		return *this;
	}
	static std::mutex mtx;
private:
	bool to_cout_, to_file_;
};

extern Message_stream message_stream;
extern Message_stream verbose_stream;
extern Message_stream log_stream;

struct task_timer
{
	task_timer(unsigned level = 1) :
		level_(level),
		msg_(nullptr)
	{
		start(nullptr);
	}
	task_timer(const char *msg, unsigned level=1) :
		level_(level),
		msg_(msg)
	{
		start(msg);
	}
	~task_timer()
	{
		finish();
	}
	void go(const char *msg)
	{
		finish();
		start(msg);
		msg_ = msg;
	}
	void finish()
	{
		if (!msg_ || level_ == UINT_MAX)
			return;
		//if (print_ && !Cfg::debug_log)
		get_stream() << " [" << get() << "s]" << endl;
		/*else if (Cfg::debug_log) {
			log_stream << '/' << msg_ << " [" << timer_.getElapsedTimeInSec() << "s]" << endl;
		}*/
		msg_ = 0;
	}
	double get()
	{
		return (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t).count() / 1000.0;
	}
	Message_stream& get_stream() const
	{
		switch (level_) {
		case 1:
			return message_stream;
		case 2:
			return verbose_stream;
		case 3:
			return log_stream;
		default:
			return message_stream;
		}
	}
private:
	void start(const char *msg)
	{
		//if (print_ && !Cfg::debug_log) {
		if (level_ == UINT_MAX)
			return;
		t = std::chrono::high_resolution_clock::now();
		if (!msg)
			return;
		get_stream() << msg << "... " << std::flush;
			//fflush(stdout);
		/**}
		else if (Cfg::debug_log)
			log_stream << msg << "..." << endl;*/
	}
	unsigned level_;
	const char *msg_;
	std::chrono::high_resolution_clock::time_point t;
};

#endif
