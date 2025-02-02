#include <iostream>
#include "DrawThread.h"
#include "GuiMain.h"
#include <vector>
#include <algorithm>
#include "../../shared/ImGuiSrc/imgui.h"

bool show_fav_list = false;
void DrawAppWindow(void* common_ptr)
{
	auto common = (CommonObjects*)common_ptr;
	ImGui::Begin("Book Management App");
	ImGui::Checkbox("Favorites List", &show_fav_list);
	ImGui::Text("Search Bar		");
	static char buff[256];
	ImGui::SameLine();
	ImGui::InputText( " ", buff, sizeof(buff));
	ImGui::SameLine();
	if (ImGui::Button("Search Books")) {
		
		std::lock_guard<std::mutex> lock(common->mtx);
		common->searchBar = buff;
		
		common->start_download.store(true);
		common->cv.notify_all();
	}
	if (common->data_ready)
	{
		if (ImGui::BeginTable("Book List", 5))  //, flags))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("Author");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			if (!common->books.empty()) {
				for (size_t i = 0; i < common->books.size(); ++i) {
					const Book& book = common->books[i];

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(book.title.c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(book.author_name[0].c_str());
					ImGui::TableSetColumnIndex(3);

					std::string popupID = "Book Details##" + std::to_string(i);
					if (ImGui::Button(("Show more details ## " + std::to_string(i)).c_str()))
					{
						//show details
						ImGui::OpenPopup(popupID.c_str());
					}
					if (ImGui::BeginPopup(popupID.c_str())) {
						ImGui::Text("Title: %s", book.title.c_str());

						std::string authors = book.author_name.empty() ? "Unknown" : book.author_name[0];
						for (size_t i = 1; i < book.author_name.size(); ++i) {
							authors += ", " + book.author_name[i];
						}
						ImGui::Text("Authors: %s", authors.c_str());

						ImGui::Text("Publish Year: %d", book.first_publish_year);

						std::string languages = book.language.empty() ? "Unknown" : book.language[0];
						for (size_t i = 1; i < book.language.size(); ++i) {
							languages += ", " + book.language[i];
						}
						ImGui::Text("Languages: %s", languages.c_str());

						if (ImGui::Button("Close")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					ImGui::TableSetColumnIndex(4);
					if (std::find(common->FavBooks.begin(), common->FavBooks.end(), (Book)book) != common->FavBooks.end()) {
						ImGui::Text("Added to Favorites");
					}
					else {
						if (ImGui::Button(("Add to Favorites ## " + std::to_string(i)).c_str()))
						{
							//show details
							common->FavBooks.push_back(book);
						}
					}

				}
			}
			else {
				ImGui::Text("No Results Found! Search again.");
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();

	if (show_fav_list)
	{
		ImGui::Begin("Favorites Window", &show_fav_list);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Here Lies all the Favorite Books that you Selected!");
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			show_fav_list = false;
		}

		if (ImGui::BeginTable("Book List", 5))  //, flags))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("Author");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			for (size_t i = 0; i < common->FavBooks.size(); ++i) {
				const Book& book = common->FavBooks[i];

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(book.title.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(book.author_name[0].c_str());
				ImGui::TableSetColumnIndex(3);

				std::string popupID = "Book Details##" + std::to_string(i);
				if (ImGui::Button(("Show more details ## " + std::to_string(i)).c_str()))
				{
					//show details
					ImGui::OpenPopup(popupID.c_str());
				}
				if (ImGui::BeginPopup(popupID.c_str())) {
					ImGui::Text("Title: %s", book.title.c_str());

					std::string authors = book.author_name.empty() ? "Unknown" : book.author_name[0];
					for (size_t i = 1; i < book.author_name.size(); ++i) {
						authors += ", " + book.author_name[i];
					}
					ImGui::Text("Authors: %s", authors.c_str());

					ImGui::Text("Publish Year: %d", book.first_publish_year);

					std::string languages = book.language.empty() ? "Unknown" : book.language[0];
					for (size_t i = 1; i < book.language.size(); ++i) {
						languages += ", " + book.language[i];
					}
					ImGui::Text("Languages: %s", languages.c_str());

					if (ImGui::Button("Close")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::TableSetColumnIndex(4);
				
				if (ImGui::Button(("Remove Book ## " + std::to_string(i)).c_str()))
				{
					auto it = std::remove(common->FavBooks.begin(), common->FavBooks.end(), book);
					if (it != common->FavBooks.end()) {
						// Erase the element(s) from the vector
						common->FavBooks.erase(it, common->FavBooks.end());
					}
				}
				

			}
			ImGui::EndTable();
		}
		

		ImGui::End();
	}


}

void DrawThread::operator()(CommonObjects& common)
{
	//GuiMain([] { DrawAppWindow(); });
	GuiMain(DrawAppWindow, &common);
	common.exit_flag = true;
}

