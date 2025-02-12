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
	int coverID;					//Id to get the cover image later
	std::string description;				//Summary from work api
	double rating = 0.0;					//Average rating
	std::string personal_note;				//A string to save a note
	std::vector<unsigned char> image;

	bool operator==(const Book& other) const {
		return (title == other.title) && (first_publish_year == other.first_publish_year) && (author_name == other.author_name);
	}
};

struct CommonObjects
{
	size_t Index;									//Index for Books vector.
	size_t FavIndex;								//Index for FavBooks vector.
	std::mutex mtx;
	std::condition_variable cv, cv2, cv3;			//Variables to signal the Threads.
	std::atomic_bool exit_flag = false;				//Signal exit.
	std::atomic_bool start_download = false;		//Signal library Search
	std::atomic_bool data_ready = false;			//Signal data draw on UI.
	std::atomic_bool info_ready = false;			//Signal extra info to draw on UI.
	std::atomic_bool download_info_desc = false;	//Signal extra info Search.
	std::atomic_bool img_ready = false;
	std::atomic_bool download_img = false;
	std::string searchBar = "";						//Search field.
	std::vector<Book> books;						//Vector of Books in library.
	std::vector<Book> FavBooks;						//Vector of saved Books.
};

