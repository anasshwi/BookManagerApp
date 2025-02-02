#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Book, title, author_name, language, first_publish_year, key);

void DownloadThread::operator()(CommonObjects& common)
{
	httplib::Client cli("https://openlibrary.org");

	std::string url = "/search.json?q="+ common.searchBar+"&mode=everything&limit=200";

	auto res = cli.Get(url.c_str());
	if (res->status == 200)
	{
		auto json_result = nlohmann::json::parse(res->body);
		
		/*for (auto& item : json_result["docs"]) {
			Book book;
			book.title = item["title"];
			book.author = item["author_name"].is_array() ? item["author_name"][0] : "";
			common.books.push_back(book);
		}*/

		common.books = json_result["docs"].get<std::vector<Book>>();


		if (common.books.size() > 0)
			common.data_ready = true;
	}
}

void DownloadThread::SetUrl(std::string_view new_url)
{
	_download_url = new_url;
}
