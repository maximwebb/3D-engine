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
int sgn(float num);
void clear(HDC hdc, RECT* prc);
void output(string str, bool line_break);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

RECT window_rect;
int window_width;
int window_height;
const float PI = 3.14159;
const float GRAVITY = 0.4;
const float AIR_RES = 0.003;
const int FLOOR_Y = 300;
const bool SKY = false;
int draw = 0;

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

int sgn(float num) {
	if (num > 0) {
		return 1;
	}
	else if (num < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

void output(string str, bool line_break) {
	std::ostringstream msg;
	msg << str;
	if (line_break) {
		msg << "\n";
	}
	string s(msg.str());
	OutputDebugStringA(s.c_str());
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

	vector<int> vel = { 0, 0, 0 };

	//The amount by which the player moves after one incrementation of x/y/zVel (move speed).
	int masterVel = 8;

	//Used for adjusting the rotation matrix.
	float xAng;
	float yAng;

	//The amount by which the player rotates after one incrementation of x/yAng (sensitivity).
	float angVel = 0.025;

	//Player's location and orientation. 
	matrix pos = Mat({ {1, 0, 0, -500,
						0, 1, 0, 0,
						0, 0, 1, 500,
						0, 0, 0, 1} });

	//matrix pos = Mat({ { 1, 0, 0, 0, 1, 0, 0, 0, 1 } });

	//Defines the viewer's plane's equation.
	vector<float> vp_dir = { };
	float vp_dot = { };

};

PLAYER player;

class Obloid;
class Plane;
class Line;

class Point {

public:


	/*Parameters*/
	vector<int> abs_pos;
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
	bool *on_floor_temp = false;

	/*Other properties*/
	int screen_x;
	int screen_y;
	bool offscreen = false;
	float relative_ang = 0;
	int width = 5;
	int height = 5;
	vector<int> pseudo_pos = abs_pos;
	vector<int> pos = abs_pos;

	/* Methods */
	void assign_properties();

	void update(bool pseudo) {

		/* Safeguards against null pointer */
		if (xVel) {
			abs_pos[0] += *xVel;
			abs_pos[2] += *zVel;

			if (!*on_floor) {
				abs_pos[1] += *yVel;
			}
		}

		//Calculates pixel position of point on viewport. Either uses pseudo-coordinates or real coordinates, depending on location of viewer
		if (offscreen) {
			screen_x = window_width * 0.5 + window_width * pseudo_pos[0] / (tan(player_fov / 2));
			screen_y = window_height * 0.5 + window_width * pseudo_pos[1] / (tan(player_fov / 2));
		}
		else {
			screen_x = window_width * 0.5 + window_width * pos[0] / (abs(pos[2]) * tan(player_fov / 2));
			screen_y = window_height * 0.5 + window_width * pos[1] / (abs(pos[2]) * tan(player_fov / 2));
		}

		//Check if point is out of the player's viewport
		offscreen = pos[2] < 1;
		
	}

	void pseudo_shift(vector<int> end_pos) {
		float scal = ((float)pos[2] - 1) / ((float)pos[2] - (float)end_pos[2]);

		pseudo_pos[0] =  (((float)pos[0] * (1 - scal)) + (scal * (float)end_pos[0]));
		pseudo_pos[1] = (((float)pos[1] * (1 - scal)) + (scal * (float)end_pos[1]));
	}

	void draw(HDC hdc) {

		if (!offscreen) {
			HBRUSH brush_black = CreateSolidBrush(RGB(0, 0, 0));
			RECT point_rect = { screen_x, screen_y, screen_x + width, screen_y + height };
			FillRect(hdc, &point_rect, brush_black);

			DeleteObject(brush_black);
		}
	}

	//Tests if point is below ground and then updates parent obloid's on_floor property.
	void detect_collision() {
		if (abs_pos[1] > FLOOR_Y) {
			*on_floor = true;
		}
	}
};

Point foo{ { -50, 0, 300, 1 }, true };


class Line {
public:

	/* Parameters */
	vector<Point> points;

	void draw(HDC hdc) {
		MoveToEx(hdc, points[0].screen_x, points[0].screen_y, NULL);
		LineTo(hdc, points[1].screen_x, points[1].screen_y);
		points[0].draw(hdc);
		points[1].draw(hdc);
	}

	void update_points() {
		points[0].pos = apply(player.pos, points[0].abs_pos);
		points[0].update(false);
		points[1].pos = apply(player.pos, points[1].abs_pos);
		points[1].update(false);

		if (points[0].pos[2] * points[1].pos[2] < 0) {
			if (points[0].pos[2] < 0) {
				points[0].pseudo_shift(points[1].pos);
			}
			else if (points[1].pos[2] < 0) {
				points[1].pseudo_shift(points[0].pos);
			}
		}
			


		//if (points[0].pos[2] * points[1].pos[2] <= 0) {
		//	output("Both in front and behind", true);
		//}
		//else if (points[0].pos[2] + points[1].pos[2] > 0) {
		//	output("Fully in front", true);
		//}
		//else {
		//	output("Fully behind", true);
		//}
	}



};

Line line_1{{Point{ { -50, 100, 800, 1 } },
			Point{ { -50, 100, 200, 1 } } 
} };

vector<Line> line_list = { line_1 };

class Plane {
public:

	vector<Point> points;
	POINT arr[4];

	Obloid *parent_obloid;

	bool offscreen = false;
	bool total_offscreen = true;
	bool partial_offscreen = false;
	bool pseudo = false;
	int points_size = 4;
	int closest_point;
	int furthest_point;
	int depth;

	/* Reference to containing obloid. */
	//Obloid parent_obloid;

	void initialisePoints() {
		for (int n = 0; n < points_size; n++) {

			points[n].parent_obloid = parent_obloid;
			points[n].parent_plane = this;
			points[n].pos = points[n].abs_pos;
			points[n].assign_properties();
			
		}
	}

	/* Recalculates each point's coordinates. */
	void update_points(HDC hdc, bool mass) {

		/* Resets closest point */
		closest_point = points[0].pos[0] + points[0].pos[1] + points[0].pos[2];

		total_offscreen = true;
		partial_offscreen = false;

		for (int i = 0; i < points_size; i++) {

			//points[i].abs_pos[1];
			points[i].pos = apply(player.pos, points[i].abs_pos);

			points[i].update(pseudo);

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

			if (points[i].pos[0] + points[i].pos[1] + points[i].pos[2] < closest_point) {
				closest_point = points[i].pos[0] + points[i].pos[1] + points[i].pos[2];
			}

			else if (points[i].pos[2] > furthest_point) {
				furthest_point = points[i].pos[2];
			}
			depth = closest_point;
		}

		if (total_offscreen) {
			partial_offscreen = false;
		}

		pseudo = partial_offscreen;

		/* Assigns points that are offscreen a pseudo position, to keep visuals normal. */
		if (partial_offscreen) {

			for (int i = 0; i < points_size; i++) {
				if (points[i].offscreen) {
					if (points[(i + 1 + points_size) % points_size].pos[2] > 0) {
						points[i].pseudo_shift(points[(i + 1 + points_size) % points_size].pos);
					}
					else if (points[(i - 1 + points_size) % points_size].pos[2] > 0) {
						points[i].pseudo_shift(points[(i - 1 + points_size) % points_size].pos);
					}
				}
			}
		}

		//if (partial_offscreen) {
		//	output("Partially offscreen", true);
		//}
		//if (total_offscreen) {
		//	output("Totally offscreen", true);
		//}


		offscreen = total_offscreen;
	}

	void connectPoints(HDC hdc) {
		MoveToEx(hdc, points[points.size() - 1].screen_x, points[points.size() - 1].screen_y, NULL);
		//for (int i = 0; i < points.size(); i++) {
		//	LineTo(hdc, points[i].screen_x, points[i].screen_y);
		//}
	}

	void fill_plane(HDC hdc, int n) {
		if (!offscreen) {
			//Currently not used.
			HBRUSH blue_brush = CreateSolidBrush(RGB(25, 60, 124));
			HBRUSH red_brush = CreateSolidBrush(RGB(124, 60, 25));
			//HBRUSH foo = SelectBrush(hdc, red_brush);

			SelectObject(hdc, GetStockObject(DC_BRUSH));
			if (n == 0) {
				SetDCBrushColor(hdc, RGB(200, 35, 16));
			}
			else if (n == 1) {
				SetDCBrushColor(hdc, RGB(16, 35, 200));
			}
			else if (n == 2) {
				SetDCBrushColor(hdc, RGB(200, 200, 30));
			}
			else if (n == 3) {
				SetDCBrushColor(hdc, RGB(250, 170, 24));
			}
			else if (n == 4) {
				SetDCBrushColor(hdc, RGB(40, 205, 114));
			}
			else if (n == 5) {
				SetDCBrushColor(hdc, RGB(250, 170, 24));
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

	int mass = 5;

	float restitution = 0.3;
	float friction = 0.01;
	int bounces = 0;
	bool on_floor = false;
	bool on_floor_temp = false;
	float *floor;

	//Generates the other 4 coordinates: Bottom-Right-Front, Bottom-Right-Back, Top-Left-Front, Top-Left-Back

	Point brf{ { blb.abs_pos[0], trf.abs_pos[1], trf.abs_pos[2], 1 } };
	Point brb{ { blb.abs_pos[0], trf.abs_pos[1], blb.abs_pos[2], 1 } };
	Point tlf{ { trf.abs_pos[0], blb.abs_pos[1], trf.abs_pos[2], 1 } };
	Point tlb{ { trf.abs_pos[0], blb.abs_pos[1], blb.abs_pos[2], 1 } };

	Plane pl_top{ { trf, trb, tlb, tlf } };
	Plane pl_bottom{ { brf, brb, blb, blf } };
	Plane pl_left{ { tlf, tlb, blb, blf } };
	Plane pl_right{ { trf, brf, brb, trb } };
	Plane pl_front{ { trf, tlf, blf, brf } };
	Plane pl_back{ { trb, tlb, blb, brb } };

	vector<Plane> faces = { pl_front, pl_left, pl_bottom, pl_top, pl_right, pl_back  };
	//vector<Plane> faces = { pl_left, pl_right };
	vector<Point> points = { trf, trb, blb, blf, brf, brb, tlf, tlb };

	vector<float> velocity = { 30, 0, 0 };

	void initialise_planes() {
		for (int m = 0; m < faces.size(); m++) {
			faces[m].parent_obloid = this;
			faces[m].initialisePoints();
			faces[m].points_size = faces[m].points.size();
			faces[m].closest_point = points[0].pos[2] + points[1].pos[2] + points[2].pos[2];
			faces[m].furthest_point = points[0].pos[2];
		}
	}

	/* Sorts planes from furthest to nearest. */
	void order_planes() {
		bool ordered = false;
		int swaps = 1;
		int max = 0;
		int place = 0;
		while (!ordered) {
			if (place >= faces.size()) {
				ordered = true;
			}

			max = place;

			for (int i = place + 1; i < faces.size(); i++) {
				if (faces[i].depth > faces[max].depth) {
					max = i;
					swaps++;
				}
			}

			if (max != place) {
				Plane temp = faces[place];
				faces[place] = faces[max];
				faces[max] = temp;
			}
			place++;
		}
	}

	void update_faces(HDC hdc) {


		if (mass && !on_floor) {
			//if (abs(velocity[1]) > 0.2) {
				velocity[1] += GRAVITY;
				//velocity[1] -= sgn(velocity[1]) * velocity[1] * velocity[1] * AIR_RES;
				float bar = velocity[1];
				std::ostringstream new_str;
				new_str << bar;
				string doo = new_str.str();
				output(doo, true);
			//}

			if (abs(velocity[0]) < 0.2) {
				velocity[0] = 0;
			}
			else {
				velocity[0] -= sgn(velocity[0]) * velocity[0] * velocity[0] * AIR_RES;
			}
			if (abs(velocity[2]) < 0.2) {
				velocity[2] = 0;
			}
			else {
				velocity[2] -= sgn(velocity[2]) * velocity[2] * velocity[2] * AIR_RES;
			}
		}
		else if (on_floor) {
			bounces++;
			//velocity[1] = 0;

			for (int k = 0; k < faces.size(); k++) {
				//faces[k].update_points(hdc, mass);
				//faces[k].fill_plane(hdc, k);
			}
			
			if (abs(velocity[1]) < 0.6) {
				velocity[1] = 0;
				bounces = 0;
				on_floor = true;
			}
			else {
				on_floor = false;
				//velocity[1] = -10 * pow(0.6, bounces);
				velocity[1] = -velocity[1] * restitution; //* pow(0.95, bounces);
			}

			/* Applies friction */
			if (abs(velocity[0]) < 0.2) {
				velocity[0] = 0;
			}
			else {
				velocity[0] -= sgn(velocity[0]) * mass * friction;
			}
			if (abs(velocity[2]) < 0.2) {
				velocity[2] = 0;
			}
			else {
				velocity[2] -= sgn(velocity[2]) * mass * friction;
			}
		}
		//on_floor_temp = false;
		//order_planes();
		//if (draw == 7) {
			for (int k = 0; k < faces.size(); k++) {
				faces[k].update_points(hdc, mass);
				//faces[k].fill_plane(hdc, k);
			}
			faces[5].points[0].draw(hdc);
		//	draw = 0;
		//}
		//else {
		//	for (int k = 0; k < faces.size(); k++) {
		//		faces[k].update_points(hdc, mass);
		//	}
		//	draw++;
		//}

		for (int m = 0; m < faces.size(); m++) {
			for (int n = 0; n < faces[m].points.size(); n++) {
				faces[m].points[n].detect_collision();
			}
		}

		//on_floor = on_floor_temp;



		//Checks if height of object has shrunk and corrects it. This will break if you introduce moments!
		//if (trf.pos[1] - blf.pos[1] != trf.abs_pos[1] - blf.abs_pos[1]) {
		//	for (int k = 0; k < faces.size(); k++) {
		//		for (int m = 0; m < faces[k].points.size(); m++) {

		//		}
		//	}
		//}
	};
};

/* Define inherited property assignment outside class definition so that the point's obloid parent is defined first. */
void Point::assign_properties() {
	mass = parent_obloid->mass;
	on_floor = &parent_obloid->on_floor;
	on_floor_temp = &parent_obloid->on_floor_temp;
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
		Point{ { 100, 0, 300, 1 } },
		Point{ { 100, 100, 300, 1 } },
		Point{ { 500, 100, 300, 1 } },
		Point{ { 500, 0, 300, 1 } },
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


	if (SKY) {
		HBRUSH brush_sky = CreateSolidBrush(RGB(102, 158, 226));
		RECT sky_rect = { 0, 0, window_width, window_height };
		FillRect(hdc, &sky_rect, brush_sky);

		HBRUSH brush_grass = CreateSolidBrush(RGB(50, 139, 50));
		RECT grass_rect = { 0, window_height * 0.5 + (player.coords[1] * 0.1), window_width, window_height };
		FillRect(hdc, &grass_rect, brush_grass);

		HBRUSH brush_sun = CreateSolidBrush(RGB(236, 209, 70));
		RECT sun_rect = { 200, 200, 300, 300 };
		FillRect(hdc, &sun_rect, brush_sun);

		DeleteObject(brush_sun);
		DeleteObject(brush_grass);
		DeleteObject(brush_sky);
	}
	else {
		HBRUSH brush_white = CreateSolidBrush(RGB(255, 255, 255));
		FillRect(hdc, prc, brush_white);
		DeleteObject(brush_white);
	}
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
			player.vel[0] = -player.masterVel;
		}
		else if (GetAsyncKeyState('D') < 0) {
			player.vel[0] = player.masterVel;
		}
		else {
			player.vel[0] = 0;
		}

		if (GetAsyncKeyState('W') < 0) {
			player.vel[2] = player.masterVel;
		}
		else if (GetAsyncKeyState('S') < 0) {
			player.vel[2] = -player.masterVel;
		}
		else {
			player.vel[2] = 0;
		}

		if (GetAsyncKeyState(0x20) < 0) {
			player.vel[1] = player.masterVel;
		}
		else if (GetAsyncKeyState(0xA0) < 0) {
			player.vel[1] = -player.masterVel;
		}
		else {
			player.vel[1] = 0;
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
		player.pos = multiply(rotXMat, player.pos);

		//Translates the player's position matrix, to subsequently translate other objects in the universe.
		player.pos[0][3] -= player.vel[0];
		player.pos[1][3] += player.vel[1];
		player.pos[2][3] -= player.vel[2];

		//Keeps track of player's universe coordinates relative to starting position.
		player.coords[0] += (player.vel[0] * cos(player.masterAng) + player.vel[2] * -sin(player.masterAng));
		player.coords[1] += player.vel[1];
		player.coords[2] += (player.vel[0] * sin(player.masterAng) + player.vel[2] * cos(player.masterAng));

		/*std::ostringstream msg;
		msg << "Position is: " << player.coords[0] << ", " << player.coords[1] << ", " << player.coords[2] << " and angle is " << player.masterAng << "\n";*/

		player.setAng(player.masterAng);
		/*string s(msg.str());
		OutputDebugStringA(s.c_str());*/


		//clear(hdc, &window_rect);
		for (int ob = 0; ob < obloid_list.size(); ob++) {
			obloid_list[ob].update_faces(hdc);
		}


		foo.pos = apply(player.pos, foo.abs_pos);
		foo.update(false);
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


