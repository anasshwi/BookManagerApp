#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"

void DrawAppWindow(void* common_ptr)
{
	auto common = (CommonObjects*)common_ptr;
	ImGui::Begin("Book Management App");
	ImGui::Text("Search Bar		");
	static char buff[256];
	ImGui::SameLine();
	ImGui::InputText( " ", buff, sizeof(buff));
	ImGui::SameLine();
	if (ImGui::Button("Search Books"))
		common->searchBar = buff;
	if (common->data_ready)
	{
		if (ImGui::BeginTable("Book List", 4))  //, flags))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("Author");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			for (size_t i = 0; i < common->books.size(); ++i) {
				const Book& book = common->books[i];

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(book.title.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(book.author_name[0].c_str());
				ImGui::TableSetColumnIndex(3);

				std::string popupID = "Book Details##" + std::to_string(i);
				if (ImGui::Button(("Show more details ## "+ std::to_string(i)).c_str()))
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
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();


}

void DrawThread::operator()(CommonObjects& common)
{
	//GuiMain([] { DrawAppWindow(); });
	GuiMain(DrawAppWindow, &common);
	common.exit_flag = true;
}

