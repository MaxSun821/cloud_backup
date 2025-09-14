#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 1

#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <experimental/filesystem>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>

namespace cloud
{
    namespace fs = std::experimental::filesystem;
    class FileUtils
    {
    public:
        FileUtils(const std::string& filename) : filename_(filename) {}
        int64_t fileSize()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cerr << "get file size failed" << std::endl;
                return -1;
            }
            return st.st_size;
        }
        time_t getLastMTime()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cerr << "get file last modifiy time failed" << std::endl;
                return -1;
            }
            return st.st_mtime;
        }
        time_t getLastATime()
        {
            struct stat st;
            if (stat(filename_.c_str(), &st) < 0)
            {
                std::cerr << "get file last access time failed" << std::endl;
                return -1;
            }
            return st.st_atime;
        }
        std::string getFilename()
        {
            // ./abc/test.txt
            /*size_t pos = filename_.find_last_of("\\");
            if (pos == std::string::npos)
            {
                return filename_;
            }
            return filename_.substr(pos + 1);*/
            return fs::path(filename_).filename().string();
        }
        bool setContent(const std::string& body)
        {
            std::ofstream ofs;
            ofs.open(filename_, std::ios::binary);
            if (!ofs)
            {
                std::cerr << "FATAL: write open file failed" << std::endl;
                return false;
            }
            ofs.write(&body[0], body.size());
            if (ofs.good() == false)
            {
                std::cerr << "ERROR: write file failed" << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }
        bool getContent(std::string* body)
        {
            int64_t fSize = this->fileSize();
            return getPosLen(body, 0, fSize);
        }
        bool getPosLen(std::string* body, int64_t pos, int64_t len)
        {
            int64_t fSize = this->fileSize();
            if (pos + len > fSize)
            {
                std::cerr << "ERROR: get file len is error" << std::endl;
                return false;
            }

            std::ifstream ifs;
            ifs.open(filename_, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "FATAL: open file failed!" << std::endl;
                return false;
            }
            ifs.seekg(pos, std::ios::beg);
            body->resize((unsigned int)len);
            ifs.read(&(*body)[0], len);
            if (ifs.good() == false)
            {
                std::cerr << "ERROR: get file content failed" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }
        bool exists()
        {
            return fs::exists(filename_);
        }
        bool createDir()
        {
            if (this->exists())
                return true;
            return fs::create_directories(filename_);
        }
        bool scanDir(std::vector<std::string>* array)
        {
            this->createDir();
            for (auto& p : fs::directory_iterator(filename_))
            {
                if (fs::is_directory(p))
                {
                    continue;
                }
                // relative_path 带有路径的文件名
                array->push_back(fs::path(p).relative_path().string());
            }
            return true;
        }
        bool removeFile()
        {
            if (this->exists() == false)
            {
                return true;
            }
            remove(filename_.c_str());
            return true;
        }
        bool getContentRange(std::string* out, size_t start, size_t end)
        {
            int64_t fSize = this->fileSize();
            if (fSize < 0)
            {
                std::cerr << "ERROR: cannot get file size" << std::endl;
                return false;
            }
            if (start > end || end >= (size_t)fSize)
            {
                std::cerr << "ERROR: invalid range " << start << "-" << end
                    << ", file size=" << fSize << std::endl;
                return false;
            }

            size_t len = end - start + 1;
            return getPosLen(out, start, len);
        }

    private:
        std::string filename_;
    };
}

#endif