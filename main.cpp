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

    hid_device *pro_handle = hid_open(0x057E, 0x2009, NULL);
	hid_device *r_handle = hid_open(0x57E, 0x2007, NULL);

	SwitchController controller(r_handle);
	controller.set_input_report_mode(InputReportMode::STANDARD);
	controller.set_imu_enabled(true);
	controller.request_device_info();
	controller.request_stick_calibration();
	controller.request_color_data();

	controller.enable_ringcon();

	HDRumbleConfig rumble;
	rumble.right.low.amplitude = 0.9;
	rumble.right.low.frequency = 140.0;

	rumble.right.high.amplitude = 0.3;
	rumble.right.high.frequency = 110.0;

	HDRumbleConfig no_rumble;

	while (true)
	{
		controller.poll();

		auto ls = controller.get_stick(SwitchController::Stick::LEFT);
		auto rs = controller.get_stick(SwitchController::Stick::RIGHT);

		printf("Flex: %.2lf\n", controller.get_ringcon_flex());

		if (controller.get_ringcon_flex() > 0.65) {
			controller.rumble(rumble);
		} else {
			controller.rumble(no_rumble);
		}
		
	}

	hid_close(handle);

	res = hid_exit();
}