#include <thread>
#include "util.hpp"
#include "config.hpp"
#include "data.hpp"
#include "hot.hpp"
#include "service.hpp"

void fileUtilsTest(const std::string &filename)
{
    // cloud::FileUtils fu(filename);
    // std::cout << fu.fileSize() << std::endl;
    // std::cout << fu.getFilename() << std::endl;
    // std::cout << fu.getLastATime() << std::endl;
    // std::cout << fu.getLastMTime() << std::endl;

    // cloud::FileUtils fu(filename);
    // std::string body;
    // fu.getContent(&body);

    // cloud::FileUtils nfu("./hello.txt");
    // nfu.setContent(body);

    // std::string packname = filename + ".gz";
    // cloud::FileUtils fu(filename);
    // fu.compress(packname);

    // cloud::FileUtils pfu(packname);
    // pfu.decompress("./dec.txt");

    cloud::FileUtils fu(filename);
    fu.createDir();
    std::vector<std::string> array;
    fu.scanDir(&array);
    for (auto &item : array)
    {
        std::cout << item << std::endl;
    }
    return;
}

void JsonUtilsTest()
{
    const char *name = "小明";
    int age = 19;
    float scores[] = {92, 34, 56};
    Json::Value root;
    root["姓名"] = name;
    root["年龄"] = age;
    root["成绩"].append(scores[0]);
    root["成绩"].append(scores[1]);
    root["成绩"].append(scores[2]);
    std::string json_str;
    cloud::JsonUtils::serialization(root, &json_str);
    std::cout << json_str << std::endl;

    Json::Value newRoot;
    cloud::JsonUtils::deserialization(json_str, newRoot);
    std::cout << newRoot["姓名"].asString() << std::endl;
    std::cout << newRoot["年龄"].asInt() << std::endl;
    for (int i = 0; i < newRoot["成绩"].size(); i++)
    {
        std::cout << newRoot["成绩"][i].asFloat() << std::endl;
    }
}

void ConfigTest()
{
    cloud::Config *config = cloud::Config::getInstance();
    std::cout << config->getHotTime() << std::endl;
    std::cout << config->getServerPort() << std::endl;
    std::cout << config->getServerIp() << std::endl;
    std::cout << config->getDownloadPrefix() << std::endl;
    std::cout << config->getPackfileSuffix() << std::endl;
    std::cout << config->getPackDir() << std::endl;
    std::cout << config->getBackDir() << std::endl;
    std::cout << config->getBackupFile() << std::endl;

}

void DataTest(const std::string &filename)
{
    cloud::DataManager data;
    std::vector<cloud::BackupInfo> array;
    data.getAll(&array);
    for (auto &i : array)
    {
        std::cout << i.fSize << std::endl;
        std::cout << i.atime << std::endl;
        std::cout << i.mtime << std::endl;
        std::cout << i.pack_flag << std::endl;
        std::cout << i.pack_path << std::endl;
        std::cout << i.real_path << std::endl;
        std::cout << i.url << std::endl;
    }
    // cloud::BackupInfo info;
    // info.newBackupInfo(filename);
    // cloud::DataManager data;
    // data.insert(info);

    // cloud::BackupInfo tmp;
    // data.getOneByUrl("/download/data.hpp", &tmp);

    // std::cout << tmp.fSize << std::endl;
    // std::cout << tmp.atime << std::endl;
    // std::cout << tmp.mtime << std::endl;
    // std::cout << tmp.pack_flag << std::endl;
    // std::cout << tmp.pack_path << std::endl;
    // std::cout << tmp.real_path << std::endl;
    // std::cout << tmp.url << std::endl;

    // std::cout << "--------------------" << std::endl;

    // info.pack_flag = true;
    // data.update(info);

    // std::vector<cloud::BackupInfo> array;
    // data.getAll(&array);

    // for (auto &i : array)
    // {
    //     std::cout << i.fSize << std::endl;
    //     std::cout << i.atime << std::endl;
    //     std::cout << i.mtime << std::endl;
    //     std::cout << i.pack_flag << std::endl;
    //     std::cout << i.pack_path << std::endl;
    //     std::cout << i.real_path << std::endl;
    //     std::cout << i.url << std::endl;
    // }

    // std::cout << "--------------------" << std::endl;

    // data.getOneByRealPath(filename, &tmp);
    // std::cout << tmp.fSize << std::endl;
    // std::cout << tmp.atime << std::endl;
    // std::cout << tmp.mtime << std::endl;
    // std::cout << tmp.pack_flag << std::endl;
    // std::cout << tmp.pack_path << std::endl;
    // std::cout << tmp.real_path << std::endl;
    // std::cout << tmp.url << std::endl;
}

cloud::DataManager *data_;

void HotTest()
{
    data_ = new cloud::DataManager();
    cloud::HotManager hot;
    hot.runModule();
}

void ServiceTest()
{
    data_ = new cloud::DataManager();
    cloud::Service srv;
    srv.runModule();
    std::cout << "exit" << std::endl;
}

int main(int argc, char *argv[])
{
    // fileUtilsTest(argv[1]);
    // JsonUtilsTest();
    // ConfigTest();
    // DataTest(argv[1]);
    // HotTest();
    // ServiceTest();

    std::thread thread_hot_manager(HotTest);
    std::thread thread_service(ServiceTest);
    thread_hot_manager.join();
    thread_service.join();
    return 0;
}