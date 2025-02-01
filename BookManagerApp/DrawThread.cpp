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
	ImGui::InputText("", buff, sizeof(buff));
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

			for (auto& rec : common->books)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(rec.title.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(rec.author_name[0].c_str());
				ImGui::TableSetColumnIndex(3);
				if (ImGui::Button("Show more details"))
				{
					//show details
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

