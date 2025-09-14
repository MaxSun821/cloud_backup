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
			// ��ȡ�ļ�����
			FileUtils fu(filename);
			std::string body;
			fu.getContent(&body);
			// �http�ͻ����ϴ��ļ�����
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
			// ��Ҫ�ϴ����ļ����ж��������ļ��������ģ����������ĵ����޸Ĺ�
			// �ļ���������һ����û����ʷ������Ϣ
			std::string id;
			if (data_->getOneByKey(filename, &id) != false)
			{
				// ���������ģ�����ʷ��Ϣ����Ψһ��ʶ��һ��
				// ����ʷ��Ϣ
				std::string new_id = getFileIdentifier(filename);
				if (new_id == id)
				{
					return false; // ����Ҫ���ϴ�
				}
			}
			// һ���ļ��Ƚϴ����ڿ�����Ŀ¼�£�������Ҫһ������
			// ���ÿ�α����򶼻��жϱ�ʶ��һ��
			// ���Ӧ���ж�һ���ļ�һ��ʱ�䶼û�б��޸Ĺ���������ϴ�
			FileUtils fu(filename);
			if (time(nullptr) - fu.getLastMTime() < 3)
			{
				// 3��û�б��޸Ĺ�
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
