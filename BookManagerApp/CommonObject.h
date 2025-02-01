#pragma once
#include <atomic>
#include <string>
#include <vector>

struct Book
{
	std::string title;
	std::vector<std::string> author_name; // Handle multiple authors
};

struct CommonObjects
{
	std::atomic_bool exit_flag = false;
	std::atomic_bool start_download = false;
	std::atomic_bool data_ready = false;
	std::string searchBar = "1";
	std::vector<Book> books;
};

