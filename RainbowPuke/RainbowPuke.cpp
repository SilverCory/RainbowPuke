// RainbowPuke.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SDKDLL.h"
#include <signal.h>
#include <cstdint>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <WinUser.h>
#include <future>

using namespace std;

const uint8_t HSVlights[61] =
{ 0, 4, 8, 13, 17, 21, 25, 30, 34, 38, 42, 47, 51, 55, 59, 64, 68, 72, 76,
81, 85, 89, 93, 98, 102, 106, 110, 115, 119, 123, 127, 132, 136, 140, 144,
149, 153, 157, 161, 166, 170, 174, 178, 183, 187, 191, 195, 200, 204, 208,
212, 217, 221, 225, 229, 234, 238, 242, 246, 251, 255 };

COLOR_MATRIX matrix;

void createColourMatrix(int angle) {

	for (int column = MAX_LED_COLUMN; column >= 0; column--) {
		angle = angle + 15;
		if (angle > 360) {
			angle = angle - 360;
		}

		BYTE red, green, blue;
		if (angle < 60) { red = 255; green = HSVlights[angle]; blue = 0; }
		else
			if (angle < 120) { red = HSVlights[120 - angle]; green = 255; blue = 0; }
			else
				if (angle < 180) { red = 0, green = 255; blue = HSVlights[angle - 120]; }
				else
					if (angle < 240) { red = 0, green = HSVlights[240 - angle]; blue = 255; }
					else
						if (angle < 300) { red = HSVlights[angle - 240], green = 0; blue = 255; }
						else
						{
							red = 255, green = 0; blue = HSVlights[360 - angle];
						}

		for (int row = 0; row < MAX_LED_ROW; row++) {
			if ((row == 0 && column == 16 && (GetKeyState(VK_SCROLL) & 0x0001) == 0) || (row == 3 && column == 0 && (GetKeyState(VK_CAPITAL) & 0x0001) == 0)) {
				matrix.KeyColor[row][column] = KEY_COLOR(red / 2, green / 2, blue / 2);;
			}
			else {
				matrix.KeyColor[row][column] = KEY_COLOR(red, green, blue);
			}
		}
	}

}

void update(std::future<void> futureObj) {

	int angle = 0;
	SetControlDevice(DEV_MKeys_S);
	if (!EnableLedControl(true, DEV_MKeys_S)) {
		cout << "Unable to enable LED control..." << endl;
		exit(1);
	}

	while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {

		while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
			if (!IsDevicePlug(DEV_MKeys_S)) {
				cout << "No device detected" << endl;
			}
			else {
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}

		createColourMatrix(angle);
		if (!SetAllLedColor(matrix, DEV_MKeys_S)) {
			cout << "An error occurred duing sending the matrix.. Angle: " << angle << endl;
		}

		angle = angle + 4;
		if (angle > 360) {
			angle = angle - 360;
		}

	}
}

std::promise<void> exitSignal;
std::future<void> futureObj = exitSignal.get_future();
std::thread updateThread(&update, std::move(futureObj));

void deathHandler(int s) {
	exitSignal.set_value();
	updateThread.join();
	SetFullLedColor(0, 0, 0, DEV_MKeys_S);
	EnableLedControl(false, DEV_MKeys_S);
	RefreshLed(false, DEV_MKeys_S);
	SwitchLedEffect(EFF_FULL_ON, DEV_MKeys_S);
	exit(0);
}

BOOL WINAPI windowsDeath(DWORD CEvent)
{
	deathHandler(0);
}

int main() {
	signal(SIGABRT, &deathHandler);
	signal(SIGTERM, &deathHandler);
	signal(SIGINT, &deathHandler);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)windowsDeath, TRUE);

	while (1);
	return 0;
}

