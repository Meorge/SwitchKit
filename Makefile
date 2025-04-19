make: main.cpp
	clang++ -o joycon_test main.cpp subcommand_response.cpp switch_controller.cpp /opt/homebrew/Cellar/hidapi/0.14.0/lib/libhidapi.dylib -I /opt/homebrew/Cellar/hidapi/0.14.0/include -std=c++11

# switch_controller: switch_controller.cpp switch_controller.h