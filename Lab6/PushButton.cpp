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

int swNum;
int ldNum;
int ldState;
int allSw;

// Open and map the memory
char *Initialize(int *fd)
{
    *fd = open("/dev/mem", (O_RDWR | O_SYNC));

    if (*fd == -1)
    {
        cout << "ERROR: could not open /dev/mem..." << endl;
        exit(1);
    }

    char *virtual_base = (char *)mmap(NULL, LW_BRIDGE_SPAN,
                         (PROT_READ | PROT_WRITE),
                         MAP_SHARED, *fd, LW_BRIDGE_BASE);

    if (virtual_base == MAP_FAILED)
    {
        cout << "ERROR: mmap() failed..." << endl;
        close(*fd);
        exit(1);
    }

    return virtual_base;
}

// Close the memory
void Finalize(char *pBase, int fd)
{
    if (munmap(pBase, LW_BRIDGE_SPAN) != 0)
    {
        cout << "ERROR: munmap() failed..." << endl;
        exit(1);
    }

    close(fd);
}

// Write a value to the register
void RegisterWrite(char *pBase, unsigned int reg_offset, int value)
{
    *(volatile unsigned int *)(pBase + reg_offset) = value;
}

// Read a value from the register
int RegisterRead(char *pBase, unsigned int reg_offset)
{
    return *(volatile unsigned int *)(pBase + reg_offset);
}

// Write a value to all LEDs
void WriteAllLeds(char *pBase, int value)
{
    RegisterWrite(pBase, LEDR_BASE, value);
}

// Read one switch
int Read1Switch(char *pBase, int switchNum)
{
    int Switches09 = RegisterRead(pBase, SW_BASE);

    return (Switches09 >> switchNum) & 0x1;
}

// Turn one LED on or off
void Write1Led(char *pBase, int ledNum, int state)
{
    int Leds09 = RegisterRead(pBase, LEDR_BASE);

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

    RegisterWrite(pBase, LEDR_BASE, Leds09);
}

// Read all 10 switches
int ReadAllSwitches(char *pBase)
{
    return RegisterRead(pBase, SW_BASE) & 0x3FF;
}

// Read which push button is pressed
int PushButtonGet(char *pBase)
{
    int keys = RegisterRead(pBase, KEY_BASE) & 0xF;

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

int main()
{
    // Initialize
    int fd;
    char *pBase = Initialize(&fd);

    // Get starting counter value from switches
    int counter = ReadAllSwitches(pBase);

    WriteAllLeds(pBase, counter & 0x3FF);

    int nextButtonState = -1;

    // Continuously read the push buttons
    while (true)
    {
        int buttonState = PushButtonGet(pBase);

        // Only update when a new button is pressed
        if (buttonState != -1 && nextButtonState != buttonState)
        {
            cout << "Button state: " << buttonState << endl;

            if (buttonState == 4)
            {
                // Reset counter when multiple buttons are pressed
                cout << "Multiple buttons pressed!" << endl;
                counter = ReadAllSwitches(pBase);
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
            WriteAllLeds(pBase, counter);
        }

        // Save previous button state
        nextButtonState = buttonState;

        // Delay 50ms before checking again
        usleep(50000);
    }
}
