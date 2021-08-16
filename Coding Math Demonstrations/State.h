#pragma once
#include "olcPixelGameEngine.h"

class State {
public:
	State() = default;
	virtual void Draw(olc::PixelGameEngine &pge) = 0;
	virtual ~State() = 0;
};

class Rotate_Line_State : public State {
public:
	olc::vi2d origin;
	olc::vi2d end_one;
	olc::vi2d end_two;
	Rotate_Line_State(const olc::vi2d& aspect_ratio) {
		origin = { aspect_ratio.x / 2, aspect_ratio.y / 2 };
		end_one = { origin.x + 100, origin.y - 100 };
		end_two = { end_one.x, end_one.y };
	}

	void Draw(olc::PixelGameEngine &pge) override {
		pge.DrawLine(origin, end_one);
		pge.DrawLine(origin, end_two);
	}

	~Rotate_Line_State() {

	}


};
