/*
 * Anthony Wang
 * Drew Bancroft
 * November 6, 2026
 * EECE 2160 Lab 06
 * This program will read the switches and control the LEDs on the DE1-SoC board.
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

// Addresses for the LEDs, switches, and push buttons
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

    // Map physical memory
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

    // Shift and get the value of the selected switch
    return (Switches09 >> switchNum) & 0x1;
}

// Turn one LED on or off
void Write1Led(char *pBase, int ledNum, int state)
{
    int Leds09 = RegisterRead(pBase, LEDR_BASE);

    cout << "Current LED value: " << Leds09 << endl;

    if (state == 1)
    {
        // Turn on the selected LED
        Leds09 |= (1 << ledNum);
        cout << "USERSAID1 " << endl;
    }
    else
    {
        // Turn off the selected LED
        Leds09 &= ~(1 << ledNum);
        cout << "USERSAID0 " << endl;
    }

    cout << "Current LED value: " << Leds09 << endl;

    RegisterWrite(pBase, LEDR_BASE, Leds09);
}

// Read all switches
int ReadAllSwitches(char *pBase)
{
    // Only get the first 10 bits
    return RegisterRead(pBase, SW_BASE) & 0x3FF;
}

int main()
{
    // Initialize
    int fd;
    char *pBase = Initialize(&fd);

    // Test writing a value to the LEDs
    int value = 0;

    cout << "Enter an int value between 0 to 1023: " << endl;
    cin >> value;

    cout << "value to be written to LEDs = " << value << endl;

    WriteAllLeds(pBase, value);

    int readLEDs = RegisterRead(pBase, LEDR_BASE);
    cout << "value of LEDS read = " << readLEDs << endl;

    // Read one switch
    cout << "Enter a switch number (0-9): ";
    cin >> swNum;

    int swVal = Read1Switch(pBase, swNum);

    cout << "Switch " << swNum << " value = " << swVal << endl;

    // Control one LED
    cout << "Enter a led numbe 0-9" << endl;
    cin >> ldNum;

    cout << "Enter a state, 1 or 0" << endl;
    cin >> ldState;

    Write1Led(pBase, ldNum, ldState);

    // Wait for the switches to be changed
    cout << "Press Enter to continue program, flip some switchs first" << endl;

    cin.get();
    cin.get();

    cout << "Ok! moving on" << endl;

    // Read all switches and display on LEDs
    int allSw = ReadAllSwitches(pBase);

    cout << "Switches value int: " << allSw << endl;

    WriteAllLeds(pBase, allSw);

    cout << "Look at the leds" << endl;
    cout << "Press Enter to close program" << endl;

    cin.get();

    // Close the memory
    Finalize(pBase, fd);

    return 0;
}
