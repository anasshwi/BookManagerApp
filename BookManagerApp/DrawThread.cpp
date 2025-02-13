#include <iostream>
#include <fstream>
#include "DrawThread.h"
#include "DownloadThread.h"
#include "GuiMain.h"
#include <vector>
#include <algorithm>
#include <thread>
#include "../../shared/ImGuiSrc/imgui.h"

bool show_fav_list = false;
bool show_detail_window = false;
bool show_fav_detail_window = false;
bool show_cover_window = false;

// Global for Image
int my_image_width = 0;
int my_image_height = 0;
ID3D11ShaderResourceView* my_texture = NULL;

void SaveFavoritesToFile(const  std::vector<Book>& favorites) 
{
	//Function that saves favorites list of books to a file saved in the path: "${SolutionDir}\BookManagerApp\".
	std::ofstream file("Favorites.txt");
	if (!file) {
		std::cout << "Error: Couldn't open file for writing\n";
	}

	for (const auto& book : favorites) {
		file << "Title: " << book.title << "\n";
		file << "Author(s): ";
		for (const auto& author : book.author_name) {
			file << author << "; ";
		}
		file << "\nFirst Publish Year: " << book.first_publish_year << "\n";
		file << "Description: " << book.description << "\n";
		file << "Rating: " << book.rating << "\n";
		file << "Personal Note: " << book.personal_note << "\n";
		file << "---------------------------------------------------------------------------------\n";
	}
}

void DrawAppWindow(void* common_ptr)
{
	//Function that draws the User Interface for the App.
	auto common = (CommonObjects*)common_ptr;
	ImGui::Begin("Book Management App");
	ImGui::Checkbox("Favorites List", &show_fav_list);
	ImGui::Text("Search Bar		");
	static char buff[256];
	ImGui::SameLine();
	ImGui::InputText( " ", buff, sizeof(buff));
	ImGui::SameLine();
	if (ImGui::Button("Search Books")) 
	{
		//Start the Thread to search for Books in Url.
		std::lock_guard<std::mutex> lock(common->mtx);
		common->searchBar = buff;
		
		common->start_download.store(true);
		common->cv.notify_all();
	}

	ImGui::NewLine();
	ImGui::Separator();

	if (common->data_ready)
	{
		//If the Data from the Search is Ready to Show in the UI.
		if (ImGui::BeginTable("Book List", 5))  //, flags))
		{
			ImGui::TableSetupColumn("#");
			ImGui::TableSetupColumn("Book Name");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("Author");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			if (!common->books.empty()) {
				for (size_t i = 0; i < common->books.size(); ++i) {
					const Book& book = common->books[i];

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%d", i+1);
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(book.title.c_str());
					ImGui::TableSetColumnIndex(3);
					ImGui::Text(book.author_name[0].c_str());
					ImGui::TableSetColumnIndex(4);

					std::string popupID = "Book Details##" + std::to_string(i);
					if (ImGui::Button(("Show more details ## " + std::to_string(i)).c_str()))
					{
						//show details
						std::lock_guard<std::mutex> lock(common->mtx);
						common->Index = i;

						common->info_ready = false;
						common->img_ready = false;
						common->download_info_desc.store(true);
						common->cv2.notify_all();
						common->download_img.store(true);
						common->cv3.notify_all();
						show_detail_window = true;
					}

				}
			}
			else {
				ImGui::Text("No Results Found! Search again.");
			}
			ImGui::EndTable();
		}
	}
	else {
		//When theres no data of books to show.
		ImGui::NewLine();
		ImGui::Text("Search Books to Get Started!");
	}
	ImGui::End();

	if (show_detail_window)
	{
		//Shows the detail window of a selected book.
		ImGui::Begin("Book Info", &show_detail_window);
		ImGui::Text("Title: %s", common->books[common->Index].title.c_str());
		ImGui::NewLine();

		std::string authors = common->books[common->Index].author_name.empty() ? "Unknown" : common->books[common->Index].author_name[0];
		for (size_t j = 1; j < common->books[common->Index].author_name.size(); ++j) {
			authors += ", " + common->books[common->Index].author_name[j];
		}
		ImGui::Text("Authors: %s", authors.c_str());
		ImGui::NewLine();

		ImGui::Text("Publish Year: %d", common->books[common->Index].first_publish_year);
		ImGui::NewLine();

		ImGui::TextWrapped("Description: %s", common->books[common->Index].description.c_str());
		ImGui::NewLine();

		ImGui::Text("Rating: %.2f", common->books[common->Index].rating);
		ImGui::NewLine();

		
		if (common->info_ready && common->img_ready) {
			if (std::find(common->FavBooks.begin(), common->FavBooks.end(), common->books[common->Index]) != common->FavBooks.end()) {
				ImGui::Text("Added to Favorites");
			}
			else {
				if (ImGui::Button("Add to Favorites"))
				{
					//show details
					common->FavBooks.push_back(common->books[common->Index]);
				}

			}
			ImGui::SameLine();

			if ((common->books[common->Index].coverID != -1)) {
				if (ImGui::Button("Show Cover")) {
					my_image_width = 0;
					my_image_height = 0;
					my_texture = NULL;
					LoadTexture(common->books[common->Index].image, &my_texture, &my_image_width, &my_image_height);

					show_cover_window = true;
				}
			}
			else {
				ImGui::Text("No Image Data.");
			}
		}
		else
		{
			ImGui::Text("Loading Data");
		}


		if (ImGui::Button("Close")) {
			show_detail_window = false;
		}
		ImGui::End();
	}

	if (show_cover_window) {
		ImGui::Begin("Image Viewer", &show_cover_window);
		if (my_texture) {
			ImGui::Image((void*)my_texture, ImVec2(200, 300));
		}
		else {
			ImGui::Text("No Image Data");
		}
		ImGui::End();
	}

	if (show_fav_list)
	{
		//Shows a window with a list of the favorite books saved.
		ImGui::Begin("Favorites Window", &show_fav_list);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Here Lies all the Favorite Books that you Selected!");
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			show_fav_list = false;
		}

		if (ImGui::BeginTable("Book List", 6))  //, flags))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("Author");
			ImGui::TableSetupColumn("");
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
					common->FavIndex = i;
					show_fav_detail_window = true;
				}

				ImGui::TableSetColumnIndex(4);
				static char note[256] = "";  // Buffer for note input
				if (ImGui::Button(("Leave Note ## " + std::to_string(i)).c_str())){
					
					if (!common->FavBooks[i].personal_note.empty()) {
						strncpy_s(note, common->FavBooks[i].personal_note.c_str(), sizeof(note) - 1);
						note[sizeof(note) - 1] = '\0';  // Ensure null termination
					}
					ImGui::OpenPopup(("NotePopup_" + std::to_string(i)).c_str());
				}

				if (ImGui::BeginPopup(("NotePopup_" + std::to_string(i)).c_str())) {
					ImGui::InputTextMultiline("##note", note, IM_ARRAYSIZE(note));

					if (ImGui::Button("Save")) {
						common->FavBooks[i].personal_note = note;
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::TableSetColumnIndex(5);
				
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

			ImGui::NewLine();
			if (ImGui::Button("Save Favorites to File")) {
				SaveFavoritesToFile(common->FavBooks);
				ImGui::OpenPopup("Saved!");
			}
				if (ImGui::BeginPopup("Saved!")) {
					ImGui::Text("The Favorites List has been Saved in the App's Directory!");

					if (ImGui::Button("OK")) {
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
		}
		

		ImGui::End();
	}

	if (show_fav_detail_window) 
	{
		//Shows a window with the details of the favorite book selected.
		ImGui::Begin("Fav Book Info", &show_fav_detail_window);
		ImGui::Text("Title: %s", common->FavBooks[common->FavIndex].title.c_str());
		ImGui::NewLine();

		std::string authors = common->FavBooks[common->FavIndex].author_name.empty() ? "Unknown" : common->FavBooks[common->FavIndex].author_name[0];
		for (size_t j = 1; j < common->FavBooks[common->FavIndex].author_name.size(); ++j) {
			authors += ", " + common->FavBooks[common->FavIndex].author_name[j];
		}
		ImGui::Text("Authors: %s", authors.c_str());
		ImGui::NewLine();

		ImGui::Text("Publish Year: %d", common->FavBooks[common->FavIndex].first_publish_year);
		ImGui::NewLine();

		ImGui::TextWrapped("Description: %s", common->FavBooks[common->FavIndex].description.c_str());
		ImGui::NewLine();

		ImGui::Text("Rating: %.2f", common->FavBooks[common->FavIndex].rating);
		ImGui::NewLine();

		ImGui::Text("Personal Notes: %s", common->FavBooks[common->FavIndex].personal_note.c_str());
		ImGui::NewLine();

		if ((common->FavBooks[common->FavIndex].coverID != -1)) {
			if (ImGui::Button("Show Cover")) {
				my_image_width = 0;
				my_image_height = 0;
				my_texture = NULL;
				LoadTexture(common->FavBooks[common->FavIndex].image, &my_texture, &my_image_width, &my_image_height);

				show_cover_window = true;
			}
		}
		else {
			ImGui::Text("No Image Data.");
		}
		ImGui::NewLine();

		if (ImGui::Button("Close")) {
			show_fav_detail_window = false;
		}
		ImGui::End();
	}
}



void DrawThread::operator()(CommonObjects& common)
{
	//GuiMain([] { DrawAppWindow(); });
	GuiMain(DrawAppWindow, &common);
	common.exit_flag.store(true);
	common.cv.notify_all();
	common.cv2.notify_all();
	common.cv3.notify_all();
}

