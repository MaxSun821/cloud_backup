#ifndef __CLOUD_HPP__
#define __CLOUD_HPP__

#include "data.hpp"
#include "httplib.h"
#include <windows.h>

namespace cloud
{
#define SERVER_ADDR "106.15.37.17"
#define SERVER_PORT 8080
	class Backup
	{
	public:
		Backup(const std::string& back_dir, const std::string& back_file)
			: back_dir_(back_dir)
		{
			data_ = new DataManager(back_file);
		}
		std::string getFileIdentifier(const std::string& filename)
		{
			FileUtils fu(filename);
			std::stringstream ss;
			ss << fu.getFilename() << "-" << fu.fileSize() << "-" << fu.getLastMTime();
			return ss.str();
		}
		bool upload(const std::string &filename)
		{
			// 获取文件数据
			FileUtils fu(filename);
			std::string body;
			fu.getContent(&body);
			// 搭建http客户端上传文件数据
			httplib::Client client(SERVER_ADDR, SERVER_PORT);
			httplib::UploadFormData item;
			item.content = body;
			item.filename = fu.getFilename();
			item.name = "file";
			item.content_type = "application/octet-stream";

			httplib::UploadFormDataItems items;
			items.push_back(item);

			auto res = client.Post("/upload", items);
			if (!res || res->status != 200)
			{
				return false;
			}
			return true;
		}
		bool isNeedUpload(const std::string& filename)
		{
			// 需要上传的文件的判断条件：文件是新增的；不是新增的但被修改过
			// 文件新增：看一下有没有历史备份信息
			std::string id;
			if (data_->getOneByKey(filename, &id) != false)
			{
				// 不是新增的：有历史信息，但唯一标识不一致
				// 有历史信息
				std::string new_id = getFileIdentifier(filename);
				if (new_id == id)
				{
					return false; // 不需要被上传
				}
			}
			// 一个文件比较大，正在拷贝到目录下，拷贝需要一个过程
			// 如果每次便利则都会判断标识不一致
			// 因此应该判断一个文件一段时间都没有被修改过，则才能上传
			FileUtils fu(filename);
			if (time(nullptr) - fu.getLastMTime() < 3)
			{
				// 3秒没有被修改过
				return false;
			}
			std::cout << filename << " need upload success" << std::endl;
			return true;
		}
		bool runModule()
		{
			while (true)
			{
				FileUtils fu(back_dir_);
				std::vector<std::string> array;
				fu.scanDir(&array);
				for (auto& item : array)
				{
					if (isNeedUpload(item) == false)
					{
						continue;
					}
					if (upload(item) == true)
					{
						std::string id = getFileIdentifier(item);
						data_->insert(item, id);
						std::cout << item << " upload success" << std::endl;
					}
				}
				Sleep(1);
			}
		}
	private:
		std::string back_dir_;
		DataManager* data_;
	};
}
#endif
