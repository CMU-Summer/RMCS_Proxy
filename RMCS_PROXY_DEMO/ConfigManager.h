#pragma once
#ifndef ConfigManager_H
#define ConfigManager_H

#include <iostream>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <map>
#include "common.h"
using namespace std;
using namespace boost::property_tree;

class ConfigManager {
private:
	vector<GroupStruct> groupList;
	vector<RedisCofig> redisList;
	vector<DBconfig> dbList;
	vector<ServerConfig> serverList;
	int sleep_time;
	int feedBackFrequency;
public:
	ConfigManager();
	ConfigManager(string filePath);
	
	~ConfigManager();
	vector<GroupStruct> getGroupList();
	vector<RedisCofig> getRedisList();
	vector<DBconfig> getDbConfig();
	vector<ServerConfig> getServersConfig();
	int getSleepTime();
	int getFeedbackFrequency();
};

#endif // !ConfigManager_H
