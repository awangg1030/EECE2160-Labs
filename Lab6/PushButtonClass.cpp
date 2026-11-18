/*
 * Anthony Wang
 * Drew Bancroft
 * November 6, 2026
 * EECE 2160 Lab 06
 * This program will use the push buttons to control a counter
 * and display the counter value on the LEDs.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
using namespace std;

// FPGA base address
const unsigned int LW_BRIDGE_BASE = 0xFF200000;

// Memory address range
const unsigned int LW_BRIDGE_SPAN = 0x00005000;

// Addresses for LEDs, switches, and push buttons
const unsigned int LEDR_BASE = 0x00000000;
const unsigned int SW_BASE = 0x00000040;
const unsigned int KEY_BASE = 0x00000050;

class DE1SoCfpga {
    char *pBase;
    int fd;

public:

    DE1SoCfpga() {
        // Open and map the memory
        fd = open("/dev/mem", (O_RDWR | O_SYNC));

        if (fd == -1)
        {
            cout << "ERROR: could not open /dev/mem..." << endl;
            exit(1);
        }

        pBase = (char *)mmap(NULL, LW_BRIDGE_SPAN,
                        (PROT_READ | PROT_WRITE),
                        MAP_SHARED, fd, LW_BRIDGE_BASE);

        if (pBase == MAP_FAILED)
        {
            cout << "ERROR: mmap() failed..." << endl;
            close(fd);
            exit(1);
        }
    }

    ~DE1SoCfpga() {
        // Close the memory
        if (munmap(pBase, LW_BRIDGE_SPAN) != 0)
        {
            cout << "ERROR: munmap() failed..." << endl;
            exit(1);
        }
    close(fd);
    }

    // Write a value to the register
    void RegisterWrite(unsigned int reg_offset, int value)
    {
        *(volatile unsigned int *)(pBase + reg_offset) = value;
    }

    // Read a value from the register
    int RegisterRead(unsigned int reg_offset)
    {
        return *(volatile unsigned int *)(pBase + reg_offset);
    }

};

class LEDControl {

public:

    LEDControl() {
        cout << "LEDControl created." << endl;
    }

    ~LEDControl() {
        cout << "LEDControl destroyed." << endl;
    }

    // Write a value to all LEDs
    void WriteAllLeds(DE1SoCfpga &fpga, int value)
    {
        fpga.RegisterWrite(LEDR_BASE, value);
    }

    // Read one switch
    int Read1Switch(DE1SoCfpga &fpga, int switchNum)
    {
        int Switches09 = fpga.RegisterRead(SW_BASE);

        return (Switches09 >> switchNum) & 0x1;
    }

    // Turn one LED on or off
    void Write1Led(DE1SoCfpga &fpga, int ledNum, int state)
    {
        int Leds09 = fpga.RegisterRead(LEDR_BASE);

        cout << "Current LED value: " << Leds09 << endl;

        if (state == 1)
        {
            Leds09 |= (1 << ledNum);
            cout << "USERSAID1 " << endl;
        }
        else
        {
            Leds09 &= ~(1 << ledNum);
            cout << "USERSAID0 " << endl;
        }

        cout << "Current LED value: " << Leds09 << endl;

        fpga.RegisterWrite(LEDR_BASE, Leds09);
    }

    // Read all 10 switches
    int ReadAllSwitches(DE1SoCfpga &fpga)
    {
        return fpga.RegisterRead(SW_BASE) & 0x3FF;
    }

    // Read which push button is pressed
    int PushButtonGet(DE1SoCfpga &fpga)
    {
        int keys = fpga.RegisterRead(KEY_BASE) & 0xF;

        // Return -1 if no button is pressed
        if (keys == 0)
        {
            return -1;
        }

        int bitCount = 0;
        int ACTIVEPRESSED = -1;

        // Check each button
        for (int i = 0; i < 4; i++)
        {
            if (keys & (1 << i))
            {
                bitCount++;
                ACTIVEPRESSED = i;
            }
        }

        // Return 4 if multiple buttons are pressed
        if (bitCount > 1)
        {
            return 4;
        }
        else
        {
            return ACTIVEPRESSED;
        }
    }
};

int main()
{
    // Initialize
    DE1SoCfpga fpga;
    LEDControl leds;

    // Get starting counter value from switches
    int counter = leds.ReadAllSwitches(fpga);

    leds.WriteAllLeds(fpga, counter & 0x3FF);

    int nextButtonState = -1;

    // Continuously read the push buttons
    while (true)
    {
        int buttonState = leds.PushButtonGet(fpga);

        // Only update when a new button is pressed
        if (buttonState != -1 && nextButtonState != buttonState)
        {
            cout << "Button state: " << buttonState << endl;

            if (buttonState == 4)
            {
                // Reset counter when multiple buttons are pressed
                cout << "Multiple buttons pressed!" << endl;
                counter = leds.ReadAllSwitches(fpga);
            }
            else if (buttonState == 0)
            {
                // KEY0 increases counter by 1
                cout << "Button 0 pressed!" << endl;
                counter++;
            }
            else if (buttonState == 1)
            {
                // KEY1 decreases counter by 1
                cout << "Button 1 pressed!" << endl;
                counter--;
            }
            else if (buttonState == 2)
            {
                // KEY2 shifts counter right by 1
                cout << "Button 2 pressed!" << endl;
                counter = (counter >> 1) & 0x3FF;
            }
            else if (buttonState == 3)
            {
                // KEY3 shifts counter left by 1
                cout << "Button 3 pressed!" << endl;
                counter = (counter << 1) & 0x3FF;
            }

            // Keep counter between 0 and 1023
            counter &= 0x3FF;

            // Display counter value on LEDs
            leds.WriteAllLeds(fpga, counter);
        }

        // Save previous button state
        nextButtonState = buttonState;

        // Delay 50ms before checking again
        usleep(50000);
    }
}