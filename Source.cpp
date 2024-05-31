#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <codecvt>
#include <time.h>
#include <ctime>

#include "Listener.h"
#include "SoftwareDefinitions.h"

#include <Windows.h>
#include "resource.h"

HANDLE socketThread;

int mainWndSizeX = 500;
int mainWndSizeY = 570;



INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int ncmdshow) // точка вхождения, создаёт окно
{
	fontMain = CreateFont(15, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic");

	WNDCLASS SoftwareMainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)),
		L"MainWndClass", SoftwareMainProcedure);

	if (!RegisterClassW(&SoftwareMainClass))
		return -1;
	
	MSG SoftwareMainMessage = { 0 };
	HWND hWnd = CreateWindow(L"MainWndClass", L"CC:Tweaked Miner", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 300, 300, mainWndSizeX, mainWndSizeY, NULL, NULL, NULL, NULL);

	setWindowFont(hWnd, fontMain);
	

	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL)) {
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}
	TerminateThread(socketThread, 0);


}

WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure)
{
	WNDCLASS NWC = { 0 };
	NWC.hbrBackground = BGColor;
	NWC.hCursor = Cursor;
	NWC.hInstance = hInst;
	NWC.hIcon = Icon;
	NWC.lpszClassName = Name;
	NWC.lpfnWndProc = Procedure;

	return NWC;
}

bool hostStarted = false;
std::vector < std::shared_ptr<EchoWebSocket>> socketList;

DWORD WINAPI LookForConnections(LPVOID lpParameter)
{
	std::string tempMsg = "Socket started";
	SetWindowTextA(hStaticControl, tempMsg.c_str());

	namespace net = boost::asio;

	auto const port = 8083;
	int threadAmount = 4;

	net::io_context ioc{ threadAmount }; // кол-во поддерживаемых потоков
	std::make_shared<Listener>(ioc, port)->asyncAccept();

	ioc.run(); // не позволяет завершиться программе main (return 1)
	return 0;
}

void setWindowFont(HWND hWnd, HFONT hFont)
{
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	HWND hChild = GetWindow(hWnd, GW_CHILD);
	while (hChild != NULL)
	{
		SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
		hChild = GetWindow(hChild, GW_HWNDNEXT);
	}
}

void socketHost();

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case::WndCommands::QuickStart:
			MessageBoxA(hWnd, "Start", "Starting", MB_OK);
			break;
		case::WndCommands::BtnSendData:
			GetWindowTextA(hEditControl, TempBuffer, 20);
			for (auto it : socketList)
				it->send(TempBuffer);
			break;
		case WndCommands::HostSocket:
			if (!hostStarted)
			{
				hostStarted = true;
				MessageBoxA(hWnd, "Host started", "ws://127.0.0.1:8083/", MB_OK);
				//SetWindowTextA(hStaticConnection, "Status: Opened for connections");
				socketThread = CreateThread(NULL, 0, LookForConnections, NULL, 0, NULL);
			}
			else
				MessageBoxA(hWnd, "Already running", "Error", MB_OK);

			break;
		case WndCommands::AdressSelection:
			MessageBoxA(hWnd, "IP:\n Port\n", "Adress configuration", MB_OK);
			break;
		case WndCommands::AvailablePorts:
			MessageBoxA(hWnd, "8083\n 5656\n 1243\n 1337", "Ports list", MB_OK);
				break;
		default:
			break;
		}
		break;
	case WM_CREATE:
		if (lp != 0)
		{
			auto& myInst{ *reinterpret_cast<LPCREATESTRUCTW>(lp) };
			MainWndAddMenus(hWnd);
			MainWndAddWidgets(hWnd, myInst.hInstance);
		}
		else
			break;
		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	case WM_MEASUREITEM:
	{
		MEASUREITEMSTRUCT* pmis = (MEASUREITEMSTRUCT*)lp;
		pmis->itemHeight = 50;
		return TRUE;

	}
	case WM_DRAWITEM:
	{
		DRAWITEMSTRUCT* pdis = (DRAWITEMSTRUCT*)lp;
		if (pdis->itemID != -1 && (pdis->itemAction & ODA_DRAWENTIRE))
		{
			const int iconSizeHeight = 40;
			const int iconSizeWidth = 40;

			HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
			if (hIcon)
			{
				RECT rcIcon = pdis->rcItem;
				rcIcon.left += 5;
				rcIcon.top += 5;
				rcIcon.right = rcIcon.left + GetSystemMetrics(SM_CXSMICON);
				rcIcon.bottom = rcIcon.top + GetSystemMetrics(SM_CYSMICON);
				
				DrawIconEx(pdis->hDC, rcIcon.left, rcIcon.top, hIcon, 
					iconSizeWidth, iconSizeHeight, 0, NULL, DI_NORMAL);
			}

			RECT rcText = pdis->rcItem;
			rcText.left += iconSizeWidth+10;
			rcText.bottom -= 30;
			WCHAR buffer[256];
			SendMessage(pdis->hwndItem, LB_GETTEXT, pdis->itemID, (LPARAM)buffer); // получаем текст элемента
			DrawText(pdis->hDC, buffer, lstrlen(buffer), &rcText, DT_SINGLELINE | DT_VCENTER); // рисуем текст
		}
	}
	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void MainWndAddMenus(HWND hWnd)
{
	HWND hEditControl;
	HMENU RootMenu = CreateMenu();
	HMENU SubMenu = CreateMenu();

	HMENU StartMenu = CreateMenu();
	AppendMenu(RootMenu, MF_STRING, WndCommands::HostSocket, L"Host");

	AppendMenu(SubMenu, MF_STRING, WndCommands::HostSocket, L"Host socket");
	AppendMenu(SubMenu, MF_STRING, WndCommands::AdressSelection, L"Select adress");
	AppendMenu(SubMenu, MF_STRING, WndCommands::AvailablePorts, L"Available ports list");

	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"File");
	
	SetMenu(hWnd, RootMenu);
}

void mnePoh(HWND hWnd, HINSTANCE hInst)
{

}

void MainWndAddWidgets(HWND hWnd, HINSTANCE hInst)
{
	//cords
		const int turtleListBoxCordX = 20;
		const int turtleListBoxCordY = 20;
		const int turtleListBoxSizeX = 200;
		const int turtleListBoxSizeY = 300;

		const int chatEditCordX = turtleListBoxCordX;
		const int chatEditCordY = turtleListBoxSizeY + turtleListBoxCordY + 25;
		const int chatEditSizeX = mainWndSizeX - (turtleListBoxCordX * 3);
		const int chatEditSizeY = 400;
	//hStaticControl = CreateWindowA("static", "Static text", WS_VISIBLE | WS_CHILD | ES_CENTER, 1115, 80, 470, 20, hWnd, NULL, NULL, NULL);
	//connection status
	{
		hStaticConnection = CreateWindowA("static", "Status: No connections", WS_VISIBLE | WS_CHILD, mainWndSizeX-155, 10, 150, 150, hWnd, NULL, NULL, NULL);
	}

	//chat window
	{
		hEditChat = CreateWindowA("edit", "", WS_VSCROLL | ES_MULTILINE |WS_VISIBLE | WS_CHILD | ES_READONLY, chatEditCordX, chatEditCordY, chatEditSizeX, chatEditSizeY, hWnd, NULL, NULL, NULL);
		hStaticChatStatus = CreateWindowA("static", "Chat: Unavailable", WS_VISIBLE | WS_CHILD | ES_CENTER,chatEditCordX,
			chatEditCordY-19, chatEditSizeX, 30, hWnd, NULL, NULL, NULL);
		//hStaticLastMsg = CreateWindowA("static", "Last msg", WS_VISIBLE | WS_CHILD | ES_CENTER, 200,
		//	200, 200, 200, hWnd, NULL, NULL, NULL);
		hEditControl = CreateWindowA("edit", NULL, WS_VISIBLE | WS_CHILD, chatEditCordX,
			chatEditCordY, chatEditSizeX, chatEditSizeY, hWnd, NULL, NULL, NULL);
	}

	//turtle list
	{
		hStaticTurtleList = CreateWindowA("static", "Turtles available: 0/0", WS_VISIBLE | WS_CHILD | ES_CENTER,
			turtleListBoxCordX, 2, turtleListBoxSizeX, 20, hWnd, NULL, NULL, NULL);
		hEditTurtleList = CreateWindow(L"LISTBOX", NULL, 
			WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS, 
			turtleListBoxCordX, turtleListBoxCordY, turtleListBoxSizeX, turtleListBoxSizeY, hWnd, NULL, hInst, NULL);
		
		for (int i = 0; i < 1; i++) // filling with icons l8r switch for multi sockets
		{
			

		}
		
	}
}