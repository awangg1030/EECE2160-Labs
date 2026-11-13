#include <iostream>
#include <unistd.h>
#include "MMap.h"
#include "SpiderLeg.h"

using namespace std;

// Wait until all three motors are ready
bool waitForLeg(SpiderLeg *leg)
{
    for (int i = 0; i < 500; i++)
    {
        if (leg->IsReady())
        {
            return true;
        }

        usleep(20000);
    }

    cout << "Motor is not ready!" << endl;
    return false;
}

int main()
{
    cout << "\nProgram Starting...!" << endl;

    MMap *m_map = new MMap();

    int legID;

    cout << "Enter leg number (0-5): ";
    cin >> legID;

    if (cin.fail() || legID < 0 || legID > 5)
    {
        cout << "Invalid leg number!" << endl;
        delete m_map;
        return 1;
    }

    // Get the motor IDs for the selected leg
    int hipID = legID * 3;
    int kneeID = hipID + 1;
    int ankleID = hipID + 2;

    SpiderLeg *leg = new SpiderLeg(m_map, hipID, kneeID, ankleID);

    // Set all motors to the slowest speed
    leg->SetSpeed(SpiderLeg::Hip, 0);
    leg->SetSpeed(SpiderLeg::Knee, 0);
    leg->SetSpeed(SpiderLeg::Ankle, 0);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    cout << "Starting leg movement..." << endl;

    // Lift the leg
    leg->MoveJoint(SpiderLeg::Knee, 15);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    leg->MoveJoint(SpiderLeg::Ankle, 15);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    // Move the leg forward
    leg->MoveJoint(SpiderLeg::Hip, 15);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    // Lower the leg
    leg->MoveJoint(SpiderLeg::Knee, 0);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    leg->MoveJoint(SpiderLeg::Ankle, 0);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    // Return the leg to the starting position
    leg->MoveJoint(SpiderLeg::Hip, 0);

    if (!waitForLeg(leg))
    {
        delete leg;
        delete m_map;
        return 1;
    }

    cout << "Movement completed!" << endl;

    // detele dynamic objects
    delete leg;
    delete m_map;

    cout << "Terminating..." << endl;

    return 0;
}
