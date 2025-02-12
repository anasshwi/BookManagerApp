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
	char lastChar = 'z';
	for (char c : input) {
		if (c == ' ') {
			if (!firstChar && !result.empty() && lastChar != '+') {
				result += '+';
				lastChar = '+';
			}
			

		}
		else
		{
			result += c;
			lastChar = c;
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

		std::string url = "/search.json?q=" + toSearch + "&fields=key,title,author_name,first_publish_year,cover_i";

		std::cout << url << "\n";

		auto res = cli.Get(url.c_str());
		if (res && res->status == 200)
		{
			auto json_result = nlohmann::json::parse(res->body);
			std::cout << json_result["num_found"] << "\n";
			//std::cout << json_result["docs"] << "\n";

			try {
				common.books.clear();

				//Prasing the data and inserting it in the vector Books in the common object.
				for (auto& item : json_result["docs"]) {
					Book book;
					if (item.contains("title") && item.contains("author_name") && item.contains("first_publish_year") && item.contains("key")) {
						book.title = item["title"];
						book.author_name = item["author_name"];
						book.first_publish_year = item["first_publish_year"];
						book.key = item["key"];
					
						if (item.contains("cover_i")) {
							book.coverID = item["cover_i"];
						}
						else {
							book.coverID = -1;
						}
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
				//std::cout << json_desc << "\n\n";
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
				//std::cout << json_rate << "\n\n\n";
				
				//Checking if rates exists in the database
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
void DownloadThread::getImage(CommonObjects& common)
{
	//Function that gets Image of selected books, works as a infinite while loop that waits for requests to search for that info.
	while (!common.exit_flag.load()) {
		std::unique_lock<std::mutex> lock(common.mtx);

		//Waits for the lock to be released and for a search request to be made.
		common.cv3.wait(lock, [&common] {return common.download_img.load() || common.exit_flag.load(); });

		if (common.exit_flag.load()) {
			break;
		}
		common.img_ready = false;
		try {
			//Getting the cover img
			if (common.books[common.Index].coverID != -1)
			{
				//First try getting the image:
				httplib::Client imgCli("https://covers.openlibrary.org");

				std::string cover_url = "/b/id/" + std::to_string(common.books[common.Index].coverID) + "-L.jpg";
				std::cout << cover_url << "\n\n";

				auto img = imgCli.Get(cover_url.c_str());
				if (img && img->status == 200)
				{
					//Image retreved succesfully:
					std::vector<unsigned char> imagedata(img->body.begin(), img->body.end());

					common.books[common.Index].image = imagedata;
					std::cout << "Image Downloaded succesfully!\n\n";
				}
				else if (img && (img->status == 301 || img->status == 302))
				{
					//URL got redirected once
					std::string new_url = img->get_header_value("Location");
					std::cout << "Redirected to: " << new_url << "\n\n";

					// Handle Redirection of image's Url
					std::string archive_prefix = "https://archive.org";
					if (new_url.find(archive_prefix) == 0)
					{
						new_url = new_url.substr(archive_prefix.length());
					}

					//std::cout << "New Url: " << new_url << "\n";
					httplib::Client reCli("https://archive.org");
					auto redirected = reCli.Get(new_url.c_str());

					//Trying the redirected Url to get the image
					if (redirected && redirected->status == 200)
					{
						std::vector<unsigned char> imagedata(redirected->body.begin(), redirected->body.end());

						common.books[common.Index].image = imagedata;
						std::cout << "Image Downloaded succesfully!\n\n";
					}
					else if (redirected && (redirected->status == 301 || redirected->status == 302))
					{
						//Got redirected again
						std::string redirectedAgain = redirected->get_header_value("Location");
						std::cout << "Redirected again to:" << redirectedAgain << "\n\n";

						// splitting the url to domain and path:
						size_t domain_end_pos = redirectedAgain.find('/', 8);  // 8 to skip "https://"

						//seperating the Url to base and path:
						if (domain_end_pos != std::string::npos)
						{
							//Extract the base url:
							std::string base_url = redirectedAgain.substr(0, domain_end_pos);

							//Extract the path:
							std::string path = redirectedAgain.substr(domain_end_pos);

							httplib::Client reReCli(base_url);
							auto newRequest = reReCli.Get(path);
							if (newRequest && newRequest->status == 200)
							{
								//Trying to get the image after redirected twice
								std::vector<unsigned char> imagedata(newRequest->body.begin(), newRequest->body.end());

								common.books[common.Index].image = imagedata;
								std::cout << "Image Downloaded succesfully!\n\n";
							}
							else {
								std::cout << "failed to download image from 2 Redirections..." << "Status:" << (newRequest ? newRequest->status : -1) << "\n\n";
							}
						}
						else
						{
							std::cout << "failed to seperate base and path...\n\n";
						}

					}
					else
					{
						std::cout << "failed to download image from Archives..." << "Status:" << (redirected ? redirected->status : -1) << "\n\n";
					}
				}
				else
				{
					std::cout << "failed to download image...\n\n";
				}
			}

		}
		catch (const nlohmann::json::exception& e) {
			std::cerr << "JSON Error: " << e.what() << std::endl;
		}

		//Releasing the lock after finishing the job.
		common.img_ready = true;
		common.download_img.store(false);
	}

}
