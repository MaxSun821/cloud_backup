#ifndef __DATA_HPP__
#define __DATA_HPP__

#include <unordered_map>
#include <pthread.h>
#include "util.hpp"
#include "config.hpp"

namespace cloud
{
    struct BackupInfo
    {
        bool pack_flag;
        size_t fSize;
        time_t mtime;
        time_t atime;
        std::string real_path;
        std::string pack_path;
        std::string url;

        bool newBackupInfo(const std::string &realpath)
        {
            FileUtils fu(realpath);
            if (fu.exists() == false)
            {
                std::cerr << "FATAL: file not exists" << std::endl;
                return false;
            }
            Config *config = Config::getInstance();
            std::string pack_dir = config->getPackDir();
            std::string pack_suffix = config->getPackfileSuffix();
            std::string download_prefix = config->getDownloadPrefix();
            
            this->pack_flag = false;
            this->fSize = fu.fileSize();
            this->mtime = fu.getLastMTime();
            this->atime = fu.getLastATime();
            this->real_path = realpath;
            this->pack_path = pack_dir + fu.getFilename() + pack_suffix;
            this->url = download_prefix + fu.getFilename();
            return true;
        }
    };

    class DataManager
    {
    public:
        DataManager()
        {
            backup_file_ = Config::getInstance()->getBackupFile();
            pthread_rwlock_init(&rwlock_, nullptr);
            initLoad();
        }
        ~DataManager()
        {
            pthread_rwlock_destroy(&rwlock_);
        }
        bool insert(const BackupInfo &info)
        {
            pthread_rwlock_wrlock(&rwlock_);
            table_[info.url] = info;
            pthread_rwlock_unlock(&rwlock_);
            storage();
            return true;
        }
        bool update(const BackupInfo &info)
        {
            pthread_rwlock_wrlock(&rwlock_);
            table_[info.url] = info;
            pthread_rwlock_unlock(&rwlock_);
            storage();
            return true;
        }
        bool getOneByUrl(const std::string &url, BackupInfo *info)
        {
            pthread_rwlock_wrlock(&rwlock_);
            // url为key值，可直接查找
            auto it = table_.find(url);
            if (it == table_.end())
            {
                pthread_rwlock_unlock(&rwlock_);
                return false;
            }
            *info = it->second;
            pthread_rwlock_unlock(&rwlock_);
            return true;
        }
        bool getOneByRealPath(const std::string &realpath, BackupInfo *info)
        {
            pthread_rwlock_wrlock(&rwlock_);
            for (auto item : table_)
            {
                if (item.second.real_path == realpath)
                {
                    *info = item.second;
                    pthread_rwlock_unlock(&rwlock_);
                    return true;
                }
            }
            pthread_rwlock_unlock(&rwlock_);
            return false;
        }
        bool getAll(std::vector<BackupInfo> *array)
        {
            pthread_rwlock_wrlock(&rwlock_);
            for (auto item : table_)
            {
                array->push_back(item.second);
            }
            pthread_rwlock_unlock(&rwlock_);
            return true;
        }

        bool storage()
        {
            // 获取所有数据
            std::vector<BackupInfo> array;
            this->getAll(&array);
            // 添加到Json::Value
            Json::Value root;
            for (int i = 0; i < array.size(); i++)
            {
                Json::Value item;
                item["pack_flag"] = array[i].pack_flag;
                item["fSize"] = static_cast<Json::Int64>(array[i].fSize);
                item["mtime"] = static_cast<Json::Int64>(array[i].mtime);
                item["atime"] = static_cast<Json::Int64>(array[i].atime);
                item["real_path"] = array[i].real_path;
                item["pack_path"] = array[i].pack_path;
                item["url"] = array[i].url;
                root.append(item); // 添加数组元素
            }
            // 对Json::Value序列化
            std::string body;
            JsonUtils::serialization(root, &body);

            // 写文件
            FileUtils fu(backup_file_);
            fu.setContent(body);
            return true;
        }

        bool initLoad()
        {
            // 将文件中的数据读取出来
            FileUtils fu(backup_file_);
            if (fu.exists() == false)
            {
                return true;
            }
            std::string body;
            fu.getContent(&body);

            // 反序列化
            Json::Value root;
            JsonUtils::deserialization(body, root);

            // 添加到table_中
            for (int i = 0; i < root.size(); i++)
            {
                BackupInfo info;
                info.pack_flag = root[i]["pack_flag"].asBool();
                info.fSize = root[i]["fSize"].asInt64();
                info.mtime = root[i]["mtime"].asInt64();
                info.atime = root[i]["atime"].asInt64();
                info.real_path = root[i]["real_path"].asString();
                info.pack_path = root[i]["pack_path"].asString();
                info.url = root[i]["url"].asString();
                insert(info);
            }
            return true;
        }
    private:
        pthread_rwlock_t rwlock_;
        std::unordered_map<std::string, BackupInfo> table_;
        std::string backup_file_;
    };
}

#endif