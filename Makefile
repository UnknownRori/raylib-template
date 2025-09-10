# Set the CMAKE_FLAGS
PROJECT_NAME = huntress
CMAKE_FLAGS = -G "MSYS Makefiles"

all: desktop
	./build/$(PROJECT_NAME)

clean:
	rm -r ./build/

clean-web:
	rm -r ./dist
	rm -r ./web-build

desktop:
	([ ! -e ./build ] && cmake $(CMAKE_FLAGS) -S . -B build) || [ -e ./build ]
	cmake --build build  -j 6
	cp -r ./resources/ ./build/

desktop-release:
	([ ! -e ./build ] && cmake $(CMAKE_FLAGS) -S . -B build) || [ -e ./build ]
	cmake --build build --config Release  -j 6
	cp -r ./resources/ ./build/

web:
	([ ! -e ./web-build ] && emcmake cmake -S . -B web-build -DPLATFORM=Web) || [ -e ./web-build ]
	([ ! -e ./dist ] && mkdir dist) || [ -e ./dist ]
	cmake --build web-build  -j 6
	cp -r ./resources ./dist
	cp ./index.html ./dist/index.html
	cp ./web-build/$(PROJECT_NAME).js ./dist
	cp ./web-build/$(PROJECT_NAME).data ./dist
	cp ./web-build/$(PROJECT_NAME).wasm ./dist
