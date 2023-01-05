#include "stdafx.h"
#include "Windows.h"
#include "ViGEmClient.h"
#include <iostream>
#include <string>
#include <csignal>
#include <tuple>
#include <unordered_map>
#include "Yaml.hpp"
#include <openvr.h>
#include "SteamIVRInput.h"
#include <chrono>
#include <map>
#include <conio.h>

#define u8 uint8_t
#define u16 uint16_t

const int XBOX_ANALOG_MIN = -32768;
const int XBOX_ANALOG_MAX = 32767;
const int XBOX_ANALOG_DIAG_MAX = round(XBOX_ANALOG_MAX * 0.5 * sqrt(2.0));
const int XBOX_ANALOG_DIAG_MIN = round(XBOX_ANALOG_MIN * 0.5 * sqrt(2.0));
u8 global_counter[2] = { 0,0 };

SteamIVRInput inputhandler;
PVIGEM_CLIENT client1 = vigem_alloc();
PVIGEM_CLIENT client2 = vigem_alloc();
PVIGEM_TARGET target1;
PVIGEM_TARGET target2;
XUSB_REPORT report1;
XUSB_REPORT report2;
int res;
HANDLE left_thread;
DWORD left_thread_id;
HANDLE right_thread;
DWORD right_thread_id;
HANDLE report_mutex;
bool kill_threads = false;
bool keep_rumbling = false;
UCHAR rumble_left_intensity1 = 0;
UCHAR rumble_right_intensity1 = 0;
UCHAR rumble_left_intensity2 = 0;
UCHAR rumble_right_intensity2 = 0;

bool double_controller = false;

//Controller (XBOX 360 For Windows)

enum ControllerButtons
{
  GAMEPAD_DPAD_UP = 0x0001,
  GAMEPAD_DPAD_DOWN = 0x0002,
  GAMEPAD_DPAD_LEFT = 0x0004,
  GAMEPAD_DPAD_RIGHT = 0x0008,
  GAMEPAD_START = 0x0010,
  GAMEPAD_BACK = 0x0020,
  GAMEPAD_LEFT_THUMB = 0x0040,
  GAMEPAD_RIGHT_THUMB = 0x0080,
  GAMEPAD_LEFT_SHOULDER = 0x0100,
  GAMEPAD_RIGHT_SHOULDER = 0x0200,
  GAMEPAD_GUIDE = 0x0400,
  GAMEPAD_A = 0x1000,
  GAMEPAD_B = 0x2000,
  GAMEPAD_X = 0x4000,
  GAMEPAD_Y = 0x8000

};

std::map<USHORT, USHORT> button_conversion_chart = {
    { 0, 0},
    { GAMEPAD_DPAD_UP, GAMEPAD_START },
    { GAMEPAD_DPAD_DOWN, GAMEPAD_BACK },
    { GAMEPAD_DPAD_LEFT, GAMEPAD_GUIDE },
    { GAMEPAD_DPAD_RIGHT, GAMEPAD_GUIDE },
    { GAMEPAD_START, GAMEPAD_START },
    { GAMEPAD_BACK, GAMEPAD_BACK },
    { GAMEPAD_LEFT_THUMB, GAMEPAD_LEFT_THUMB },
    { GAMEPAD_RIGHT_THUMB, GAMEPAD_LEFT_THUMB },
    { GAMEPAD_LEFT_SHOULDER, GAMEPAD_RIGHT_SHOULDER },
    { GAMEPAD_RIGHT_SHOULDER, GAMEPAD_RIGHT_SHOULDER },
    { GAMEPAD_GUIDE, GAMEPAD_GUIDE },
    { GAMEPAD_A, GAMEPAD_A },
    { GAMEPAD_B, GAMEPAD_B },
    { GAMEPAD_X, GAMEPAD_A },
    { GAMEPAD_Y, GAMEPAD_B }
};

USHORT convert_buttons(USHORT buttons) {
  USHORT flags = 0;
  USHORT i = 0x8000;
  while (i) {
    flags |= button_conversion_chart[buttons & i];
    i = i >> 1;
  }
  return flags;
}

USHORT left_button_selection = GAMEPAD_X |
  GAMEPAD_Y |
  GAMEPAD_LEFT_SHOULDER |
  GAMEPAD_LEFT_THUMB |
  GAMEPAD_DPAD_UP |
  GAMEPAD_DPAD_DOWN |
  GAMEPAD_DPAD_LEFT |
  GAMEPAD_DPAD_RIGHT;
USHORT right_button_selection = GAMEPAD_A |
  GAMEPAD_B |
  GAMEPAD_RIGHT_SHOULDER |
  GAMEPAD_RIGHT_THUMB |
  GAMEPAD_START |
  GAMEPAD_BACK |
  GAMEPAD_GUIDE ;

XUSB_BUTTON string_to_xbox_button(std::string input) {
  if(input == "XUSB_GAMEPAD_DPAD_UP") return XUSB_GAMEPAD_DPAD_UP;
  if(input == "XUSB_GAMEPAD_DPAD_DOWN") return XUSB_GAMEPAD_DPAD_DOWN;
  if(input == "XUSB_GAMEPAD_DPAD_LEFT") return XUSB_GAMEPAD_DPAD_LEFT;
  if(input == "XUSB_GAMEPAD_DPAD_RIGHT") return XUSB_GAMEPAD_DPAD_RIGHT;
  if(input == "XUSB_GAMEPAD_START") return XUSB_GAMEPAD_START;
  if(input == "XUSB_GAMEPAD_BACK") return XUSB_GAMEPAD_BACK;
  if(input == "XUSB_GAMEPAD_LEFT_THUMB") return XUSB_GAMEPAD_LEFT_THUMB;
  if(input == "XUSB_GAMEPAD_RIGHT_THUMB") return XUSB_GAMEPAD_RIGHT_THUMB;
  if(input == "XUSB_GAMEPAD_LEFT_SHOULDER") return XUSB_GAMEPAD_LEFT_SHOULDER;
  if(input == "XUSB_GAMEPAD_RIGHT_SHOULDER") return XUSB_GAMEPAD_RIGHT_SHOULDER;
  if(input == "XUSB_GAMEPAD_GUIDE") return XUSB_GAMEPAD_GUIDE;
  if(input == "XUSB_GAMEPAD_A") return XUSB_GAMEPAD_A;
  if(input == "XUSB_GAMEPAD_B") return XUSB_GAMEPAD_B;
  if(input == "XUSB_GAMEPAD_X") return XUSB_GAMEPAD_X;
  if(input == "XUSB_GAMEPAD_Y") return XUSB_GAMEPAD_Y;
  throw "invalid xbox button: " + input;
}

std::string vigem_error_to_string(VIGEM_ERROR err) {
  switch(err) {
    case VIGEM_ERROR_NONE: return "none";
    case VIGEM_ERROR_ALREADY_CONNECTED: return "already connected";
    case VIGEM_ERROR_BUS_ACCESS_FAILED: return "bus access failed";
    case VIGEM_ERROR_BUS_ALREADY_CONNECTED: return "bus already connected";
    case VIGEM_ERROR_BUS_NOT_FOUND: return "bus not found";
    case VIGEM_ERROR_BUS_VERSION_MISMATCH: return "bus version mismatch";
    case VIGEM_ERROR_CALLBACK_ALREADY_REGISTERED: return "callback already registered";
    case VIGEM_ERROR_CALLBACK_NOT_FOUND: return "callback not found";
    case VIGEM_ERROR_INVALID_TARGET: return "invalid target";
    case VIGEM_ERROR_NO_FREE_SLOT: return "no free slot";
    case VIGEM_ERROR_REMOVAL_FAILED: return "removal failed";
    case VIGEM_ERROR_TARGET_NOT_PLUGGED_IN: return "target not plugged in";
    case VIGEM_ERROR_TARGET_UNINITIALIZED: return "target uninitialized";
    default: return "none";
  }
}

void initialize_xbox(PVIGEM_CLIENT& client, PVIGEM_TARGET& target, XUSB_REPORT& report, USHORT PID) {
  std::cout << "initializing emulated Xbox 360 controller..." << std::endl;
  VIGEM_ERROR err = vigem_connect(client);
  if(err == VIGEM_ERROR_NONE) {
    std::cout << " => connected successfully" << std::endl;
  } else {
    std::cout << "connection error: " << vigem_error_to_string(err) << std::endl;
    vigem_free(client);
    std::cout << "press [Enter] to exit" << std::endl;
    getchar();
    exit(1);
  }

  target = vigem_target_x360_alloc(0x045E,PID);// 0x28DE, PID);
  vigem_target_add(client, target);
  XUSB_REPORT_INIT(&report);
  std::cout << " => added target Xbox 360 Controller" << std::endl;
  // TODO VIBRATION vigem_target_x360_register_notification()
  std::cout << std::endl;
}

void clear_vigem(PVIGEM_CLIENT& client, PVIGEM_TARGET& target) {
  vigem_target_remove(client, target);
  vigem_target_free(target);
  vigem_disconnect(client);
  vigem_free(client);
}


void disconnect_exit() {
  clear_vigem(client1, target1);
  if (double_controller) {
    clear_vigem(client2, target2);
  }
  exit(0);
}

void terminateapp() {
  kill_threads = true;
  inputhandler.Shutdown();
  Sleep(10);/*


  TerminateThread(left_thread, 0);
  TerminateThread(right_thread, 0);*/
  std::cout << "disconnecting and exiting..." << std::endl;
  disconnect_exit();
}

void exit_handler(int signum) {
	terminateapp();
  exit(signum);
}

void rumbleNotification1(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber)
{
	// std::cout << "Rumble received! " << int(LargeMotor) << " | " << int(SmallMotor) << std::endl;
	keep_rumbling = LargeMotor > 0 || SmallMotor > 0;
	rumble_left_intensity1 = LargeMotor;
	rumble_right_intensity1 = SmallMotor;
}

void rumbleNotification2(PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber)
{
  // std::cout << "Rumble received! " << int(LargeMotor) << " | " << int(SmallMotor) << std::endl;
  keep_rumbling = LargeMotor > 0 || SmallMotor > 0;
  rumble_left_intensity2 = LargeMotor;
  rumble_right_intensity2 = SmallMotor;
}

void handle_rumble(SteamIVRInput& inputhandler, float count) {
  if (double_controller) {
    if (rumble_left_intensity1 > 0)
    {
      inputhandler.rumbleController(1, count / 1000, 60, rumble_left_intensity1);
    }
    if (rumble_right_intensity1 > 0)
    {
      inputhandler.rumbleController(1, count / 1000, 60, rumble_right_intensity1);
    }
    if (rumble_left_intensity2 > 0)
    {
      inputhandler.rumbleController(0, count / 1000, 60, rumble_left_intensity2);
    }
    if (rumble_right_intensity2 > 0)
    {
      inputhandler.rumbleController(0, count / 1000, 60, rumble_right_intensity2);
    }
  }
  else
  {
    if (rumble_left_intensity1 > 0)
    {
      inputhandler.rumbleController(1, count / 1000, 60, rumble_left_intensity1);
    }
    if (rumble_right_intensity1 > 0)
    {
      inputhandler.rumbleController(0, count / 1000, 60, rumble_right_intensity1);
    }
  }

}

int main() {
  SetConsoleTitle(L"IndeX360 - Valve Index Xbox360 Emulator");

  signal(SIGINT, exit_handler);
  std::cout << "Based on XJoy v0.2.0" << std::endl << std::endl;

  initialize_xbox(client1, target1, report1, 0x2302);
  if (double_controller) {
    initialize_xbox(client2, target2, report2, 0x2303);
    std::cout << "__ Controller mode dual __" << std::endl;
    std::cout << std::endl;
  }
  else {
    std::cout << "__ Controller mode single __" << std::endl;
    std::cout << std::endl;
  }
  
  vigem_target_x360_register_notification(client1, target1, rumbleNotification1);
  if (double_controller) {
	  vigem_target_x360_register_notification(client2, target2, rumbleNotification2);
  }
  //hid_init();
  inputhandler.Init(true);
  std::cout << std::endl;
  std::cout << "initializing threads..." << std::endl;
  report_mutex = CreateMutex(NULL, FALSE, NULL);
  if(report_mutex == NULL) {
    printf("CreateMutex error: %d\n", GetLastError());
    return 1;
  }
  std::cout << " => created report mutex" << std::endl;
  std::cout << std::endl;

  std::cout << "press [1] or [2] to switch between single or dual controller mode." << std::endl;
  std::cout << "press [Escape] to exit." << std::endl;
  std::cout << std::endl;
  /*
  left_thread = CreateThread(0, 0, left_joycon_thread, 0, 0, &left_thread_id);
  right_thread = CreateThread(0, 0, right_joycon_thread, 0, 0, &right_thread_id);*/
  auto lastTick = std::chrono::steady_clock::now();
  int key_code;

  // seperate controllers
  do
  {
    key_code = 0;
    auto state = inputhandler.Loop();
    if (double_controller)
    {
      report1.bLeftTrigger = state.bLeftTrigger;
      report2.bLeftTrigger = state.bRightTrigger;
      report1.sThumbLX = state.sThumbLX;
      report1.sThumbLY = state.sThumbLY;
      report2.sThumbLX = state.sThumbRX;
      report2.sThumbLY = state.sThumbRY;
      report1.wButtons = convert_buttons(state.wButtons & left_button_selection);
      report2.wButtons = convert_buttons(state.wButtons & right_button_selection);
      vigem_target_x360_update(client2, target2, report2);
    }
    else {
      report1.bLeftTrigger = state.bLeftTrigger;
      report1.bRightTrigger = state.bRightTrigger;
      report1.sThumbLX = state.sThumbLX;
      report1.sThumbLY = state.sThumbLY;
      report1.sThumbRX = state.sThumbRX;
      report1.sThumbRY = state.sThumbRY;
      report1.wButtons = state.wButtons;
    }
    vigem_target_x360_update(client1, target1, report1);

    if (keep_rumbling)
    {
      std::chrono::duration<float, std::milli> delta = std::chrono::steady_clock::now() - lastTick;
      handle_rumble(inputhandler, delta.count());
    }

    if (_kbhit())
    {
      key_code = _getch();

      if (key_code == '2') {
        if (!double_controller) {
          client2 = vigem_alloc();
          initialize_xbox(client2, target2, report2, 0x2303);
          std::cout << "__ Controller mode dual __" << std::endl;
          std::cout << std::endl;
          vigem_target_x360_register_notification(client2, target2, rumbleNotification2);
        }
        double_controller = true;
      }
      if (key_code == '1') {
        if (double_controller) {
          std::cout << "Unload secondary controller..." << std::endl;
          clear_vigem(client2, target2);
          std::cout << " => memory unallocated" << std::endl;
          std::cout << std::endl;

          std::cout << "__ Controller mode single __" << std::endl;
          std::cout << std::endl;
        }
        double_controller = false;
      }
    }

	  lastTick = std::chrono::steady_clock::now();

	  Sleep(1);
  } while (key_code != 27);


  std::cout << std::endl;
  //getchar();
  terminateapp();
}
