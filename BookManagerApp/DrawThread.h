
#pragma once
#include "CommonObject.h"
#include <d3d11.h>

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;


class DrawThread
{
public:
	void operator()(CommonObjects& common);
	//void DrawAppWindow();
};

