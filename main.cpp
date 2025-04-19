#include <stdio.h>
#include <iostream>
#include <hidapi/hidapi.h>

#include "switch_controller.h"
// #include "joycon_report.h"

#define MAX_STR 256

// To run: make sure to run
// export DYLD_LIBRARY_PATH=/usr/local/lib
// before trying to run the program
// (or, better, set up CMake so it works)

int main(void)
{
	int res;
	unsigned char buf[361];
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	// Initialize the hidapi library
	res = hid_init();

	SwitchProController controller;
	controller.request_stick_calibration();

	while (true)
	{
		controller.poll();

		auto ls = controller.get_stick(SwitchProController::Stick::LEFT);
		auto rs = controller.get_stick(SwitchProController::Stick::RIGHT);

		printf("LS: %.2f, %.2f\t\tRS: %.2f, %.2f\n", ls.x, ls.y, rs.x, rs.y);
	}

	res = hid_exit();
}