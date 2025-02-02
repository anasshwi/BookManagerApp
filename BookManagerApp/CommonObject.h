#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

struct Book
{
	std::string title;
	std::vector<std::string> author_name; // Handle multiple authors
	std::vector<std::string> language;
	int first_publish_year = 0;
	std::string key;

	bool operator==(const Book& other) const {
		return (title == other.title) && (first_publish_year == other.first_publish_year) && (author_name == other.author_name);
	}
};

struct CommonObjects
{
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic_bool exit_flag = false;
	std::atomic_bool start_download = false;
	std::atomic_bool data_ready = false;
	std::string searchBar = "jane";
	std::vector<Book> books;
	std::vector<Book> FavBooks;
};

