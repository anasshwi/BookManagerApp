#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

struct Book
{
	std::string title;
	std::vector<std::string> author_name;	// Handle multiple authors
	int first_publish_year = 0;
	std::string key;						//Work key ("/work/##########")
	//std::string cover_id = "";				//Cover Image ID
	std::string description;			//Summary from work api
	double rating = 0.0;					//Average rating

	bool operator==(const Book& other) const {
		return (title == other.title) && (first_publish_year == other.first_publish_year) && (author_name == other.author_name);
	}
};

struct CommonObjects
{
	size_t Index;
	size_t FavIndex;
	std::mutex mtx;
	std::condition_variable cv, cv2;
	std::atomic_bool exit_flag = false;
	std::atomic_bool start_download = false;
	std::atomic_bool data_ready = false;
	std::atomic_bool info_ready = false;
	std::atomic_bool download_info_desc = false;
	std::string searchBar = "";
	std::vector<Book> books;
	std::vector<Book> FavBooks;
};

