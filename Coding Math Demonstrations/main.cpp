#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <stdlib.h>
#include <vector>

#define PI 3.141592
#define rad_constant PI/180.0f

struct vertices {
	olc::vf2d verts[3];
};

struct Rotate_Line_State {
	olc::vi2d origin;
	olc::vi2d point;
	olc::vi2d rotated;
};

float lerp(float norm, float min, float max) {
	return (max - min) * norm + min;
}

struct Gun {
	int x; 
	int y;
	float angle;
};


olc::vi2d Convert_To_Tile_Position(olc::vf2d& pixel_cord, int cell_size) {
	olc::vf2d tile_cordf = pixel_cord / cell_size;
	olc::vi2d tile_cordi = tile_cordf;
	return tile_cordi;
}

struct Node {
	bool b_obstacle = false;
	bool b_visited = false;
	float f_global_goal;
	float f_local_goal;
	int x;
	int y;
	std::vector<Node*> vec_neighbors;
	Node* parent;
};


// Override base class with your custom functionality
class CodingMath : public olc::PixelGameEngine
{
public:
	CodingMath()
	{
		// Name your application
		sAppName = "Spline Editor";
	}

	int demon;
	int total_demon;
	olc::vi2d origin;
	olc::vi2d mouse_pos;
	std::stringstream s;
	int vertical_scale;
	float horizontal_scale;
	vertices tri_vert;
	olc::vi2d rotated;
	float angle = 0.0f;
	float t;
	Gun gun;
	olc::vf2d tile_player = { 0, 0 };
	static const int size_x = 32;
	static const int size_y = 30;
	olc::vi2d tile_map_size = { 32, 30 };
	olc::vi2d cell_size = { 16, 16 };
	std::vector<int> tile_map;
	Node nodes[size_x * size_y];
	Node* node_end = nullptr;
	Node* node_start = nullptr;

public:
	bool OnUserCreate() override
	{
		demon = 0;
		total_demon = 12;
		origin = { ScreenWidth() / 2, ScreenHeight() / 2 };
		s.str("");
		vertical_scale = 30;
		horizontal_scale = 0.0f;
		tri_vert = { olc::vf2d(0.0f, -30.0f), olc::vf2d(30.0f, 30.0f), olc::vf2d(-30.0f, 30.0f)};
		t = 0.0f;
		gun.x = 100; gun.y = 100; gun.angle = 0.0f;
		tile_map.resize(tile_map_size.x * tile_map_size.y);
		for (int y = 0; y < tile_map_size.y; y++) {
			for (int x = 0; x < tile_map_size.x; x++) {
				nodes[y * tile_map_size.x + x].x = x;
				nodes[y * tile_map_size.x + x].y = y;
				nodes[y * tile_map_size.x + x].b_obstacle = false;
				nodes[y * tile_map_size.x + x].parent = nullptr;
				nodes[y * tile_map_size.x + x].b_visited = false;
			}
		}

		for (int x = 0; x < tile_map_size.x; x++) {
			for (int y = 0; y < tile_map_size.y; y++)
			{
				if (y > 0) {
					nodes[y * tile_map_size.x + x].vec_neighbors.push_back(&nodes[(y - 1) * tile_map_size.x + (x + 0)]);
				}
				if (y < tile_map_size.y - 1) {
					nodes[y * tile_map_size.x + x].vec_neighbors.push_back(&nodes[(y + 1) * tile_map_size.x + (x + 0)]);
				}
				if (x > 0) {
					nodes[y * tile_map_size.x + x].vec_neighbors.push_back(&nodes[(y + 0) * tile_map_size.x + (x - 1)]);
				}
				if (x < tile_map_size.x - 1) {
					nodes[y * tile_map_size.x + x].vec_neighbors.push_back(&nodes[(y + 0) * tile_map_size.x + (x + 1)]);
				}
			}
		}
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		if (GetKey(olc::Key::TAB).bReleased) {
			demon++;
			if (demon == total_demon) {
				demon = 0;
			}
		}

		

		switch (demon) {
		case 0: {
			for (int i = 0; i < 50; i++) {
				DrawLine({ rand() % ScreenWidth() + 1, rand() % ScreenHeight() + 1 },
					{ rand() % ScreenWidth() + 1, rand() % ScreenHeight() + 1 });
			}
		} break;
		case 1: {
			//Trig

			mouse_pos = { GetMouseX(), GetMouseY() };

			olc::vf2d line = { float(mouse_pos.x - origin.x), float(mouse_pos.y - origin.y) };
			float l = sqrtf(line.x * line.x + line.y * line.y);
			line.x /= l; line.y /= l;

			s << "Length: " << l << std::endl;
			s << "Sin: " << line.y << std::endl;
			s << "Cosine: " << line.x;
			DrawString({ 4, 4 }, s.str());

			DrawLine({ ScreenWidth() / 2, 0 }, { ScreenWidth() / 2, ScreenHeight() });
			DrawLine({ 0, ScreenHeight() / 2 }, { ScreenWidth(), ScreenHeight() / 2 });
			DrawLine(origin, mouse_pos, olc::GREEN);
			s.str("");


		} break;
		case 2: {

			mouse_pos = { GetMouseX(), GetMouseY() };
			//Circle
			olc::vf2d line = { float(mouse_pos.x - origin.x), float(mouse_pos.y - origin.y) };
			float l = sqrtf(line.x * line.x + line.y * line.y);
			s << "Circumference: " << 2 * PI * l << std::endl;
			s << "Area: " << PI * l * l;
			DrawString({ 4, 4 }, s.str());
			DrawCircle(origin, l, olc::GREEN);
			DrawLine({ ScreenWidth() / 2, 0 }, { ScreenWidth() / 2, ScreenHeight() });
			DrawLine({ 0, ScreenHeight() / 2 }, { ScreenWidth(), ScreenHeight() / 2 });
			DrawLine(origin, mouse_pos, olc::GREEN);
			s.str("");
		} break;
		case 3: {
			if (GetKey(olc::RIGHT).bHeld) {
				horizontal_scale += 50.0f * fElapsedTime;
			}

			if (GetKey(olc::LEFT).bHeld) {
				horizontal_scale -= 50.0f * fElapsedTime;
			}

			if (GetKey(olc::UP).bPressed) {
				vertical_scale += 1;
			}

			if (GetKey(olc::DOWN).bPressed && vertical_scale > 5) {
				vertical_scale -= 1;
			}

			mouse_pos = { GetMouseX(), GetMouseY() };
			float start_angle = 0.0f + horizontal_scale;

			int mid = ScreenHeight() / 2;
			float rad_temp = PI / 180.0f * (mouse_pos.x + horizontal_scale);
			olc::vi2d dot_center = olc::vi2d(mouse_pos.x, mid + (sinf(rad_temp) * vertical_scale));
			FillRect(dot_center.x, dot_center.y, 4, 4, olc::GREEN);
			s << "Sin Graph: " << std::endl;
			s << "Start: " << start_angle << " Rad: " << rad_constant * start_angle << std::endl;
			s << "Current Rad: " << rad_temp;
			DrawString({ 4, 4 }, s.str());
			DrawLine({ 0, ScreenHeight() / 2 }, { ScreenWidth(), ScreenHeight() / 2 });

			for (float i = start_angle; i < float(ScreenWidth()) + horizontal_scale; i += 1.0f) {
				float rad = PI / 180.0f * i;
				Draw((int32_t)i - horizontal_scale, mid + (sinf(rad) * vertical_scale));
			}
			s.str("");
		} break;
		case 4: {
			if (GetKey(olc::RIGHT).bHeld) {
				horizontal_scale += 50.0f * fElapsedTime;
			}

			if (GetKey(olc::LEFT).bHeld) {
				horizontal_scale -= 50.0f * fElapsedTime;
			}

			if (GetKey(olc::UP).bPressed) {
				vertical_scale += 1;
			}

			if (GetKey(olc::DOWN).bPressed && vertical_scale > 5) {
				vertical_scale -= 1;
			}
			float start_angle = 0.0f + horizontal_scale;

			s << "Cos Graph: " << std::endl;
			s << "Start at Angle: " << start_angle << " Rad: " << rad_constant * start_angle;
			DrawString({ 4, 4 }, s.str());
			DrawLine({ 0, ScreenHeight() / 2 }, { ScreenWidth(), ScreenHeight() / 2 });
			int mid = ScreenHeight() / 2;
			for (float i = start_angle; i < float(ScreenWidth()) + horizontal_scale; i += 1.0f) {
				float rad = PI / 180.0f * i;
				Draw((int32_t)i - horizontal_scale, mid + (cosf(rad) * vertical_scale));
			}
			s.str("");
		} break;
		case 5: {
			if (GetKey(olc::UP).bHeld) {
				horizontal_scale += 50.0f * fElapsedTime;
			}

			if (GetKey(olc::DOWN).bHeld) {
				horizontal_scale -= 50.0f * fElapsedTime;
			}

			if (GetKey(olc::RIGHT).bHeld) {
				angle += 1.0f * fElapsedTime;
			}

			if (GetKey(olc::LEFT).bHeld) {
				angle -= 1.0f * fElapsedTime;
			}
			int mid = ScreenHeight() / 2;
			olc::vf2d model_origin = olc::vf2d(horizontal_scale, mid + cosf(PI / 180.0f * horizontal_scale) * 30);
			for (int i = 0; i < 3; i++) {
				float l = (tri_vert.verts[i].x * tri_vert.verts[i].x + tri_vert.verts[i].y * tri_vert.verts[i].y);
				tri_vert.verts[i].x /= l; tri_vert.verts[i].y /= l;
				float start_x = tri_vert.verts[i].x; float start_y = tri_vert.verts[i].y;
				olc::vf2d rotated_v = olc::vf2d(start_x * cosf(angle) - start_y * sinf(angle), start_x * sinf(angle) + start_y * cosf(angle));
				rotated_v.x *= l;
				rotated_v.y *= l;
				DrawLine({ (int32_t)model_origin.x, (int32_t)model_origin.y },
					{ int32_t(model_origin.x + start_x), int32_t(model_origin.y + start_y) });
			}
			DrawLine({ (int32_t)model_origin.x, (int32_t)model_origin.y },
				{ int32_t(model_origin.x + tri_vert.verts[0].x), int32_t(model_origin.y + tri_vert.verts[0].y) });
			DrawLine({ (int32_t)model_origin.x, (int32_t)model_origin.y },
				{ int32_t(model_origin.x + tri_vert.verts[1].x), int32_t(model_origin.y + tri_vert.verts[1].y) });
			DrawLine({ (int32_t)model_origin.x, (int32_t)model_origin.y },
				{ int32_t(model_origin.x + tri_vert.verts[2].x), int32_t(model_origin.y + tri_vert.verts[2].y) });
			for (float i = 0.0f; i < float(ScreenWidth()); i += 1.0f) {
				float rad = PI / 180.0f * i;
				Draw((int32_t)i, mid + (cosf(rad) * 30));

			}
		} break;
		case 6: {
			float alpha = lerp(t, 255, 0);
			if (GetMouse(0).bReleased) {
				mouse_pos.x = GetMouseX();
				mouse_pos.y = GetMouseY();
				t = 0.0f;
			}
			DrawString({ 4, 4 }, "Lerped circle");
			FillCircle({ mouse_pos.x, mouse_pos.y }, (int32_t)lerp(t, 1, ScreenWidth() / 2), olc::Pixel(255, 255, 255, alpha));
			t += 0.2f * fElapsedTime;
			if (t > 1.0f) {
				t = 0.0f;
			}
		} break;

		case 7: {
			olc::vf2d base_line = olc::vi2d(origin.x + 100, origin.y + 100);
			DrawLine(origin, { (int32_t)base_line.x, (int32_t)base_line.y });
			float length = sqrt(100 * 100 + 100 * 100);
			float x = 100 / length; float y = 100 / length;
			if (GetKey(olc::Key::LEFT).bHeld) {
				angle -= 5.0f * fElapsedTime;
			}

			if (GetKey(olc::Key::RIGHT).bHeld) {
				angle += 5.0f * fElapsedTime;
			}
			float new_x = x * cosf(angle) - y * sinf(angle);
			float new_y = x * sinf(angle) + y * cosf(angle);
			olc::vf2d new_v = olc::vf2d(new_x * length, new_y * length);
			DrawString({ 4, 4 }, "Rotating Line");
			DrawLine(origin, { origin.x + (int32_t)new_v.x, origin.y + (int32_t)new_v.y });

		} break;

		case 8: {
			if (GetKey(olc::Key::LEFT).bHeld) {
				gun.angle -= 1.0f * fElapsedTime;
			}

			if (GetKey(olc::Key::RIGHT).bHeld) {
				gun.angle += 1.0f * fElapsedTime;
			}

			//We start with a hypothetical unit vector so we don't need to multiply by x
			float x_rotate = cosf(gun.angle) - sinf(gun.angle);
			float y_rotate = sinf(gun.angle) + cosf(gun.angle);

			olc::vf2d cannon = olc::vf2d(40 * x_rotate, 40 * y_rotate);

			DrawString({ 4, 4 }, "Rotating 2D Plane");


			FillCircle({ gun.x, gun.y }, 24);

			for (int i = -10; i < 10; i++) {
				DrawLine({ gun.x, gun.y + i }, { gun.x + (int32_t)cannon.x, gun.y + (int32_t)cannon.y + i });
			}

		} break;

		case 9: {
			std::stringstream s;
			s.str("");
			olc::vf2d mouse_f = { float(GetMouseX()), float(GetMouseY()) };
			//Mouse position in cells
			olc::vf2d mouse_cellf = mouse_f / cell_size;
			//Mouse position in int cells
			olc::vi2d mouse_celli = mouse_cellf; //cast cell to int

			//Take mouse position in cells to get to stuff
			if (GetKey(olc::Key::P).bHeld) {
				int cell = mouse_celli.y * tile_map_size.x + mouse_celli.x;
				tile_map[cell] = 1;
			}

			if (GetKey(olc::Key::LEFT).bHeld) {
				tile_player.x -= 25.0f * fElapsedTime;
			}

			if (GetKey(olc::Key::RIGHT).bHeld) {
				tile_player.x += 25.0f * fElapsedTime;
			}

			if (GetKey(olc::Key::DOWN).bHeld) {
				tile_player.y += 25.0f * fElapsedTime;
			}

			if (GetKey(olc::Key::UP).bHeld) {
				tile_player.y -= 25.0f * fElapsedTime;
			}


			//Ray cast from player to mouse
			olc::vf2d ray_origin = tile_player;
			//Normalize the ray
			olc::vf2d ray_dir = (mouse_cellf - tile_player).norm();

			olc::vf2d ray_step_size =
			{ sqrt(1 + (ray_dir.y / ray_dir.x) * (ray_dir.y / ray_dir.x)),
				sqrt(1 + (ray_dir.x / ray_dir.y) * (ray_dir.x / ray_dir.y)) };

			//Int because we are checking which cell we are in, and cells are represented in integer coordinates
			olc::vi2d map_check = ray_origin;
			//length of ray in colums and rows. This value will be accumulated over time
			olc::vf2d ray_length_1d;
			olc::vi2d step;


			//Determine whether we move forwards or backwards when checking cells in either the x or y direction
			if (ray_dir.x < 0) {
				step.x = -1;
				//We perform this calculation to determine how much of the ray's pixel oirgin

				ray_length_1d.x = (ray_origin.x - float(map_check.x)) * ray_step_size.x;
			}
			else {
				step.x = 1;
				//If ray is positive we check how long it takes for ray to exit cell
				//Why I don't know.
				ray_length_1d.x = (float(map_check.x + 1) - ray_origin.x) * ray_step_size.x;

			}

			if (ray_dir.y < 0) {
				step.y = -1;
				//Since the origin of the ray and map check are the same, then why perform this calculation?
				ray_length_1d.y = (ray_origin.y - float(map_check.y)) * ray_step_size.y;
			}
			else {
				step.y = 1;
				ray_length_1d.y = (float(map_check.y + 1) - ray_origin.y) * ray_step_size.y;

			}

			bool tile_found = false;
			float max_distance = 100.0f;
			float cur_distance = 0.0f;

			while (!tile_found && cur_distance < max_distance) {
				if (ray_length_1d.x < ray_length_1d.y) {
					map_check.x += step.x;
					cur_distance = ray_length_1d.x;
					ray_length_1d.x += ray_step_size.x;

				}
				else {
					map_check.y += step.y;
					cur_distance = ray_length_1d.y;
					ray_length_1d.y += ray_step_size.y;

				}

				if (map_check.x >= 0 && map_check.x < tile_map_size.x &&
					map_check.y >= 0 && map_check.y < tile_map_size.y) {
					if (tile_map[map_check.y * tile_map_size.x + map_check.x] == 1) {
						tile_found = true;
					}
				}
			}

			olc::vf2d intersect;
			if (tile_found) {
				intersect = ray_origin + ray_dir * cur_distance;
			}



			for (int y = 0; y < tile_map_size.y; y++) {
				for (int x = 0; x < tile_map_size.x; x++) {
					int cell = tile_map[y * tile_map_size.x + x];
					if (cell == 1) {
						FillRect(olc::vi2d(x, y) * cell_size, cell_size, olc::BLUE);
					}
					DrawRect(olc::vi2d(x, y) * cell_size, cell_size, olc::DARK_GREY);
				}
			}

			if (GetMouse(0).bHeld) {
				//We don't multiply by the cell size to make the line always originate in the center of a cell
				//Rather, this makes sure that whatever constant speed we've defined for player movement
				//Is always defined by the size of the cell, irrespective of what that speed may 
				//intially be
				DrawLine(tile_player * cell_size, mouse_f, olc::WHITE);
				//DrawLine(tile_player, mouse_pos);

				if (tile_found) {
					DrawCircle(intersect * cell_size, 2);
				}
			}

			s << "Normalized Ray Length: " << ray_dir.x << ", " << ray_dir.y << std::endl;
			s << "Normalized Step Unit: " << ray_step_size.x << ", " << ray_step_size.y << std::endl;
			s << "Real Step Unit X: " << ray_length_1d.x;

			DrawString({ 4, 4 }, s.str(), olc::YELLOW);
			s.str("");

			FillCircle(tile_player * cell_size, 4, olc::RED);
			FillCircle(mouse_f, 4, olc::GREEN);

		} break;

		case 10: {
			std::stringstream s;
			s.str("");
			olc::vf2d mouse_f = { float(GetMouseX()), float(GetMouseY()) };
			//Mouse position in cells
			olc::vf2d mouse_cellf = mouse_f / cell_size;
			//Mouse position in int cells
			olc::vi2d mouse_celli = mouse_cellf; //cast cell to int

			//Take mouse position in cells to get to stuff
			if (GetKey(olc::Key::P).bHeld) {
				int cell = mouse_celli.y * tile_map_size.x + mouse_celli.x;
				tile_map[cell] = 1;
			}

			//Ray cast from player to mouse
			olc::vf2d ray_origin = tile_player;
			//Normalize the ray
			olc::vf2d ray_dir = (mouse_cellf - tile_player).norm();

			olc::vf2d ray_step_size =
			{ sqrt(1 + (ray_dir.y / ray_dir.x) * (ray_dir.y / ray_dir.x)),
				sqrt(1 + (ray_dir.x / ray_dir.y) * (ray_dir.x / ray_dir.y)) };

			//Int because we are checking which cell we are in, and cells are represented in integer coordinates
			olc::vi2d map_check = ray_origin;
			//length of ray in colums and rows. This value will be accumulated over time
			olc::vf2d ray_length_1d;
			olc::vi2d step;


			//Determine whether we move forwards or backwards when checking cells in either the x or y direction
			if (ray_dir.x < 0) {
				step.x = -1;
				//We perform this calculation to determine how much of the ray's pixel oirgin

				ray_length_1d.x = (ray_origin.x - float(map_check.x)) * ray_step_size.x;
			}
			else {
				step.x = 1;
				//If ray is positive we check how long it takes for ray to exit cell
				//Why I don't know.
				ray_length_1d.x = (float(map_check.x + 1) - ray_origin.x) * ray_step_size.x;

			}

			if (ray_dir.y < 0) {
				step.y = -1;
				//Since the origin of the ray and map check are the same, then why perform this calculation?
				ray_length_1d.y = (ray_origin.y - float(map_check.y)) * ray_step_size.y;
			}
			else {
				step.y = 1;
				ray_length_1d.y = (float(map_check.y + 1) - ray_origin.y) * ray_step_size.y;

			}

			bool tile_found = false;
			float max_distance = 100.0f;
			float cur_distance = 0.0f;

			while (!tile_found && cur_distance < max_distance) {
				if (ray_length_1d.x < ray_length_1d.y) {
					map_check.x += step.x;
					cur_distance = ray_length_1d.x;
					ray_length_1d.x += ray_step_size.x;

				}
				else {
					map_check.y += step.y;
					cur_distance = ray_length_1d.y;
					ray_length_1d.y += ray_step_size.y;

				}

				if (map_check.x >= 0 && map_check.x < tile_map_size.x &&
					map_check.y >= 0 && map_check.y < tile_map_size.y) {
					if (tile_map[map_check.y * tile_map_size.x + map_check.x] == 1) {
						tile_found = true;
					}
				}
			}

			olc::vf2d intersect;
			if (tile_found) {
				intersect = ray_origin + ray_dir * cur_distance;
				if (ray_dir.x < 0 && intersect.x < mouse_cellf.x) {
					tile_player.x += ray_dir.x * 10.0f * fElapsedTime;
					tile_player.y += ray_dir.y * 10.0f * fElapsedTime;
				}

				if (ray_dir.x > 0 && intersect.x > mouse_cellf.x) {
					tile_player.x += ray_dir.x * 10.0f * fElapsedTime;
					tile_player.y += ray_dir.y * 10.0f * fElapsedTime;
				}
			} else {
				tile_player.x += ray_dir.x * 10.0f * fElapsedTime;
				tile_player.y += ray_dir.y * 10.0f * fElapsedTime;
			}



			for (int y = 0; y < tile_map_size.y; y++) {
				for (int x = 0; x < tile_map_size.x; x++) {
					int cell = tile_map[y * tile_map_size.x + x];
					if (cell == 1) {
						FillRect(olc::vi2d(x, y) * cell_size, cell_size, olc::BLUE);
					}
					DrawRect(olc::vi2d(x, y) * cell_size, cell_size, olc::DARK_GREY);
				}
			}

			if (GetMouse(0).bHeld) {
				//We don't multiply by the cell size to make the line always originate in the center of a cell
				//Rather, this makes sure that whatever constant speed we've defined for player movement
				//Is always defined by the size of the cell, irrespective of what that speed may 
				//intially be
				DrawLine(tile_player * cell_size, mouse_f, olc::WHITE);
				//DrawLine(tile_player, mouse_pos);

				if (tile_found) {
					DrawCircle(intersect * cell_size, 2);
				}
			}

			s << "Normalized Ray Length: " << ray_dir.x << ", " << ray_dir.y << std::endl;
			s << "Normalized Step Unit: " << ray_step_size.x << ", " << ray_step_size.y << std::endl;
			s << "Real Step Unit X: " << ray_length_1d.x;

			DrawString({ 4, 4 }, s.str(), olc::YELLOW);
			s.str("");

			FillCircle(tile_player * cell_size, 4, olc::RED);
			FillCircle(mouse_f, 4, olc::GREEN);
		} break;

		case 11: {
			std::stringstream s;
			s.str("");
			olc::vf2d mouse_f = { float(GetMouseX()), float(GetMouseY()) };
			//Mouse position in cells
			olc::vf2d mouse_cellf = mouse_f / cell_size;
			//Mouse position in int cells
			olc::vi2d mouse_celli = mouse_cellf; //cast cell to int

			node_end = &nodes[mouse_celli.y * tile_map_size.x + mouse_celli.x];

			//Take mouse position in cells to get to stuff
			if (GetKey(olc::Key::P).bHeld) {
				int cell = mouse_celli.y * tile_map_size.x + mouse_celli.x;
				tile_map[cell] = 1;
			}

			//Ray cast from player to mouse
			olc::vf2d ray_origin = tile_player;
			//Normalize the ray
			olc::vf2d ray_dir = (mouse_cellf - tile_player).norm();

			olc::vf2d ray_step_size =
			{ sqrt(1 + (ray_dir.y / ray_dir.x) * (ray_dir.y / ray_dir.x)),
				sqrt(1 + (ray_dir.x / ray_dir.y) * (ray_dir.x / ray_dir.y)) };

			//Int because we are checking which cell we are in, and cells are represented in integer coordinates
			olc::vi2d map_check = ray_origin;
			//length of ray in colums and rows. This value will be accumulated over time
			olc::vf2d ray_length_1d;
			olc::vi2d step;


			//Determine whether we move forwards or backwards when checking cells in either the x or y direction
			if (ray_dir.x < 0) {
				step.x = -1;
				//We perform this calculation to determine how much of the ray's pixel oirgin

				ray_length_1d.x = (ray_origin.x - float(map_check.x)) * ray_step_size.x;
			}
			else {
				step.x = 1;
				//If ray is positive we check how long it takes for ray to exit cell
				//Why I don't know.
				ray_length_1d.x = (float(map_check.x + 1) - ray_origin.x) * ray_step_size.x;

			}

			if (ray_dir.y < 0) {
				step.y = -1;
				//Since the origin of the ray and map check are the same, then why perform this calculation?
				ray_length_1d.y = (ray_origin.y - float(map_check.y)) * ray_step_size.y;
			}
			else {
				step.y = 1;
				ray_length_1d.y = (float(map_check.y + 1) - ray_origin.y) * ray_step_size.y;

			}

			bool tile_found = false;
			float max_distance = 100.0f;
			float cur_distance = 0.0f;

			while (!tile_found && cur_distance < max_distance) {
				if (ray_length_1d.x < ray_length_1d.y) {
					map_check.x += step.x;
					cur_distance = ray_length_1d.x;
					ray_length_1d.x += ray_step_size.x;

				}
				else {
					map_check.y += step.y;
					cur_distance = ray_length_1d.y;
					ray_length_1d.y += ray_step_size.y;

				}

				if (map_check.x >= 0 && map_check.x < tile_map_size.x &&
					map_check.y >= 0 && map_check.y < tile_map_size.y) {
					if (tile_map[map_check.y * tile_map_size.x + map_check.x] == 1) {
						tile_found = true;
					}
				}
			}

			olc::vf2d intersect;
			if (tile_found) {
				intersect = ray_origin + ray_dir * cur_distance;
				if (ray_dir.x < 0 && intersect.x < mouse_cellf.x) {
					tile_player.x += ray_dir.x * 5.0f * fElapsedTime;
					tile_player.y += ray_dir.y * 5.0f * fElapsedTime;
				} else if (ray_dir.x > 0 && intersect.x > mouse_cellf.x) {
					tile_player.x += ray_dir.x * 5.0f * fElapsedTime;
					tile_player.y += ray_dir.y * 5.0f * fElapsedTime;
				} else {

				}
			} else {
				tile_player.x += ray_dir.x * 5.0f * fElapsedTime;
				tile_player.y += ray_dir.y * 5.0f * fElapsedTime;
			}



			for (int y = 0; y < tile_map_size.y; y++) {
				for (int x = 0; x < tile_map_size.x; x++) {
					int cell = tile_map[y * tile_map_size.x + x];
					if (cell == 1) {
						FillRect(olc::vi2d(x, y) * cell_size, cell_size, olc::BLUE);
					}
					DrawRect(olc::vi2d(x, y) * cell_size, cell_size, olc::DARK_GREY);
				}
			}

			if (GetMouse(0).bHeld) {
				//We don't multiply by the cell size to make the line always originate in the center of a cell
				//Rather, this makes sure that whatever constant speed we've defined for player movement
				//Is always defined by the size of the cell, irrespective of what that speed may 
				//intially be
				DrawLine(tile_player * cell_size, mouse_f, olc::WHITE);
				//DrawLine(tile_player, mouse_pos);

				if (tile_found) {
					DrawCircle(intersect * cell_size, 2);
				}
			}

			s << "Mouse Cell: " << mouse_celli.x << ", " << mouse_celli.y;
			s << "Node Cell: " << nodes[mouse_celli.y * tile_map_size.x + mouse_celli.y].x << ", " << nodes[mouse_celli.y * tile_map_size.x + mouse_celli.y].y;

			DrawString({ 4, 4 }, s.str(), olc::YELLOW);
			s.str("");

			FillCircle(tile_player * cell_size, 4, olc::RED);
			FillCircle(mouse_f, 4, olc::GREEN);
		} break;

		} 
		
		return true;
		
		
	}

};
int main()
{
	CodingMath demo;
	if (demo.Construct(512, 480, 2, 2))
		demo.Start();
	return 0;
}