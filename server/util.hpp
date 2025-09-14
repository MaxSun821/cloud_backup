#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <experimental/filesystem>
#include <zlib.h>
#include <jsoncpp/json/json.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cloud
{
    namespace fs = std::experimental::filesystem;
    class FileUtils
    {
    public:
        FileUtils(const std::string &filename) : filename_(filename) {}
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
            size_t pos = filename_.find_last_of("/");
            if (pos == std::string::npos)
            {
                return filename_;
            }
            return filename_.substr(pos + 1);
        }
        bool setContent(const std::string &body)
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
        bool getContent(std::string *body)
        {
            size_t fSize = this->fileSize();
            return getPosLen(body, 0, fSize);
        }
        bool getPosLen(std::string *body, size_t pos, size_t len)
        {
            size_t fSize = this->fileSize();
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
            body->resize(len);
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
        bool compress(const std::string &packname)
        {
            std::ifstream inFile(filename_.c_str(), std::ios::binary);
            std::ofstream outFile(packname.c_str(), std::ios::binary);

            if (!inFile || !outFile)
            {
                std::cerr << "Error opening file.\n";
                return false;
            }

            // 设置 zlib 压缩流
            z_stream strm;
            memset(&strm, 0, sizeof(strm));
            deflateInit(&strm, Z_DEFAULT_COMPRESSION);

            char inBuffer[1024];
            char outBuffer[1024];

            while (inFile.read(inBuffer, sizeof(inBuffer)) || inFile.gcount() > 0)
            {
                strm.avail_in = inFile.gcount();
                strm.next_in = reinterpret_cast<Bytef *>(inBuffer);

                do
                {
                    strm.avail_out = sizeof(outBuffer);
                    strm.next_out = reinterpret_cast<Bytef *>(outBuffer);
                    deflate(&strm, Z_NO_FLUSH);
                    outFile.write(outBuffer, sizeof(outBuffer) - strm.avail_out);
                } while (strm.avail_out == 0);
            }

            // 压缩完成，刷新数据
            do
            {
                strm.avail_out = sizeof(outBuffer);
                strm.next_out = reinterpret_cast<Bytef *>(outBuffer);
                deflate(&strm, Z_FINISH);
                outFile.write(outBuffer, sizeof(outBuffer) - strm.avail_out);
            } while (strm.avail_out == 0);

            deflateEnd(&strm);

            inFile.close();
            outFile.close();
            return true;
        }
        bool decompress(const std::string &filename)
        {
            std::ifstream inFile(filename_.c_str(), std::ios::binary);
            std::ofstream outFile(filename.c_str(), std::ios::binary);

            if (!inFile || !outFile)
            {
                std::cerr << "Error opening file.\n";
                return false;
            }

            // 设置 zlib 解压流
            z_stream strm;
            memset(&strm, 0, sizeof(strm));
            inflateInit(&strm);

            char inBuffer[1024];
            char outBuffer[1024];

            while (inFile.read(inBuffer, sizeof(inBuffer)) || inFile.gcount() > 0)
            {
                strm.avail_in = inFile.gcount();
                strm.next_in = reinterpret_cast<Bytef *>(inBuffer);

                do
                {
                    strm.avail_out = sizeof(outBuffer);
                    strm.next_out = reinterpret_cast<Bytef *>(outBuffer);
                    inflate(&strm, Z_NO_FLUSH);
                    outFile.write(outBuffer, sizeof(outBuffer) - strm.avail_out);
                } while (strm.avail_out == 0);
            }

            // 解压完成，刷新数据
            do
            {
                strm.avail_out = sizeof(outBuffer);
                strm.next_out = reinterpret_cast<Bytef *>(outBuffer);
                inflate(&strm, Z_FINISH);
                outFile.write(outBuffer, sizeof(outBuffer) - strm.avail_out);
            } while (strm.avail_out == 0);

            inflateEnd(&strm);

            inFile.close();
            outFile.close();
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
        bool scanDir(std::vector<std::string> *array)
        {
            for (auto &p : fs::directory_iterator(filename_))
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
        bool getContentRange(std::string *out, size_t start, size_t end)
        {
            size_t fSize = this->fileSize();
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

    class JsonUtils
    {
    public:
        static bool serialization(const Json::Value &root, std::string *str)
        {
            Json::StreamWriterBuilder swb;
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());

            std::stringstream ss;
            sw->write(root, &ss);
            *str = ss.str();
            return true;
        }
        static bool deserialization(const std::string &str, Json::Value &root)
        {
            Json::CharReaderBuilder crb;
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
            std::string err;
            bool ret = cr->parse(str.c_str(), str.c_str() + str.size(), &root, &err);
            if (ret == false)
            {
                std::cerr << "ERROR: parse error: " << err << std::endl;
                return false;
            }
            return true;
        }
    };
}

#endif