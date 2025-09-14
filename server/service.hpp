#ifndef __SERVICE_HPP__
#define __SERVICE_HPP__

#include "data.hpp"
#include "../include/httplib.h"

extern cloud::DataManager *data_;

namespace cloud
{
    class Service
    {
    public:
        Service()
        {
            Config *config = Config::getInstance();
            server_port_ = config->getServerPort();
            server_ip_ = config->getServerIp();
            downdoad_prefix_ = config->getDownloadPrefix();
        }
        bool runModule()
        {
            svr_.Post("/upload", upload);
            svr_.Get("/listshow", showList);
            svr_.Get("/", showList);
            std::string download_url = downdoad_prefix_ + "(.*)";
            svr_.Get(download_url, download);
            std::cout << "Server start at " << server_ip_ << ":" << server_port_ << std::endl;
            bool ret = svr_.listen(server_ip_.c_str(), server_port_);
            if (!ret)
            {
                std::cerr << "ERROR: listen failed! ip=" << server_ip_
                          << " port=" << server_port_ << std::endl;
            }
            return ret;
        }

    private:
        static void upload(const httplib::Request &req, httplib::Response &res)
        {
            auto ret = req.form.has_file("file");
            if (ret == false)
            {
                std::cerr << "WARNING: not file upload" << std::endl;
                res.status = 404;
                return;
            }

            const auto &file = req.form.get_file("file");

            std::string back_dir = Config::getInstance()->getBackDir();
            std::string realpath = back_dir + FileUtils(file.filename).getFilename();
            FileUtils fu(realpath);
            fu.setContent(file.content); // 将数据写入文件中

            BackupInfo info;
            info.newBackupInfo(realpath); // 组织备份的文件信息
            data_->insert(info);          // 向数据管理模块添加备份文件信息。
            return;
        }
        static std::string timeToString(time_t t)
        {
            std::string temp = std::ctime(&t);
            return temp;
        }
        static void showList(const httplib::Request &req, httplib::Response &res)
        {
            // 获取所有的文件备份信息
            std::vector<BackupInfo> array;
            data_->getAll(&array);
            // 根据所有备份信息，组织html文件数据
            std::stringstream ss;
            ss << R"(<!DOCTYPE html>
                    <html lang="en">
                    <head>
                        <meta charset="UTF-8">
                        <meta name="viewport" content="width=device-width, initial-scale=1.0">
                        <title>云备份系统</title>
                    </head>)";
            ss << R"(<body>
                    <h1>Download</h1>
                    <table>)";
            for (auto &item : array)
            {
                ss << "<tr>";
                std::string filename = FileUtils(item.real_path).getFilename();
                ss << R"(<td><a href=")" << item.url << R"(">)" << filename << "</a></td>";
                ss << R"(<td align="right">)" << timeToString(item.mtime) << "</td>";
                ss << R"(<td align="right">)" << item.fSize / 1024 << "k</td>";
                ss << "</tr>";
            }
            ss << R"(</table>
                    </body>
                    </html>)";
            res.body = ss.str();
            res.set_header("Content-Type", "text/html");
            res.status = 200;
            return;
        }
        static std::string getETag(const BackupInfo &info)
        {
            FileUtils fu(info.real_path);
            std::string etag = fu.getFilename();
            etag += "-";
            etag += std::to_string(info.fSize);
            etag += "-";
            etag += std::to_string(info.mtime);
            return etag;
        }
        static void download(const httplib::Request &req, httplib::Response &res)
        {
            // 获取客户端请求的资源路径path
            // 根据资源路径，获取文件备份信息
            BackupInfo info;
            data_->getOneByUrl(req.path, &info);
            // 判断文件是否被压缩，如果被压缩，要先解压缩
            if (info.pack_flag)
            {
                FileUtils fu(info.pack_path);
                fu.decompress(info.real_path); // 将文件解压到备份目录下
                // 删除压缩包，修改备份信息（已经灭有被压缩）
                fu.removeFile();
                info.pack_flag = false;
                data_->update(info);
            }

            // 读取文件数据，放入res.body中。
            FileUtils fuInfo(info.real_path);
            size_t file_size = info.fSize;

            bool partial = false;
            size_t start = 0, end = file_size - 1;
            // 1. 检查 Range 头
            if (req.has_header("Range"))
            {
                std::string range = req.get_header_value("Range"); // 例: "bytes=1000-2000"
                if (range.find("bytes=") == 0)
                {
                    std::string range_val = range.substr(6);
                    size_t dash = range_val.find('-');
                    if (dash != std::string::npos)
                    {
                        std::string start_str = range_val.substr(0, dash);
                        std::string end_str = range_val.substr(dash + 1);

                        if (!start_str.empty())
                            start = std::stoul(start_str);
                        if (!end_str.empty())
                            end = std::stoul(end_str);
                        if (end >= file_size)
                            end = file_size - 1;
                        if (start < file_size && start <= end)
                        {
                            partial = true;
                        }
                    }
                }
            }

            // 2. 读取文件
            std::string content;
            if (partial)
            {
                fuInfo.getContentRange(&content, start, end); // 需要你实现：只读部分数据
                res.status = 206;
                res.set_header("Content-Range",
                               "bytes " + std::to_string(start) + "-" +
                                   std::to_string(end) + "/" + std::to_string(file_size));
            }
            else
            {
                fuInfo.getContent(&content); // 全量读取
                res.status = 200;
            }

            res.set_content(content, "application/octet-stream");
            res.set_header("Accept-Ranges", "bytes");
            res.set_header("ETag", getETag(info));
            return;
        }

    private:
        int server_port_;
        std::string server_ip_;
        std::string downdoad_prefix_;
        httplib::Server svr_;
    };
}

#endif