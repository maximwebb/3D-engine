// 3D Engine - ILA Project.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "3D Engine - ILA Project.h"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <sstream>


#define MAX_LOADSTRING 100

using namespace std;
using matrix = vector<vector<float>>;

matrix add(matrix mat1, matrix mat2);
matrix add(matrix mat1, vector<int> vec);
matrix subtract(matrix mat1, matrix mat2);
matrix subtract(matrix mat1, vector<int> vec);
matrix multiply(matrix mat1, matrix mat2);
matrix Mat(vector<float> mat1);
vector<int> apply(matrix transform_mat, vector<int> pt);
void clear(HDC hdc, RECT* prc);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

RECT window_rect;
int window_width;
int window_height;
const float PI = 3.14159;
const float GRAVITY = 0.40;
const int FLOOR_Y = 100;

float player_fov = 120 * (PI / 180);

//Function that creates a matrix, but not actaully a class constructor.
matrix Mat(vector<float> arr) {
	matrix mat;

	for (int row = 0; row < 4; row++) {
		mat.push_back({});
		for (int col = 0; col < 4; col++) {
			mat[row].push_back(arr[row * 4 + col]);
		}
	}
	return mat;
}

//Performs addition of two matrices, returning the output as a new matrix.
matrix add(matrix mat1, matrix mat2) {
	matrix mat3;
	for (int row = 0; row < 4; row++) {
		mat3.push_back({});
		for (int col = 0; col < 4; col++) {
			mat3[row].push_back(mat1[row][col] + mat2[row][col]);
		}
	}
	return mat3;
}

matrix add(matrix mat1, vector<int> vec) {
	matrix mat3;
	for (int row = 0; row < 4; row++) {
		mat3.push_back({});
		for (int col = 0; col < 4; col++) {
			mat3[row].push_back(mat1[row][col] + vec[row]);
		}
	}
	return mat3;
}

matrix subtract(matrix mat1, matrix mat2) {
	matrix mat3;
	for (int row = 0; row < 4; row++) {
		mat3.push_back({});
		for (int col = 0; col < 4; col++) {
			mat3[row].push_back(mat1[row][col] - mat2[row][col]);
		}
	}
	return mat3;
}

matrix subtract(matrix mat1, vector<int> vec) {
	matrix mat3;
	for (int row = 0; row < 4; row++) {
		mat3.push_back({});
		for (int col = 0; col < 4; col++) {
			mat3[row].push_back(mat1[row][col] - vec[row]);
		}
	}
	return mat3;
}

//Performs the multiplication of two matrices, returning the output as a new matrix.
matrix multiply(matrix mat1, matrix mat2) {
	matrix mat3;
	for (int row = 0; row < 4; row++) {
		mat3.push_back({});
		for (int num = 0; num < 4; num++) {
			mat3[row].push_back(0);
			for (int col = 0; col < 4; col++) {
				mat3[row][num] += (mat1[row][col] * mat2[col][num]);
			}
		}
	}

	return mat3;
}

//Applies the player's matrix to a point vector.
vector<int> apply(matrix transform_mat, vector<int> pt) {
	vector<int> pt2;
	for (int row = 0; row < 4; row++) {
		pt2.push_back(0);
		for (int col = 0; col < 4; col++) {
			pt2[row] += (pt[col] * transform_mat[row][col]);
		}
	}

	return pt2;
}

class PLAYER {
public:

	//Stores player's absolute universal coordinates from origin.
	vector<int> coords = { 0, 0, 0 };

	//Stores player's yaw.
	float masterAng = 0;

	void setAng(float ang) {
		if (masterAng > PI) {
			masterAng = -PI;
		}
		else if (masterAng < -PI) {
			masterAng = PI;
		}
	}

	//Used for adjusting the translation matrix.
	int xVel;
	int yVel;
	int zVel;

	//The amount by which the player moves after one incrementation of x/y/zVel (move speed).
	int masterVel = 5;

	//Used for adjusting the rotation matrix.
	float xAng;
	float yAng;

	//The amount by which the player rotates after one incrementation of x/yAng (sensitivity).
	float angVel = 0.029;

	//Player's location and orientation. 
	matrix pos = Mat({ {1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 120,
						0, 0, 0, 1} });

	//matrix pos = Mat({ { 1, 0, 0, 0, 1, 0, 0, 0, 1 } });

	//Defines the viewer's plane's equation.
	vector<float> vp_dir = { };
	float vp_dot = { };

};

PLAYER player;

class Obloid;
class Plane;

class Point {

public:


	/*Parameters*/
	vector<int> initPos;
	bool foo = false;

	/*Parent objects*/
	Obloid *parent_obloid;
	Plane *parent_plane;

	/*Inherited from parent objects*/
	float *xVel = 0;
	float *yVel = 0;
	float *zVel = 0;
	bool mass = false;
	bool *on_floor = false;

	/*Other properties*/
	int screen_x;
	int screen_y;
	bool offscreen = false;
	float relative_ang = 0;
	int width = 10;
	int height = 10;
	vector<int> pseudo_pos;
	vector<int> pos = initPos;

	/*Methods*/
	void assign_properties();


	//Recalculates where each point is drawn on client's screen based on coordinates.
	void update() {

		//Safeguards against null pointer
		if (xVel) {
			initPos[0] += *xVel;
			initPos[2] += *zVel;

			if (on_floor) {
				initPos[1] += *yVel;
			}
		}

		if (on_floor && initPos[1] > FLOOR_Y) {
			*on_floor = true;
		}

		//Calculates pixel position of point on display port.
		screen_x = window_width * 0.5 + window_width * pos[0] / (pos[2] * tan(player_fov / 2));
		cout << screen_x;
		screen_y = window_height * 0.5 + window_width * pos[1] / (pos[2] * tan(player_fov / 2));

		//Finds the point's angle relative to the player's line of sight (BROKEN)
		relative_ang = abs(atan2((initPos[0] - player.coords[0]), (initPos[2] - player.coords[2])) + player.masterAng) - PI;

		if (foo) {
			std::ostringstream msg;
			//msg << "Angle is: " << relative_ang << "\n";
			//msg << (player_fov / 2) << "\n";
			string s(msg.str());
			OutputDebugStringA(s.c_str());
		}

		//Check if point is out of the player's viewport
		offscreen = relative_ang > (player_fov / 2);
	}

	void draw(HDC hdc) {

		if (!offscreen) {
			HBRUSH brush_black = CreateSolidBrush(RGB(0, 0, 0));
			RECT point_rect = { screen_x, screen_y, screen_x + width, screen_y + height };
			FillRect(hdc, &point_rect, brush_black);

			DeleteObject(brush_black);
		}
	}
};

Point foo{ { -50, 0, 300, 1 }, true };


class Plane {
public:

	vector<Point> points;
	POINT arr[4];

	Obloid *parent_obloid;

	bool offscreen = false;
	bool total_offscreen = true;
	bool partial_offscreen = false;

	//Reference to containing obloid.
	//Obloid parent_obloid;

	void initialisePoints() {
		for (int n = 0; n < points.size(); n++) {

			points[n].parent_obloid = parent_obloid;
			points[n].parent_plane = this;
			points[n].pos = points[n].initPos;
			points[n].assign_properties();
			
		}
	}

	//Recalculates each point's coordinates.
	void updatePoints(HDC hdc, bool mass) {
		total_offscreen = true;
		partial_offscreen = false;

		for (int i = 0; i < points.size(); i++) {

			//points[i].initPos[1];
			points[i].pos = apply(player.pos, points[i].initPos);

			points[i].update();

			if (total_offscreen && points[i].offscreen) {
				total_offscreen = true;
			}
			else {
				total_offscreen = false;
			}

			if (points[i].offscreen) {
				partial_offscreen = true;
			}

			
			arr[i].x = points[i].screen_x;
			arr[i].y = points[i].screen_y;

		}

		//points[0].draw(hdc);
		//points[1].draw(hdc);

		//if (partial_offscreen) {
			std::ostringstream msg;
			//msg << "Partial Offscreen: " << partial_offscreen << "\n";
			//msg << "Total Offscreen: " << total_offscreen << "\n";
			string s(msg.str());
			OutputDebugStringA(s.c_str());
		//}


		offscreen = total_offscreen;
	}

	void connectPoints(HDC hdc) {
		MoveToEx(hdc, points[points.size() - 1].screen_x, points[points.size() - 1].screen_y, NULL);
		//for (int i = 0; i < points.size(); i++) {
		//	LineTo(hdc, points[i].screen_x, points[i].screen_y);
		//}
	}

	void fillPlane(HDC hdc, int n) {
		if (!offscreen) {
			//Currently not used.
			HBRUSH blue_brush = CreateSolidBrush(RGB(25, 60, 124));
			HBRUSH red_brush = CreateSolidBrush(RGB(124, 60, 25));
			//HBRUSH foo = SelectBrush(hdc, red_brush);

			SelectObject(hdc, GetStockObject(DC_BRUSH));
			if (n == 4) {
				SetDCBrushColor(hdc, RGB(200, 35, 16));
			}
			else {
				SetDCBrushColor(hdc, RGB(16, 35, 114));
			}
			Polygon(hdc, arr, 4);

			DeleteObject(red_brush);
			DeleteObject(blue_brush);
		}
	}

};

class Obloid {

public:

	/*PARAMETERS 
	Takes 4 coordinates: Top-Right-Front, Top-Right-Back, Bottom-Left-Back, Bottom-Left-Front*/
	Point trf{ { 0, 0, 0, 1 } };
	Point trb{ { 0, 0, 0, 1 } };
	Point blb{ { 0, 0, 0, 1 } };
	Point blf{ { 0, 0, 0, 1 } };

	bool mass = true;
	float restitution = 0.8;
	int bounces = 0;
	bool on_floor = false;
	float *floor;

	//Generates the other 4 coordinates: Bottom-Right-Front, Bottom-Right-Back, Top-Left-Front, Top-Left-Back

	Point brf{ { blb.initPos[0], trf.initPos[1], trf.initPos[2], 1 } };
	Point brb{ { blb.initPos[0], trf.initPos[1], blb.initPos[2], 1 } };
	Point tlf{ { trf.initPos[0], blb.initPos[1], trf.initPos[2], 1 } };
	Point tlb{ { trf.initPos[0], blb.initPos[1], blb.initPos[2], 1 } };

	Plane pl_top{ { trf, trb, tlb, tlf } };
	Plane pl_bottom{ { brf, brb, blb, blf } };
	Plane pl_left{ { tlf, tlb, blb, blf } };
	Plane pl_right{ { trf, brf, brb, trb } };
	Plane pl_front{ { trf, tlf, blf, brf } };
	Plane pl_back{ { trb, tlb, blb, brb } };

	vector<Plane> faces = { pl_top, pl_bottom, pl_left, pl_right, pl_back, pl_front };
	vector<Point> points = { trf, trb, blb, blf, brf, brb, tlf, tlb };

	vector<float> velocity = { 0, -20, 0 };

	void initialise_planes() {
		for (int m = 0; m < faces.size(); m++) {
			faces[m].parent_obloid = this;
			faces[m].initialisePoints();
		}
	}

	void update_faces(HDC hdc) {
		if (mass && !on_floor) {
			velocity[1] += GRAVITY;
		}
		else if (on_floor) {
			bounces++;

			//velocity[1] = 0;
			velocity[1] = -20 * pow(restitution, bounces);
			if (abs(velocity[1]) < 2) {
				velocity[1] = 0;
			}
			if (abs(velocity[0]) < 0.1) {
				velocity[0] = 0;
			}
			else {
				velocity[0] -= 0.5;
			}
			on_floor = false;
		}

		for (int k = 0; k < faces.size(); k++) {
			faces[k].updatePoints(hdc, mass);
			faces[k].fillPlane(hdc, k);
		}
	};
};

//Define inherited property assignment outside class definition so that the point's obloid parent is defined first.
void Point::assign_properties() {
	mass = parent_obloid->mass;
	on_floor = &parent_obloid->on_floor;
	xVel = &parent_obloid->velocity[0];
	yVel = &parent_obloid->velocity[1];
	zVel = &parent_obloid->velocity[2];
}

Obloid cube_1{
	Point{ { 0, 0, 400, 1 } },
	Point{ { 0, 0, 350, 1 } },
	Point{ { 50, 50, 350, 1 } },
	Point{ { 50, 50, 400, 1 } },
};

Obloid cube_2{
	Point{ { 0, -120, -400, 1 } },
	Point{ { 0, -120, -300, 1 } },
	Point{ { 100, 50, -300, 1 } },
	Point{ { 100, 50, -400, 1 } },
};

Obloid cube_3{
	Point{ { 200, 100, 600, 1 } },
	Point{ { 200, 100, 300, 1 } },
	Point{ { 300, 0, 300, 1 } },
	Point{ { 300, 0, 600, 1 } },
};

Obloid corridor_wall_1{
	Point{ { -100, 100, 900, 1 } },
	Point{ { -100, 100, 300, 1 } },
	Point{ { -50, 0, 300, 1 } },
	Point{ { -50, 0, 900, 1 } },
};

Obloid corridor_wall_2 {
	Point{ { 150, 100, 900, 1 } },
	Point{ { 150, 100, 300, 1 } },
	Point{ { 200, 0, 300, 1 } },
	Point{ { 200, 0, 900, 1 } },
};

Plane plane_1{ {
		Point{ { 100, 100, 300, 1 } },
		Point{ { 100, 200, 300, 1 } },
		Point{ { 200, 200, 300, 1 } },
		Point{ { 200, 100, 300, 1 } },
	} };

vector<Obloid> obloid_list = { cube_1 };

void setup() {

	window_width = window_rect.right - window_rect.left;
	window_height = window_rect.bottom - window_rect.top;

	for (int ob = 0; ob < obloid_list.size(); ob++) {
		obloid_list[ob].initialise_planes();
	}
}

void clear(HDC hdc, RECT* prc) {
	HBRUSH brush_white = CreateSolidBrush(RGB(255, 255, 255));
	FillRect(hdc, prc, brush_white);
}

const int MYTIMER = 1;

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

		//Updates player movements.
		if (GetAsyncKeyState('A') < 0) {
			player.xVel = -player.masterVel;
		}
		else if (GetAsyncKeyState('D') < 0) {
			player.xVel = player.masterVel;
		}
		else {
			player.xVel = 0;
		}

		if (GetAsyncKeyState('W') < 0) {
			player.zVel = player.masterVel;
		}
		else if (GetAsyncKeyState('S') < 0) {
			player.zVel = -player.masterVel;
		}
		else {
			player.zVel = 0;
		}

		if (GetAsyncKeyState(0x20) < 0) {
			player.yVel = player.masterVel;
		}
		else if (GetAsyncKeyState(0xA0) < 0) {
			player.yVel = -player.masterVel;
		}
		else {
			player.yVel = 0;
		}

		if (GetAsyncKeyState('F') < 0) {
			cube_1.velocity = { 10, 15, 2 };
		}



		//Updates rotations about y axis and z axis.
		if (GetAsyncKeyState(0x25) < 0) {
			player.xAng = player.angVel;
			player.masterAng += player.angVel;


		}
		else if (GetAsyncKeyState(0x27) < 0) {
			player.xAng = -player.angVel;
			player.masterAng -= player.angVel;

			//matrix rotXMat = Mat({ { cos(player.xAng), 0, sin(player.xAng), 0,
			//	0, 1, 0, 0,
			//	-sin(player.xAng), 0, cos(player.xAng), 0,
			//	0, 0, 0, 1
			//	} });

			//player.pos = multiply(rotXMat, player.pos);
		}

		else {
			player.xAng = 0;
		}

		matrix rotXMat = Mat({ { cos(player.xAng), 0, sin(player.xAng), 0,
								0, 1, 0, 0,
								-sin(player.xAng), 0, cos(player.xAng), 0,
								0, 0, 0, 1
								} });

		player.pos = multiply(rotXMat, player.pos);

		if (GetAsyncKeyState(0x26) < 0) {
			player.yAng = player.angVel;
		}
		else if (GetAsyncKeyState(0x28) < 0) {
			player.yAng = -player.angVel;
		}
		
		else {
			player.yAng = 0;
		}

		matrix rotYMat = Mat({ {1, 0, 0, 0,
								0, cos(player.yAng), sin(player.yAng), 0,
								0, -sin(player.yAng), cos(player.xAng), 0,
								0, 0, 0, 1
								} });

		player.pos = multiply(rotYMat, player.pos);

		//Translates the player's position matrix, to subsequently translate other objects in the universe.
		player.pos[0][3] -= player.xVel;
		player.pos[1][3] += player.yVel;
		player.pos[2][3] -= player.zVel;

		//Keeps track of player's universe coordinates relative to starting position.
		player.coords[0] += (player.xVel * cos(player.masterAng) + player.zVel * -sin(player.masterAng));
		player.coords[1] += player.yVel;
		player.coords[2] += (player.xVel * sin(player.masterAng) + player.zVel * cos(player.masterAng));

		std::ostringstream msg;
		msg << "Position is: " << player.coords[0] << ", " << player.coords[1] << ", " << player.coords[2] << " and angle is " << player.masterAng << "\n";

		player.setAng(player.masterAng);
		//msg << "Master Angle is: " << player.masterAng << "\n";
		string s(msg.str());
		OutputDebugStringA(s.c_str());


		clear(hdc, &window_rect);
		for (int ob = 0; ob < obloid_list.size(); ob++) {
			obloid_list[ob].update_faces(hdc);
		}

		foo.pos = apply(player.pos, foo.initPos);
		foo.update();
		//foo.draw(hdc);

		char debug_msg[100];

		ReleaseDC(hWnd, hdc);
	}
	case WM_PAINT:
	{

		PAINTSTRUCT ps;
		HDC hdc;
		//HDC mem_hdc;
		//HBITMAP mem_bitmap;

		hdc = BeginPaint(hWnd, &ps);

		//mem_hdc = CreateCompatibleDC(hdc);
		//mem_bitmap = CreateCompatibleBitmap(hdc, window_width, window_height);
		//SelectObject(mem_hdc, mem_bitmap);

		//BitBlt(hdc, 0, 0, window_width, window_height, mem_hdc, 0, 0, SRCCOPY);
		//DeleteObject(mem_bitmap);
		//DeleteDC(mem_hdc);
		//DeleteDC(hdc);

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





