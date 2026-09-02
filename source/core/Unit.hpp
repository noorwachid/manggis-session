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
	int min = 0;
	int max = 0;
};

struct Constrain {
	MinMax width;
	MinMax height;
};

struct Margin {
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
};
