run:
	env DISPLAY=:1 ./build/manggis-session

vmon:
	Xephyr -screen 800x600 -reset -terminate -ac :1

