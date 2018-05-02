// 3D Engine - ILA Project.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "3D Engine - ILA Project.h"
#include <vector>
#include <cmath>
#include <iostream>


#define MAX_LOADSTRING 100

using namespace std;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

int player_fov = 90;
RECT window_rect;
int window_width;
int window_height;

class PLAYER {
public:

	vector<int> coords = { 0, 0, 0 };
	int fov_x = 90;
	int fov_y = 90;

};

PLAYER player;

class Point {

public:

	vector<int> pos;
	int screen_x;
	int screen_y;
	int width = 3;
	int height = 3;

	//Recalculates where each point is drawn on client's screen based on coordinates.
	void update() {
		screen_x = 300 + window_width * pos[0] / (pos[2] * tan(player_fov / 2));
		cout << screen_x;
		screen_y = window_width * pos[1] / (pos[2] * tan(player_fov / 2));
		OutputDebugStringA("width is ");
	}

	void draw(HDC hdc) {

		HBRUSH brush_black = CreateSolidBrush(RGB(0, 0, 0));
		RECT point_rect = { screen_x, screen_y, screen_x + width, screen_y + height };
		FillRect(hdc, &point_rect, brush_black);

		DeleteObject(brush_black);
	}

};

class Plane {
public:

	vector<Point> points;

	//Recalculates each point's coordinates.
	void updatePoints(int xVel, int yVel, int zVel) {
		for (int i = 0; i < points.size(); i++) {

			points[i].pos[0] += xVel;
			points[i].pos[1] += yVel;
			points[i].pos[2] += zVel;

			points[i].update();
		}
	}

	void connectPoints(HDC hdc) {
		MoveToEx(hdc, points[points.size() - 1].screen_x, points[points.size() - 1].screen_y, NULL);
		for (int i = 0; i < points.size(); i++) {
			LineTo(hdc, points[i].screen_x, points[i].screen_y);
		}
	}

};

class Obloid {

public:

	//Takes 4 coordinates: Top-Right-Front, Top-Right-Back, Bottom-Left-Back, Bottom-Left-Front
	Point trf;
	Point trb;
	Point blb;
	Point blf;

	//Generates the other 4 coordinates: Bottom-Right-Front, Bottom-Right-Back, Top-Left-Front, Top-Left-Back
	Point brf{ { blb.pos[0], trf.pos[1], trf.pos[2] } };
	Point brb{ { blb.pos[0], trf.pos[1], blb.pos[2] } };
	Point tlf{ { trf.pos[0], blb.pos[1], trf.pos[2] } };
	Point tlb{ { trf.pos[0], blb.pos[1], blb.pos[2] } };

	Plane pl_top{ { trf, trb, tlb, tlf } };
	Plane pl_bottom{ { brf, brb, blb, blf } };
	Plane pl_left{ { tlf, tlb, blb, blf } };
	Plane pl_right{ { trf, brf, brb, trb } };
	Plane pl_front{ { trf, tlf, blf, brf } };
	Plane pl_back{ { trb, tlb, blb, brb } };

	vector<Plane> faces = { pl_top, pl_bottom, pl_left, pl_right, pl_back };

	int xVel = 0;
	int yVel = 0;
	int zVel = 0;

	int yFloor = blb.pos[0];
	int gravity = -1;

	void drawFaces(HDC hdc) {
		for (int k = 0; k < faces.size(); k++) {
			faces[k].updatePoints(xVel, yVel, zVel);
			faces[k].connectPoints(hdc);
		}
	};
};

Obloid cube_1{
	Point{ { 0, 100, 600 } },
	Point{ { 0, 100, 300 } },
	Point{ { 100, 0, 300 } },
	Point{ { 100, 0, 600 } },
};

Obloid cube_2{
	Point{ { 200, 100, 600 } },
	Point{ { 200, 100, 300 } },
	Point{ { 300, 0, 300 } },
	Point{ { 300, 0, 300 } },
};

Plane plane_1{ {
		Point{ { 100, 100, 300 } },
		Point{ { 100, 200, 300 } },
		Point{ { 200, 200, 300 } },
		Point{ { 200, 100, 300 } },
	} };

void setup() {

	window_width = window_rect.right - window_rect.left;
	window_height = window_rect.bottom - window_rect.top;
}

void clear(HDC hdc, RECT* prc) {
	HBRUSH brush_white = CreateSolidBrush(RGB(255, 255, 255));
	FillRect(hdc, prc, brush_white);
}

const int MYTIMER = 1;
const float GRAVITY = 0.8;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.


	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MY3DENGINEILAPROJECT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY3DENGINEILAPROJECT));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MY3DENGINEILAPROJECT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY3DENGINEILAPROJECT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

HWND HWND_GLOBAL;



BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);

	UpdateWindow(hWnd);
	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = GetDC(hWnd);

	switch (message)
	{
	case WM_CREATE:
	{
		int timer = SetTimer(hWnd, MYTIMER, 5, NULL);
		GetClientRect(hWnd, &window_rect);
		setup();
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_KEYDOWN:
	{

	}
	case WM_KEYUP:
	{

	}
	case WM_TIMER:
	{

		RECT rcClient;
		clear(hdc, &window_rect);
		if (GetAsyncKeyState('A') < 0) {
			cube_1.xVel = 2;
			cube_2.xVel = 2;
		}
		else if (GetAsyncKeyState('D') < 0) {
			cube_1.xVel = -2;
			cube_2.xVel = -2;
		}
		else {
			cube_1.xVel = 0;
			cube_2.xVel = 0;
		}

		if (GetAsyncKeyState('W') < 0) {
			cube_1.zVel = -2;
			cube_2.zVel = -2;
		}
		else if (GetAsyncKeyState('S') < 0) {
			cube_1.zVel = 2;
			cube_2.zVel = 2;
		}
		else {
			cube_1.zVel = 0;
			cube_2.zVel = 0;
		}

		if (GetAsyncKeyState(0x20) < 0) {
			cube_1.yVel = 2;
			cube_2.yVel = 2;
		}
		else if (GetAsyncKeyState(0xA0) < 0) {
			cube_1.yVel = -2;
			cube_2.yVel = -2;
		}
		else {
			cube_1.yVel = 0;
			cube_2.yVel = 0;
		}

		plane_1.updatePoints(0, 0, 0);
		plane_1.connectPoints(hdc);
		//cube_1.drawFaces(hdc);
		//cube_2.drawFaces(hdc);

		ReleaseDC(hWnd, hdc);
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	}
					 break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}





