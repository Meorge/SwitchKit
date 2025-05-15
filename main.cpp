#include <stdio.h>
#include <iostream>
#include <hidapi/hidapi.h>
#include <chrono>

#include "switch_controller.h"
// #include "joycon_report.h"
#include "vectors.h"

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

	handle = r_handle;

	SwitchKit::SwitchController controller(handle);
	controller.set_input_report_mode(SwitchKit::InputReportMode::MODE_STANDARD);
	controller.set_imu_enabled(true);
	controller.request_device_info();
	controller.request_stick_calibration();
	controller.request_imu_calibration();
	controller.request_color_data();

	controller.set_player_lights(SwitchKit::SwitchController::LIGHT_ON, SwitchKit::SwitchController::LIGHT_OFF, SwitchKit::SwitchController::LIGHT_FLASH, SwitchKit::SwitchController::LIGHT_ON);
	controller.enable_ringcon();

	SwitchKit::HDRumbleConfig rumble;
	rumble.right.low.amplitude = 0.9;
	rumble.right.low.frequency = 140.0;

	rumble.right.high.amplitude = 0.3;
	rumble.right.high.frequency = 110.0;

	SwitchKit::HDRumbleConfig no_rumble;

	bool leave = false;
	while (!leave)
	{
		controller.poll();

		if (controller.get_button_pressed_this_frame(SwitchKit::SwitchControllerReport::BTN_A)) {
			printf("Checking if Ring-Con is connected...\n");
			auto t1 = std::chrono::high_resolution_clock::now();
			bool conn = controller.get_ringcon_connected();
			auto t2 = std::chrono::high_resolution_clock::now();
			long long int ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			if (conn) {
				printf("Ring-Con is connected! Took %lld ms\n", ms);
			} else {
				printf("Ring-Con not connected. Took %lld ms\n", ms);
			}
		}

		if (controller.get_button_pressed_this_frame(SwitchKit::SwitchControllerReport::BTN_PLUS)) {
			leave = true;
		}

		if (controller.get_button_pressed_this_frame(SwitchKit::SwitchControllerReport::BTN_RINGCON_FLEX)) {
			printf("Ring-Con pressed!\n");
			for (int i = 0; i < 10; i++)
				controller.rumble(rumble);
			controller.rumble(no_rumble);
		}
		
		// SwitchKit::Vector3 g = controller.get_gyro();
		// printf("Gyro: %.2lf, %.2lf, %.2lf\n", g.x, g.y, g.z);
		// auto ls = controller.get_stick(SwitchKit::SwitchController::Stick::LEFT);
		// auto rs = controller.get_stick(SwitchKit::SwitchController::Stick::STICK_RIGHT);

		// printf("right stick %.2lf, %.2lf\n", rs.x, rs.y);

		// printf("Flex: %.2lf\n", controller.get_ringcon_flex());

		// if (controller.get_ringcon_flex() > 0.65) {
		// 	controller.rumble(rumble);
		// } else {
		// 	controller.rumble(no_rumble);
		// }
		
	}

	hid_close(handle);
	res = hid_exit();
}