# SwitchKit

SwitchKit is a C++ library for interacting with controllers for the Nintendo Switch on a PC/Mac.
It is relatively "pure", with [hidapi](https://github.com/libusb/hidapi) as the only dependency.
Functionality is listed below.

## Controller compatibility

SwitchKit is compatible with the following controllers:

- Nintendo Switch Joy-Con (L) and Joy-Con (R)
- Nintendo Switch Pro Controller

> [!WARNING]
> At this time, **SwitchKit is not compatible with Nintendo Switch 2 controllers, including the Joy-Con 2 controllers or the Nintendo Switch 2 Pro Controller.**
> 
> If you have information on how to get these controllers to interface with a PC/Mac, please let me know - I'd love to add support!

## Features

- [x] Basic input (buttons, sticks)
- [x] Battery info
- [x] HD Rumble
- [x] IMU (gyroscope, accelerometer)
- [x] SPI flash read/write (colors, calibration data)
- [x] Disconnect/power down controller
- [ ] NFC (amiibo)
- [ ] IR camera
- [x] Player lights
- [ ] HOME button light
- Ring-Con
  - [x] Ring flex
  - [x] Ring press as button
  - [ ] Ring gestures (turn for left/right, tilt for up/down)
  - [ ] Stored presses

## Bindings

- [Godot](https://github.com/Meorge/SwitchKit-Godot)

## Usage

See `main.cpp` for my testing ground of SwitchKit's features.

In general, the flow is:

1. Use `hidapi` to open a connection to a Joy-Con (L), Joy-Con (R), or Pro Controller.
   - Vendor ID is `0x057E`
   - Joy-Con (L) product ID is `0x2006`
   - Joy-Con (R) product ID is `0x2007`
   - Pro Controller product ID is `0x2009`
2. Pass the handle in to the constructor for the `SwitchKit::SwitchController` class.
3. Call setup methods on `SwitchController` instance
   - `set_input_report_mode(SwitchKit::InputReportMode::MODE_STANDARD)` enables main functionality.
   - `set_imu_enabled(true)` enables accelerometer and gyroscope.
   - `request_device_info()`
   - `request_stick_calibration()` and `request_imu_calibration()` get calibration data for the sticks and accelerometer/gyroscope respectively.
   - `request_color_data()` gets the Joy-Con/Pro Controller colors.
4. In your game loop, call `SwitchController::poll()` to receive the current controller state. This is especially important to call continuously if you are using the gyroscope, as it can only update the controller orientation based on data from the poll.

Here's a simple example program that connects to the Joy-Con (R), sets the player LEDs, and then prints the Joy-Con's orientation (via gyro) while it waits for the player to press the A button.

```cpp
#include <hidapi/hidapi.h>
#include "switch_controller.h"

int main(void)
{
  int res;

  // Initialize the hidapi library
  res = hid_init();
  hid_device *jc_r_handle = hid_open(0x57E, 0x2007, NULL);

  SwitchKit::SwitchController controller(jc_r_handle);
  controller.set_input_report_mode(SwitchKit::InputReportMode::MODE_STANDARD);
  controller.set_imu_enabled(true);
  controller.request_device_info();
  controller.request_stick_calibration();
  controller.request_imu_calibration();
  controller.request_color_data();

  controller.set_player_lights(
    SwitchKit::SwitchController::LIGHT_ON,
    SwitchKit::SwitchController::LIGHT_OFF,
    SwitchKit::SwitchController::LIGHT_FLASH,
    SwitchKit::SwitchController::LIGHT_ON
  );

  bool leave = false;
  while (!leave)
  {
    controller.poll();

    // Print the controller orientation.
    SwitchKit::Vector3 g = controller.get_gyro();
    printf("Gyro: %.2lf, %.2lf, %.2lf\n", g.x, g.y, g.z);

    // Close the program once the player presses the A button on the Joy-Con (R).
    if (controller.get_button_pressed_this_frame(SwitchKit::SwitchControllerReport::BTN_A)) {
      leave = true;
    }
  }

  hid_close(handle);
  res = hid_exit();
}
```

## Building

So far, SwitchKit has been solely developed and thus tested on macOS.
Please contribute usage instructions for other platforms if you're able!

### macOS

Run the command

```sh
make dynamic_macos
```

to create a dynamic library file `switchkit.dylib`.

Alternatively, run

```sh
make static_macos
```

to create an object file `switchkit.o` that you can statically link to other code.

If these build instructions seem awful, it's probably because I'm new to developing cross-platform C++ libraries like this.
So, file an issue or pull request or something with a better way.
I would appreciate learning better practices!

## Credits/acknowledgements

- [hidapi](https://github.com/libusb/hidapi) is used by this library for communicating with the controllers.
- [dekuNukem's Nintendo Switch Reverse Engineering repository](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering) was my main source of information when working on this project. Some snippets of code for decoding and encoding Joy-Con data were used.
- [Yamakaky's joy library (MIT)](https://github.com/Yamakaky/joy) was referenced as well for a developed implementation of code that interacted with Joy-Cons and the Ring-Con.
- [Suyu (GPL-3)](https://git.suyu.dev/suyu/suyu) helped with understanding how to interface with the Ring-Con.
- [jc_toolkit (MIT)](https://github.com/CTCaer/jc_toolkit) contained the CRC8 table and function necessary for some communications with the Joy-Cons.
- [My own JoyConUnityInputSystem package](https://github.com/Meorge/JoyConUnityInputSystem) helped me get a head start on developing this version. (It was built largely off of the findings from dekuNukem.)

## License

MIT
