make: main.cpp
	clang++ -dynamiclib -undefined suppress -Iinclude src/imu.cpp src/switch_controller.cpp src/rumble.cpp -I /opt/homebrew/Cellar/hidapi/0.14.0/include -std=c++11 -o libjoycon.dylib
	clang++ -o joycon_test main.cpp libjoycon.dylib -Iinclude /opt/homebrew/Cellar/hidapi/0.14.0/lib/libhidapi.dylib -I /opt/homebrew/Cellar/hidapi/0.14.0/include -std=c++11

# switch_controller: switch_controller.cpp switch_controller.h