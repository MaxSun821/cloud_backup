#ifndef __DATA_HPP__
#define __DATA_HPP__

#include <unordered_map>
#include "util.hpp"

namespace cloud
{
	class DataManager
	{
	public:
		DataManager(const std::string& backup_file) : backup_file_(backup_file)
		{
			initLoad();
		}
		bool storage()
		{
			std::stringstream ss;
			// 获取所有的备份信息
			for (auto& item : table_)
			{
				// 将所有的信息进行指定持久化格式的组织
				ss << item.first << " " << item.second << "\n";
			}
			
			// 持久化存储
			FileUtils fu(backup_file_);
			fu.setContent(ss.str());
			return true;
		}
		bool initLoad()
		{
			// 从文件中读取所有数据
			FileUtils fu(backup_file_);
			std::string body;
			fu.getContent(&body);
			// 进行数据解析，添加到表中
			std::vector<std::string> array;
			Splite(body, "\n", &array);
			for (auto& item : array)
			{
				std::vector<std::string> ret;
				Splite(item, " ", &ret);
				if (ret.size() != 2)
				{
					continue;
				}
				table_[ret[0]] = ret[1];
			}
			return true;
		}
		bool insert(const std::string& key, const std::string& val)
		{
			table_[key] = val;
			storage();
			return true;
		}
		bool update(const std::string& key, const std::string& val)
		{
			table_[key] = val;
			storage();
			return true;
		}
		bool getOneByKey(const std::string& key, std::string* val)
		{
			auto it = table_.find(key);
			if (it == table_.end())
			{
				return false;
			}
			*val = it->second;
			return true;
		}
	private:
		int Splite(const std::string& str, const std::string& sep, std::vector<std::string>* array)
		{
			int count = 0;
			size_t pos = 0, idx = 0;
			while (true)
			{
				pos = str.find(sep, idx);
				if (pos == std::string::npos)
				{
					break;
				}
				if (pos == idx)
				{
					idx = pos + sep.size();
					continue;
				}
				std::string temp = str.substr(idx, pos - idx);
				array->push_back(temp);
				count++;
				idx = pos + sep.size();
			}
			if (idx < str.size())
			{
				array->push_back(str.substr(idx));
				count++;
			}
			return count;
		}
	private:
		std::string backup_file_; // 备份信息的持久化存储文件
		std::unordered_map<std::string, std::string> table_;
	};
}

#endif
