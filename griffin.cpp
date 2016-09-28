/*MIT License

Copyright (c) 2016 bobrofon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#define FUSE_USE_VERSION 26

#define _XOPEN_SOURCE 700

#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <cerrno>
#include <cstring>

#include <algorithm>
#include <memory>
#include <sstream>
#include <fstream>

#include <fuse.h>

namespace
{

constexpr auto g_base_path = "/tmp/.griffin/real_proc";
constexpr auto g_invisible_mark = "(griffin)";

bool starts_with(const std::string& str, const std::string& prefix)
{
	return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool ends_with(const std::string& str, const std::string& suffix)
{
	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::string self_path(const std::string& path)
{
	static const std::string self{"/self"};
	if (!starts_with(path, self))
	{
		return path;
	}
	std::stringstream ss;
	ss << '/' << fuse_get_context()->pid << path.substr(self.size());
	return ss.str();
}

std::string real_path(const std::string& path)
{
	return g_base_path + self_path(path);
}

size_t real_size(const std::string& path)
{
	std::ifstream file{path, std::ios::binary};
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str().size();
}

bool is_invisible(const std::string& path)
{
	if (ends_with(path, "self"))
	{
		return false;
	}
	try
	{
		std::ifstream file{path + "/stat"};
		std::string pid;
		std::string name;
		if (file >> pid >> name)
		{
			return name == g_invisible_mark;
		}
		return false;
	}
	catch (...)
	{
		return false;
	}
}

int getattr(const char* orig_path, struct stat* stbuf)
{
	auto path = real_path(orig_path);
	if (lstat(path.c_str(), stbuf) == -1)
	{
		return -errno;
	}
	if (!stbuf->st_size && orig_path != std::string{"/kmsg"})
	{
		stbuf->st_size = real_size(path);
	}

	return 0;
}

int readdir(const char* orig_path, void* buf, fuse_fill_dir_t filler, off_t /*offset*/, fuse_file_info* /*fi*/)
{
	auto path = real_path(orig_path);
	auto dp = std::unique_ptr<DIR, int (*)(DIR *)>(opendir(path.c_str()), closedir);
	if (!dp)
	{
		return -errno;
	}

	for (auto de = ::readdir(dp.get()); de; de = ::readdir(dp.get()))
	{
		struct stat st = {0};
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (is_invisible(path + "/" + de->d_name))
		{
			continue;
		}
		if (filler(buf, de->d_name, &st, 0))
		{
			break;
		}
	}

	return 0;
}

int open(const char* orig_path, fuse_file_info* fi)
{
	auto path = real_path(orig_path);
	auto res = ::open(path.c_str(), fi->flags);
	if (res == -1)
	{
		return -errno;
	}

	close(res);
	return 0;
}

int read(const char* orig_path, char* buf, size_t size, off_t offset, fuse_file_info* /*fi*/)
{
	auto path = real_path(orig_path);
	auto fd = ::open(path.c_str(), O_RDONLY);
	if (fd == -1)
	{
		return -errno;
	}

	auto res = pread(fd, buf, size, offset);
	if (res == -1)
	{
		res = -errno;
	}

	close(fd);
	return res;
}

int readlink(const char* orig_path, char* buf, size_t size)
{
	auto path = real_path(orig_path);
	auto res = ::readlink(path.c_str(), buf, size - 1);
	if (res == -1)
	{
		return -errno;
	}

	buf[res] = '\0';
	return 0;
}

} // namespace

int main(int argc, char* argv[])
{
	fuse_operations operations = {NULL};
	operations.getattr = getattr;
	operations.readdir = readdir;
	operations.open = open;
	operations.read = read;
	operations.readlink = readlink;

	umask(0);
	return fuse_main(argc, argv, &operations, NULL);
}
