#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"

std::string trim(const std::string& str) {
	auto start = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		});

	auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base();

	return (start < end) ? std::string(start, end) : "";
}

std::string formatQuery(const std::string& input) {
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Book, title, author_name, language, first_publish_year, key);

void DownloadThread::operator()(CommonObjects& common)
{
	while(true){
		std::unique_lock<std::mutex> lock(common.mtx);

		common.cv.wait(lock, [&common] {return common.start_download.load();});

		common.data_ready = false;

		httplib::Client cli("https://openlibrary.org");

		std::string toSearch = trim(common.searchBar);
		toSearch = formatQuery(toSearch);

		std::string url = "/search.json?q=" + toSearch + "&mode=everything";

		std::cout << url << "\n";

		auto res = cli.Get(url.c_str());
		if (res && res->status == 200)
		{
		auto json_result = nlohmann::json::parse(res->body);
		std::cout << json_result["numFound"] << "\n";

		try {
			common.books.clear();
			//common.books = json_result["docs"].get<std::vector<Book>>();

			for (auto& item : json_result["docs"]) {
				Book book;
				if (item.contains("title") && item.contains("author_name") && item.contains("language") && item.contains("first_publish_year") && item.contains("key")) {
					book.title = item["title"];
					book.author_name = item["author_name"];
					book.language = item["language"];
					book.first_publish_year = item["first_publish_year"];
					book.key = item["key"];
					common.books.push_back(book);
				}
			}
		}
		catch (const nlohmann::json::exception& e) {
			std::cerr << "JSON Error: " << e.what() << std::endl;
		}


		if (common.books.size() > 0)
		common.data_ready = true;
		}
		common.start_download.store(false);
	}
}

void DownloadThread::SetUrl(std::string_view new_url)
{
	_download_url = new_url;
}
