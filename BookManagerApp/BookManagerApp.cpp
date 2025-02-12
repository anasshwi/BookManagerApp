// BookManagerApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <thread>
#include "CommonObject.h"
#include "DrawThread.h"
#include "DownloadThread.h"


int main()
{
    //Starting the app by init the common object and starting the main threads for search, drawing the UI and the extra info search.
    CommonObjects common;
    DrawThread draw;
    auto draw_th = std::jthread([&] {draw(common); });
    DownloadThread down;
    auto down_th = std::jthread([&] {down(common); });
    std::cout << "running...\n";
    auto downInfoDesc = std::jthread([&] {down.getDescInfoDesc(common); });
    auto downImage = std::jthread([&] {down.getImage(common); });
    //down_th.join();
    //draw_th.join();
    //downInfoDesc.join();

    return 0;
}

