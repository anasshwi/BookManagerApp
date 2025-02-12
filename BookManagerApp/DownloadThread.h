#pragma once
#include "CommonObject.h"

class DownloadThread
{
public:
	void operator()(CommonObjects& common);
	void getDescInfoDesc(CommonObjects& common);
	void getImage(CommonObjects& common);
private:
	std::string _download_url;
};

