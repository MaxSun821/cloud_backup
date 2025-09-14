#ifndef __HOT_HPP__
#define __HOT_HPP__

#include "data.hpp"

extern cloud::DataManager *data_;

namespace cloud
{
    class HotManager
    {
    public:
        HotManager()
        {
            Config *config = Config::getInstance();
            back_dir_ = config->getBackDir();
            pack_dir_ = config->getPackDir();
            pack_suffix_ = config->getPackfileSuffix();
            hot_time_ = config->getHotTime();

            FileUtils back(back_dir_);
            FileUtils pack(pack_dir_);

            back.createDir();
            pack.createDir();
        }
        bool runModule()
        {
            while (true)
            {
                // 便利备份目录，获取所有文件名
                FileUtils fu(back_dir_);
                std::vector<std::string> array;
                fu.scanDir(&array);
                // 便利判断文件是否是非热点文件
                for (auto &item : array)
                {
                    if (hotJudge(item))
                    {
                        continue;
                    }
                    // 获取文件的备份信息
                    BackupInfo info;
                    if (data_->getOneByRealPath(item, &info) == false)
                    {
                        // 文件存在，但是没有备份信息
                        info.newBackupInfo(item); // 设置一个新的备份信息出来
                    }
                    // 对非热点文件进行压缩处理
                    FileUtils nHT(item);
                    nHT.compress(info.pack_path);

                    // 删除源文件，修改备份信息
                    nHT.removeFile();
                    info.pack_flag = true;
                    data_->update(info);
                }
                usleep(1000); // 避免空目录循环遍历，消耗cpu太高
            }
            return true;
        }
    private:
        // 热点文件返回true，非热点文件返回false
        bool hotJudge(const std::string &filename)
        {
            FileUtils fu(filename);
            time_t last_atime = fu.getLastATime();
            time_t cur_time = time(nullptr);
            if (cur_time - last_atime < hot_time_)
            {
                return true;
            }
            return false;
        }
    private:
        std::string back_dir_;
        std::string pack_dir_;
        std::string pack_suffix_;
        time_t hot_time_;
    };
}


#endif