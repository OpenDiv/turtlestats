#ifndef SOFTWAREDEFINITIONS_H
#define SOFTWAREDEFINITIONS_H
enum WndCommands
{
	HostSocket,
	AdressSelection,
	AvailablePorts,
	BtnSendData,
	QuickStart
};

#define OnMenuAction0 QuickStart;
#define OnMenuAction1 HostSocket
#define OnMenuAction2 AdressSelection
#define OnMenuAction3 AvailablePorts
#define OnButtonClicked BtnSendData


HWND hStaticControl;
HWND hStaticChatStatus;
HWND hStaticConnection;
HWND hStaticTurtleList;
HWND hEditControl;
HWND hEditChat;
HWND hEditTurtleList;
HWND hStaticLastMsg;

HWND hCombo;
char TempBuffer[20];

HFONT fontMain;

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure);

void setWindowFont(HWND hWnd, HFONT hFont);
void MainWndAddMenus(HWND hWnd);
void MainWndAddWidgets(HWND hWnd, HINSTANCE hInst);

#endif
