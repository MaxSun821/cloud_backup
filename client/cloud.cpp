#include "util.hpp"
#include "data.hpp"
#include "cloud.hpp"

#define BACKUP_FILE "./backup.dat"
#define BACKUP_DIR "./backup/"

int main()
{
	/*cloud::FileUtils fu("./");
	std::vector<std::string> array;
	fu.scanDir(&array);
	cloud::DataManager data(BACKUP_FILE);
	for (auto& item : array)
	{
		data.insert(item, "skjwiehigik");
	}*/

	/*cloud::DataManager data(BACKUP_FILE);
	std::string str;
	data.getOneByKey(".\\cloud.cpp", &str);
	std::cout << str << std::endl;*/

	cloud::Backup backup(BACKUP_DIR, BACKUP_FILE);
	backup.runModule();
	return 0;
}