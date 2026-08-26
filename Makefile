test:
	make vmon & sleep 1 && make run

run:
	env DISPLAY=:1 ./build/manggis-session

vmon:
	Xephyr -screen 1000x600 -reset -terminate -ac :1

configure:
	mkdir /tmp/manggis-build && cmake -B /tmp/manggis-build -DCMAKE_BUILD_TYPE=Release

build: configure
	cmake --build /tmp/manggis-build

install-bin: build
	sudo cp /tmp/manggis-build/manggis-session /usr/local/bin/manggis-session

install-desktop:
	sudo cp runtime/manggis-session.desktop /usr/share/xsessions/manggis-session.desktop

install: install-bin install-desktop
