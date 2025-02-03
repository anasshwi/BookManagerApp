#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <thread>

std::string trim(const std::string& str) 
{
	//A helper Function that trims the edges from a string.
	auto start = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		});

	auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base();

	return (start < end) ? std::string(start, end) : "";
}

std::string formatQuery(const std::string& input) 
{
	//A helper Function that remove spaces from words and replace with "+".
	std::string result;
	bool firstChar = true;
	for (char c : input) {
		if (c == ' ') {
			if (!firstChar && !result.empty()) {
				result += '+';
			}

		}
		else
		{
			result += c;
			firstChar = false;
		}
	}
	return result;
}

//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Book, title, author_name, language, first_publish_year, key);

void DownloadThread::operator()(CommonObjects& common)
{
	//Function that download the search query and waits for each search request in a loop thread.
	while(!common.exit_flag.load()){
		std::unique_lock<std::mutex> lock(common.mtx);

		//If lock isnt released wait until it is.
		common.cv.wait(lock, [&common] {return common.start_download.load() || common.exit_flag.load(); });

		if (common.exit_flag.load()) { break; }

		common.data_ready = false;

		httplib::Client cli("https://openlibrary.org");

		// Using the helper function to make the query for the search....
		std::string toSearch = trim(common.searchBar);
		toSearch = formatQuery(toSearch);

		std::string url = "/search.json?q=" + toSearch + "&fields=key,title,author_name,first_publish_year";

		std::cout << url << "\n";

		auto res = cli.Get(url.c_str());
		if (res && res->status == 200)
		{
			auto json_result = nlohmann::json::parse(res->body);
			std::cout << json_result["num_found"] << "\n";

			try {
				common.books.clear();

				//Prasing the data and inserting it in the vector Books in the common object.
				for (auto& item : json_result["docs"]) {
					Book book;
					if (item.contains("title") && item.contains("author_name") && item.contains("first_publish_year") && item.contains("key")) {
						book.title = item["title"];
						book.author_name = item["author_name"];
						//book.language = item["language"];
						book.first_publish_year = item["first_publish_year"];
						book.key = item["key"];
						common.books.push_back(book);
					
					}
				}
			}
			catch (const nlohmann::json::exception& e) {
				std::cerr << "JSON Error: " << e.what() << std::endl;
			}

			if (common.books.size() > 0) {
				common.data_ready = true;
			}
		}
		//releasing the lock after finishing the search
		common.start_download.store(false);
		common.cv.notify_all();
	}
}

void DownloadThread::getDescInfoDesc(CommonObjects& common) 
{
	//Function that gets Description and rating of selected books, works as a infinite while loop that waits for requests to search for that info.
	while (!common.exit_flag.load()) {
		std::unique_lock<std::mutex> lock(common.mtx);

		//Waits for the lock to be released and for a search request to be made.
		common.cv2.wait(lock, [&common] {return common.download_info_desc.load() || common.exit_flag.load(); });

		if (common.exit_flag.load()) {
			break;
		}
		//getting more info:
		
		common.info_ready = false;
		httplib::Client cli("https://openlibrary.org");
		std::string workUrl = common.books[common.Index].key + ".json?fields=title,description";
		try {
			//Getting Description:
			auto desc = cli.Get(workUrl.c_str());
			if (desc && desc->status == 200)
			{
				auto json_desc = nlohmann::json::parse(desc->body);
				if (json_desc.contains("description"))
				{
					if (json_desc["description"].is_array())
					{
						common.books[common.Index].description = "";
						for (const auto& item : json_desc["description"]) {
							common.books[common.Index].description += (" - " + std::string(item) + "\n");
						}
					}
					else if (json_desc["description"].is_object() && json_desc["description"].contains("value")) {
						common.books[common.Index].description = json_desc["description"]["value"];
					}
					else if (json_desc["description"].is_string()) {
						common.books[common.Index].description = json_desc["description"];
					}
					else {
						common.books[common.Index].description = "No Known Data\n";
					}

					if ((common.books[common.Index].description).empty() || std::all_of((common.books[common.Index].description).begin(), (common.books[common.Index].description).end(), isspace)) {
						common.books[common.Index].description = "No Known Data\n";
					}

				}
				else
				{
					common.books[common.Index].description = "No Known Data\n";
				}
			}
			else
			{
				common.books[common.Index].description = "No Known Data\n";
			}

			//Getting Rating:
			std::string ratingUrl = common.books[common.Index].key + "/ratings.json";
			auto rate = cli.Get(ratingUrl.c_str());
			if (rate && rate->status == 200) {
				auto json_rate = nlohmann::json::parse(rate->body);
				if (json_rate.contains("summary")) {
					if (json_rate["summary"].contains("average")) {
						if (json_rate["summary"]["average"].is_number()) {
							common.books[common.Index].rating = json_rate["summary"]["average"];
						}
					}
				}
			}

		}
		catch (const nlohmann::json::exception& e) {
			std::cerr << "JSON Error: " << e.what() << std::endl;
		}
		
		//Releasing the lock after finishing the job.
		common.info_ready = true;
		common.download_info_desc.store(false);
	}

}
