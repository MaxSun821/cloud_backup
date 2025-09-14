#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <mutex>
#include <string>
#include "util.hpp"

namespace cloud
{
    #define CONFIG_FILE "./cloud.conf"
    class Config
    {
    public:
        static Config *getInstance()
        {
            if (instance_ == nullptr)
            {
                mutex_.lock();
                if (instance_ == nullptr)
                {
                    instance_ = new Config();
                }
                mutex_.unlock();
            }
            return instance_;
        }
        int getHotTime()
        {
            return hot_time_;
        }
        int getServerPort()
        {
            return server_port_;
        }
        std::string getServerIp()
        {
            return server_ip_;
        }
        std::string getDownloadPrefix()
        {
            return download_prefix_;
        }
        std::string getPackfileSuffix()
        {
            return packfile_suffix_;
        }
        std::string getPackDir()
        {
            return pack_dir_;
        }
        std::string getBackDir()
        {
            return back_dir_;
        }
        std::string getBackupFile()
        {
            return backup_file_;
        }

    private:
        Config() {readConfigFile();}
        Config(const Config &config) = delete;
        Config &operator=(const Config &config) = delete;
        ~Config() = default;
        static Config *instance_;
        static std::mutex mutex_;

    private:
        int hot_time_;
        int server_port_;
        std::string server_ip_;
        std::string download_prefix_;
        std::string packfile_suffix_;
        std::string pack_dir_;
        std::string back_dir_;
        std::string backup_file_;
        bool readConfigFile()
        {
            FileUtils fu(CONFIG_FILE);
            std::string body;
            if (fu.getContent(&body) == false)
            {
                std::cerr << "WARNING: read config file failed" << std::endl;
                return false;
            }
            Json::Value root;
            if (JsonUtils::deserialization(body, root) == false)
            {
                std::cerr << "ERROR: parse failed" << std::endl;
                return false;
            }
            hot_time_ = root["hot_time"].asInt();
            server_port_ = root["server_port"].asInt();
            server_ip_ = root["server_ip"].asString();
            download_prefix_ = root["download_prefix"].asString();
            packfile_suffix_ = root["packfile_suffix"].asString();
            pack_dir_ = root["pack_dir"].asString();
            back_dir_ = root["back_dir"].asString();
            backup_file_ = root["backup_file"].asString();
            return true;
        }
    };
    Config *Config::instance_ = nullptr;
    std::mutex Config::mutex_;
}
#endif