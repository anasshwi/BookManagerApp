#pragma once
#include <vector>

using drawcallback = void(void*);

extern ID3D11ShaderResourceView* my_texture ;

int GuiMain(drawcallback drawfunction, void* obj_ptr);
bool LoadTexture(std::vector<unsigned char> imagedata, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

