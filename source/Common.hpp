#pragma once

struct Rect {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

struct Point {
	int x = 0;
	int y = 0;
};

struct MinMax {
	int min;
	int max;
};

struct Constrain {
	MinMax width;
	MinMax height;
};
