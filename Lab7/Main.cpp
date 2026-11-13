/**
 * @file   Main.cpp
 * @brief  Tests the Spider robot rc servo movements
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include "MMap.h"
#include "Motor.h"
#include "LEDControl.h"

using namespace std;

int main(int argc, char *argv[]){

	cout << "\nProgram Starting...!" << endl;
	// Create necessary class objects
	MMap *m_map = new MMap();
	LEDControl *pio = new LEDControl(m_map);

    int motorID;

    cout << "Enter motor ID (0-17): ";
    cin >> motorID;

    if (cin.fail() || motorID < 0 || motorID > 17)
    {
        cout << "Invalid motor ID!" << endl;

        delete pio;
        delete m_map;
        return 1;
    }

    Motor *motor = new Motor(m_map, motorID);

	// TO DO: Write code to test your program below....

    int angle = 0;
    int oldKey0 = pio->ReadButton(0);
    int oldKey1 = pio->ReadButton(1);

    cout << "KEY0: Decrease angle by 15 degrees" << endl;
    cout << "KEY1: Increase angle by 15 degrees" << endl;
    cout << "SW0: Exit program" << endl;

    while (pio->Read1Switch(0) == 0)
    {
        int key0 = pio->ReadButton(0);
        int key1 = pio->ReadButton(1);

        // Check if KEY0 is pressed
        if (key0 == 1 && oldKey0 == 0)
        {
            if (angle > -90 && motor->IsReady())
            {
                angle = angle - 15;
                motor->Move(angle);

                cout << "Current angle: " << angle << endl;
            }
        }

        // Check if KEY1 is pressed
        if (key1 == 1 && oldKey1 == 0)
        {
            if (angle < 90 && motor->IsReady())
            {
                angle = angle + 15;
                motor->Move(angle);

                cout << "Current angle: " << angle << endl;
            }
        }

        oldKey0 = key0;
        oldKey1 = key1;

        usleep(20000);
    }

	// detele dynamic objects
	delete motor;
	delete pio;
	delete m_map;
	cout << "Terminating..." << endl;
	return 0;
}
