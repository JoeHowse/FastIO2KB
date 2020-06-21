// fast2kb.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#ifdef KEYMAP_PPSSPP
#include "keymap_ppsspp.h"
#else
#include "keymap_mame.h"
#endif



#define POLLING_INTERVAL 1ul // milliseconds


//#define VERBOSE

#ifdef VERBOSE
#include "keymap_verbose.h"
#define LOG_VERBOSE(...) (fprintf_s(stdout, __VA_ARGS__))
#else
#define LOG_VERBOSE(...)
#endif

#define LOG_ERROR(...) (fprintf_s(stderr, __VA_ARGS__))


#if defined(P3_START)    ||\
    defined(P3_UP)       ||\
    defined(P3_DOWN)     ||\
    defined(P3_LEFT)     ||\
    defined(P3_RIGHT)    ||\
    defined(P3_BTN1)     ||\
    defined(P3_BTN2)     ||\
    defined(P3_BTN3)     ||\
    defined(P3_BTN4)     ||\
    defined(P3_BTN5)     ||\
    defined(P3_BTN6)     ||\
    defined(P3_BTN7)     ||\
    defined(P3_BTN8)     ||\
    defined(P4_START)    ||\
    defined(P4_UP)       ||\
    defined(P4_DOWN)     ||\
    defined(P4_LEFT)     ||\
    defined(P4_RIGHT)    ||\
    defined(P4_BTN1)     ||\
    defined(P4_BTN2)     ||\
    defined(P4_BTN3)     ||\
    defined(P4_BTN4)     ||\
    defined(P4_BTN5)     ||\
    defined(P4_BTN6)     ||\
    defined(P4_BTN7)     ||\
    defined(P4_BTN8)     ||\
    defined(S_COIN3)     ||\
    defined(S_COIN4)     ||\
    defined(S_TEST2)     ||\
    defined(S_TILT2)     ||\
    defined(S_SERVICE3)  ||\
    defined(S_SERVICE4)  ||\
    defined(S_ESC3)      ||\
    defined(S_ESC4)
#define P3P4_ENABLED
#endif



struct FIOKEYS
{
    int k1start_flag;
    int k2start_flag;
    int k3start_flag;
    int k4start_flag;
    int k1up_flag;
    int k2up_flag;
    int k3up_flag;
    int k4up_flag;
    int k1down_flag;
    int k2down_flag;
    int k3down_flag;
    int k4down_flag;
    int k1left_flag;
    int k2left_flag;
    int k3left_flag;
    int k4left_flag;
    int k1right_flag;
    int k2right_flag;
    int k3right_flag;
    int k4right_flag;
    int k1b1_flag;
    int k2b1_flag;
    int k3b1_flag;
    int k4b1_flag;
    int k1b2_flag;
    int k2b2_flag;
    int k3b2_flag;
    int k4b2_flag;
    int k1b3_flag;
    int k2b3_flag;
    int k3b3_flag;
    int k4b3_flag;
    int k1b4_flag;
    int k2b4_flag;
    int k3b4_flag;
    int k4b4_flag;
    int k1b5_flag;
    int k2b5_flag;
    int k3b5_flag;
    int k4b5_flag;
    int k1b6_flag;
    int k2b6_flag;
    int k3b6_flag;
    int k4b6_flag;
    int k1b7_flag;
    int k2b7_flag;
    int k3b7_flag;
    int k4b7_flag;
    int k1b8_flag;
    int k2b8_flag;
    int k3b8_flag;
    int k4b8_flag;
    int k1coin_flag;
    int k2coin_flag;
    int k3coin_flag;
    int k4coin_flag;
    int k1test_flag;
    int k2test_flag;
    int k1tilt_flag;
    int k2tilt_flag;
    int k1service_flag;
    int k2service_flag;
    int k3service_flag;
    int k4service_flag;
    int k1esc_flag;
    int k2esc_flag;
    int k3esc_flag;
    int k4esc_flag;
};



typedef int(*dmacOpen)(int, LPVOID, LPVOID);
typedef int(*dmacRead)(int, DWORD, LPVOID, LPVOID);
typedef int(*dmacWrite)(int, DWORD, int, LPVOID);
typedef int(*dmacClose)(int, LPVOID);

HMODULE dmacdll = LoadLibrary(TEXT("iDmacDrv32.dll"));

dmacOpen iDmacDrvOpen = (dmacOpen)GetProcAddress(dmacdll, "iDmacDrvOpen");
dmacRead iDmacDrvRegisterRead = (dmacRead)GetProcAddress(dmacdll, "iDmacDrvRegisterRead");
dmacWrite iDmacDrvRegisterWrite = (dmacWrite)GetProcAddress(dmacdll, "iDmacDrvRegisterWrite");
dmacClose iDmacDrvClose = (dmacClose)GetProcAddress(dmacdll, "iDmacDrvClose");


int deviceIndex = 1;
int deviceId;
int buttonsAddressP1P2;
int buttonsAddressP3P4;

HANDLE hMainThread;
BOOL keepPolling = TRUE;



BOOL WINAPI ctrlHandler(DWORD signal)
{
    UNREFERENCED_PARAMETER(signal);

    keepPolling = FALSE;
    WaitForSingleObject(hMainThread, INFINITE);
    CloseHandle(hMainThread);

    return TRUE;
}


BOOL __cdecl FIO_Open()
{
    int flags = 0x0;
    return iDmacDrvOpen(deviceIndex, &deviceId, &flags) == 0;
}


BOOL __cdecl FIO_RegRead(DWORD address, int &data_out)
{
    int flags = 0x0;
    return iDmacDrvRegisterRead(deviceId, address, &data_out, &flags) == 0;
}


BOOL assignPlayers()
{
    int data;
    BOOL port1HasConnection = FIO_RegRead(0x4000, data) && data & 0xff;
    BOOL port2HasConnection = FIO_RegRead(0x4004, data) && data & 0xff;

    if (port1HasConnection)
    {
        buttonsAddressP1P2 = 0x4120;
        LOG_VERBOSE("Port 1: Player 1 + Player 2\n");
        if (port2HasConnection)
        {
            buttonsAddressP3P4 = 0x41a0;
            LOG_VERBOSE("Port 2: Player 3 + Player 4\n");
        }
        else
        {
            buttonsAddressP3P4 = NULL;
            LOG_VERBOSE("Port 2: Empty\n");
        }
    }
    else
    {
        buttonsAddressP3P4 = NULL;
        LOG_VERBOSE("Port 1: Empty\n");
        if (port2HasConnection)
        {
            buttonsAddressP1P2 = 0x41a0;
            LOG_VERBOSE("Port 2: Player 1 + Player 2\n");
        }
        else
        {
            buttonsAddressP1P2 = NULL;
            LOG_VERBOSE("Port 2: Empty\n");
            return FALSE;
        }
    }

    return TRUE;
}


void pollP1P2(FIOKEYS &fioKeys, INPUT &diEvent)
{
    int buttonsData;
    if (!FIO_RegRead(buttonsAddressP1P2, buttonsData))
    {
        return;
    }
    if (buttonsData != 0x0)
    {
        LOG_VERBOSE("pollP1P2: buttonsData=0x%x\n", buttonsData);
    }

#ifdef S_ESC1
    // Player 1 Start + Player 1 Button 1 + Player 1 Button 3
    if (buttonsData & 0x10 && buttonsData & 0x10000 && buttonsData & 0x100000)
    {
        if (fioKeys.k1esc_flag == 0)
        {
            diEvent.ki.dwFlags = S_ESC1_FLAGS;
            diEvent.ki.wScan = S_ESC1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1esc_flag = 1;
            LOG_VERBOSE("Pressed: Escape 1 (Player 1 Start + Button 1 + Button 3)\n");
        }
    }
    else if (fioKeys.k1esc_flag == 1)
    {
        diEvent.ki.dwFlags = S_ESC1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_ESC1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1esc_flag = 0;
        LOG_VERBOSE("Released: Escape 1 (Player 1 Start + Button 1 + Button 3)\n");
    }
#endif // S_ESC1

#ifdef S_ESC2
    // Player 2 Start + Player 2 Button 1 + Player 2 Button 3
    if (buttonsData & 0x20 && buttonsData & 0x20000 && buttonsData & 0x200000)
    {
        if (fioKeys.k2esc_flag == 0)
        {
            diEvent.ki.dwFlags = S_ESC2_FLAGS;
            diEvent.ki.wScan = S_ESC2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2esc_flag = 1;
            LOG_VERBOSE("Pressed: Escape 2 (Player 2 Start + Button 1 + Button 3)\n");
        }
    }
    else if (fioKeys.k2esc_flag == 1)
    {
        diEvent.ki.dwFlags = S_ESC2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_ESC2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2esc_flag = 0;
        LOG_VERBOSE("Released: Escape 2 (Player 2 Start + Button 1 + Button 3)\n");
    }
#endif // S_ESC2

#ifdef S_COIN1
    // Coin 1
    if (buttonsData & 0x1)
    {
        if (fioKeys.k1coin_flag == 0)
        {
            diEvent.ki.dwFlags = S_COIN1_FLAGS;
            diEvent.ki.wScan = S_COIN1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1coin_flag = 1;
            LOG_VERBOSE("Pressed: Coin 1\n");
        }
    }
    else if (fioKeys.k1coin_flag == 1)
    {
        diEvent.ki.dwFlags = S_COIN1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_COIN1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1coin_flag = 0;
        LOG_VERBOSE("Released: Coin 1\n");
    }
#endif // S_COIN1

#ifdef S_COIN2
    // Coin 2
    if (buttonsData & 0x2)
    {
        if (fioKeys.k2coin_flag == 0)
        {
            diEvent.ki.dwFlags = S_COIN2_FLAGS;
            diEvent.ki.wScan = S_COIN2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2coin_flag = 1;
            LOG_VERBOSE("Pressed: Coin 2\n");
        }
    }
    else if (fioKeys.k2coin_flag == 1)
    {
        diEvent.ki.dwFlags = S_COIN2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_COIN2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2coin_flag = 0;
        LOG_VERBOSE("Released: Coin 2\n");
    }
#endif // S_COIN2

#ifdef S_SERVICE1
    // Service 1
    if (buttonsData & 0x4)
    {
        if (fioKeys.k1service_flag == 0)
        {
            diEvent.ki.dwFlags = S_SERVICE1_FLAGS;
            diEvent.ki.wScan = S_SERVICE1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1service_flag = 1;
            LOG_VERBOSE("Pressed: Service 1\n");
        }
    }
    else if (fioKeys.k1service_flag == 1)
    {
        diEvent.ki.dwFlags = S_SERVICE1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_SERVICE1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1service_flag = 0;
        LOG_VERBOSE("Released: Service 1\n");
    }
#endif // S_SERVICE1

#ifdef S_SERVICE2
    // Service 2
    if (buttonsData & 0x8)
    {
        if (fioKeys.k2service_flag == 0)
        {
            diEvent.ki.dwFlags = S_SERVICE2_FLAGS;
            diEvent.ki.wScan = S_SERVICE2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2service_flag = 1;
            LOG_VERBOSE("Pressed: Service 2\n");
        }
    }
    else if (fioKeys.k2service_flag == 1)
    {
        diEvent.ki.dwFlags = S_SERVICE2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_SERVICE2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2service_flag = 0;
        LOG_VERBOSE("Released: Service 2\n");
    }
#endif // S_SERVICE2

#ifdef P1_START
    // Player 1 Start
    if (buttonsData & 0x10)
    {
        if (fioKeys.k1start_flag == 0)
        {
            diEvent.ki.dwFlags = P1_START_FLAGS;
            diEvent.ki.wScan = P1_START;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1start_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Start\n");
        }
    }
    else if (fioKeys.k1start_flag == 1)
    {
        diEvent.ki.dwFlags = P1_START_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_START;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1start_flag = 0;
        LOG_VERBOSE("Released: Player 1 Start\n");
    }
#endif // P1_START

#ifdef P2_START
    // Player 2 Start
    if (buttonsData & 0x20)
    {
        if (fioKeys.k2start_flag == 0)
        {
            diEvent.ki.dwFlags = P2_START_FLAGS;
            diEvent.ki.wScan = P2_START;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2start_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Start\n");
        }
    }
    else if (fioKeys.k2start_flag == 1)
    {
        diEvent.ki.dwFlags = P2_START_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_START;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2start_flag = 0;
        LOG_VERBOSE("Released: Player 2 Start\n");
    }
#endif // P2_START

#ifdef S_TEST1
    // Test 1
    if (buttonsData & 0x40)
    {
        if (fioKeys.k1test_flag == 0)
        {
            diEvent.ki.dwFlags = S_TEST1_FLAGS;
            diEvent.ki.wScan = S_TEST1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1test_flag = 1;
            LOG_VERBOSE("Pressed: Test 1\n");
        }
    }
    else if (fioKeys.k1test_flag == 1)
    {
        diEvent.ki.dwFlags = S_TEST1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_TEST1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1test_flag = 0;
        LOG_VERBOSE("Released: Test 1\n");
    }
#endif // S_TEST1

#ifdef S_TILT1
    // Tilt 1
    if (buttonsData & 0x80)
    {
        if (fioKeys.k1tilt_flag == 0)
        {
            diEvent.ki.dwFlags = S_TILT1_FLAGS;
            diEvent.ki.wScan = S_TILT1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1tilt_flag = 1;
            LOG_VERBOSE("Pressed: Tilt 1\n");
        }
    }
    else if (fioKeys.k1tilt_flag == 1)
    {
        diEvent.ki.dwFlags = S_TILT1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_TILT1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1tilt_flag = 0;
        LOG_VERBOSE("Released: Tilt 1\n");
    }
#endif // S_TILT1

#ifdef P1_UP
    // Player 1 Up
    if (buttonsData & 0x100)
    {
        if (fioKeys.k1up_flag == 0)
        {
            diEvent.ki.dwFlags = P1_UP_FLAGS;
            diEvent.ki.wScan = P1_UP;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1up_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Up\n");
        }
    }
    else if (fioKeys.k1up_flag == 1)
    {
        diEvent.ki.dwFlags = P1_UP_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_UP;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1up_flag = 0;
        LOG_VERBOSE("Released: Player 1 Up\n");
    }
#endif // P1_UP

#ifdef P2_UP
    // Player 2 Up
    if (buttonsData & 0x200)
    {
        if (fioKeys.k2up_flag == 0)
        {
            diEvent.ki.dwFlags = P2_UP_FLAGS;
            diEvent.ki.wScan = P2_UP;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2up_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Up\n");
        }
    }
    else if (fioKeys.k2up_flag == 1)
    {
        diEvent.ki.dwFlags = P2_UP_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_UP;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2up_flag = 0;
        LOG_VERBOSE("Released: Player 2 Up\n");
    }
#endif // P2_UP

#ifdef P1_DOWN
    // Player 1 Down
    if (buttonsData & 0x400)
    {
        if (fioKeys.k1down_flag == 0)
        {
            diEvent.ki.dwFlags = P1_DOWN_FLAGS;
            diEvent.ki.wScan = P1_DOWN;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1down_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Down\n");
        }
    }
    else if (fioKeys.k1down_flag == 1)
    {
        diEvent.ki.dwFlags = P1_DOWN_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_DOWN;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1down_flag = 0;
        LOG_VERBOSE("Released: Player 1 Down\n");
    }
#endif // P1_DOWN

#ifdef P2_DOWN
    // Player 2 Down
    if (buttonsData & 0x800)
    {
        if (fioKeys.k2down_flag == 0)
        {
            diEvent.ki.dwFlags = P2_DOWN_FLAGS;
            diEvent.ki.wScan = P2_DOWN;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2down_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Down\n");
        }
    }
    else if (fioKeys.k2down_flag == 1)
    {
        diEvent.ki.dwFlags = P2_DOWN_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_DOWN;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2down_flag = 0;
        LOG_VERBOSE("Released: Player 2 Down\n");
    }
#endif // P2_DOWN

#ifdef P1_LEFT
    // Player 1 Left
    if (buttonsData & 0x1000)
    {
        if (fioKeys.k1left_flag == 0)
        {
            diEvent.ki.dwFlags = P1_LEFT_FLAGS;
            diEvent.ki.wScan = P1_LEFT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1left_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Left\n");
        }
    }
    else if (fioKeys.k1left_flag == 1)
    {
        diEvent.ki.dwFlags = P1_LEFT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_LEFT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1left_flag = 0;
        LOG_VERBOSE("Released: Player 1 Left\n");
    }
#endif // P1_LEFT

#ifdef P2_LEFT
    // Player 2 Left
    if (buttonsData & 0x2000)
    {
        if (fioKeys.k2left_flag == 0)
        {
            diEvent.ki.dwFlags = P2_LEFT_FLAGS;
            diEvent.ki.wScan = P2_LEFT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2left_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Left\n");
        }
    }
    else if (fioKeys.k2left_flag == 1)
    {
        diEvent.ki.dwFlags = P2_LEFT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_LEFT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2left_flag = 0;
        LOG_VERBOSE("Released: Player 2 Left\n");
    }
#endif // P2_LEFT

#ifdef P1_RIGHT
    // Player 1 Right
    if (buttonsData & 0x4000)
    {
        if (fioKeys.k1right_flag == 0)
        {
            diEvent.ki.dwFlags = P1_RIGHT_FLAGS;
            diEvent.ki.wScan = P1_RIGHT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1right_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Right\n");
        }
    }
    else if (fioKeys.k1right_flag == 1)
    {
        diEvent.ki.dwFlags = P1_RIGHT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_RIGHT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1right_flag = 0;
        LOG_VERBOSE("Released: Player 1 Right\n");
    }
#endif // P1_RIGHT

#ifdef P2_RIGHT
    // Player 2 Right
    if (buttonsData & 0x8000)
    {
        if (fioKeys.k2right_flag == 0)
        {
            diEvent.ki.dwFlags = P2_RIGHT_FLAGS;
            diEvent.ki.wScan = P2_RIGHT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2right_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Right\n");
        }
    }
    else if (fioKeys.k2right_flag == 1)
    {
        diEvent.ki.dwFlags = P2_RIGHT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_RIGHT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2right_flag = 0;
        LOG_VERBOSE("Released: Player 2 Right\n");
    }
#endif // P2_RIGHT

#ifdef P1_BTN1
    // Player 1 Button 1
    if (buttonsData & 0x10000)
    {
        if (fioKeys.k1b1_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN1_FLAGS;
            diEvent.ki.wScan = P1_BTN1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b1_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 1\n");
        }
    }
    else if (fioKeys.k1b1_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b1_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 1\n");
    }
#endif // P1_BTN1

#ifdef P2_BTN1
    // Player 2 Button 1
    if (buttonsData & 0x20000)
    {
        if (fioKeys.k2b1_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN1_FLAGS;
            diEvent.ki.wScan = P2_BTN1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b1_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 1\n");
        }
    }
    else if (fioKeys.k2b1_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b1_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 1\n");
    }
#endif // P2_BTN1

#ifdef P1_BTN2
    // Player 1 Button 2
    if (buttonsData & 0x40000)
    {
        if (fioKeys.k1b2_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN2_FLAGS;
            diEvent.ki.wScan = P1_BTN2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b2_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 2\n");
        }
    }
    else if (fioKeys.k1b2_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b2_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 2\n");
    }
#endif // P1_BTN2

#ifdef P2_BTN2
    // Player 2 Button 2
    if (buttonsData & 0x80000)
    {
        if (fioKeys.k2b2_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN2_FLAGS;
            diEvent.ki.wScan = P2_BTN2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b2_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 2\n");
        }
    }
    else if (fioKeys.k2b2_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b2_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 2\n");
    }
#endif // P2_BTN2

#ifdef P1_BTN3
    // Player 1 Button 3
    if (buttonsData & 0x100000)
    {
        if (fioKeys.k1b3_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN3_FLAGS;
            diEvent.ki.wScan = P1_BTN3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b3_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 3\n");
        }
    }
    else if (fioKeys.k1b3_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b3_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 3\n");
    }
#endif // P1_BTN3

#ifdef P2_BTN3
    // Player 2 Button 3
    if (buttonsData & 0x200000)
    {
        if (fioKeys.k2b3_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN3_FLAGS;
            diEvent.ki.wScan = P2_BTN3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b3_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 3\n");
        }
    }
    else if (fioKeys.k2b3_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b3_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 3\n");
    }
#endif // P2_BTN3

#ifdef P1_BTN4
    // Player 1 Button 4
    if (buttonsData & 0x400000)
    {
        if (fioKeys.k1b4_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN4_FLAGS;
            diEvent.ki.wScan = P1_BTN4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b4_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 4\n");
        }
    }
    else if (fioKeys.k1b4_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b4_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 4\n");
    }
#endif // P1_BTN4

#ifdef P2_BTN4
    // Player 2 Button 4
    if (buttonsData & 0x800000)
    {
        if (fioKeys.k2b4_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN4_FLAGS;
            diEvent.ki.wScan = P2_BTN4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b4_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 4\n");
        }
    }
    else if (fioKeys.k2b4_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b4_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 4\n");
    }
#endif // P2_BTN4

#ifdef P1_BTN5
    // Player 1 Button 5
    if (buttonsData & 0x1000000)
    {
        if (fioKeys.k1b5_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN5_FLAGS;
            diEvent.ki.wScan = P1_BTN5;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b5_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 5\n");
        }
    }
    else if (fioKeys.k1b5_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN5_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN5;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b5_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 5\n");
    }
#endif // P1_BTN5

#ifdef P2_BTN5
    // Player 2 Button 5
    if (buttonsData & 0x2000000)
    {
        if (fioKeys.k2b5_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN5_FLAGS;
            diEvent.ki.wScan = P2_BTN5;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b5_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 5\n");
        }
    }
    else if (fioKeys.k2b5_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN5_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN5;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b5_flag = 0;
        LOG_VERBOSE("Pressed: Player 2 Button 5\n");
    }
#endif // P2_BTN5

#ifdef P1_BTN6
    // Player 1 Button 6
    if (buttonsData & 0x4000000)
    {
        if (fioKeys.k1b6_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN6_FLAGS;
            diEvent.ki.wScan = P1_BTN6;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b6_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 6\n");
        }
    }
    else if (fioKeys.k1b6_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN6_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN6;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b6_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 6\n");
    }
#endif // P1_BTN6

#ifdef P2_BTN6
    // Player 2 Button 6
    if (buttonsData & 0x8000000)
    {
        if (fioKeys.k2b6_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN6_FLAGS;
            diEvent.ki.wScan = P2_BTN6;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b6_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 6\n");
        }
    }
    else if (fioKeys.k2b6_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN6_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN6;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b6_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 6\n");
    }
#endif // P2_BTN6

#ifdef P1_BTN7
    // Player 1 Button 7
    if (buttonsData & 0x10000000)
    {
        if (fioKeys.k1b7_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN7_FLAGS;
            diEvent.ki.wScan = P1_BTN7;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b7_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 7\n");
        }
    }
    else if (fioKeys.k1b7_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN7_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN7;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b7_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 7\n");
    }
#endif // P1_BTN7

#ifdef P2_BTN7
    // Player 2 Button 7
    if (buttonsData & 0x20000000)
    {
        if (fioKeys.k2b7_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN7_FLAGS;
            diEvent.ki.wScan = P2_BTN7;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b7_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 7\n");
        }
    }
    else if (fioKeys.k2b7_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN7_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN7;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b7_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 7\n");
    }
#endif // P2_BTN7

#ifdef P1_BTN8
    // Player 1 Button 8
    if (buttonsData & 0x40000000)
    {
        if (fioKeys.k1b8_flag == 0)
        {
            diEvent.ki.dwFlags = P1_BTN8_FLAGS;
            diEvent.ki.wScan = P1_BTN8;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k1b8_flag = 1;
            LOG_VERBOSE("Pressed: Player 1 Button 8\n");
        }
    }
    else if (fioKeys.k1b8_flag == 1)
    {
        diEvent.ki.dwFlags = P1_BTN8_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P1_BTN8;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k1b8_flag = 0;
        LOG_VERBOSE("Released: Player 1 Button 8\n");
    }
#endif // P1_BTN8

#ifdef P2_BTN8
    // Player 2 Button 8
    if (buttonsData & 0x80000000)
    {
        if (fioKeys.k2b8_flag == 0)
        {
            diEvent.ki.dwFlags = P2_BTN8_FLAGS;
            diEvent.ki.wScan = P2_BTN8;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2b8_flag = 1;
            LOG_VERBOSE("Pressed: Player 2 Button 8\n");
        }
    }
    else if (fioKeys.k2b8_flag == 1)
    {
        diEvent.ki.dwFlags = P2_BTN8_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P2_BTN8;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2b8_flag = 0;
        LOG_VERBOSE("Released: Player 2 Button 8\n");
    }
#endif // P2_BTN8
}


#ifdef P3P4_ENABLED
void pollP3P4(FIOKEYS &fioKeys, INPUT &diEvent)
{
    int buttonsData;
    if (!FIO_RegRead(buttonsAddressP3P4, buttonsData))
    {
        return;
    }
    if (buttonsData != 0x0)
    {
        LOG_VERBOSE("pollP3P4: buttonsData=0x%x\n", buttonsData);
    }

#ifdef S_ESC3
    // Player 3 Start + Player 3 Button 1 + Player 3 Button 3
    if (buttonsData & 0x10 && buttonsData & 0x10000 && buttonsData & 0x100000)
    {
        if (fioKeys.k3esc_flag == 0)
        {
            diEvent.ki.dwFlags = S_ESC3_FLAGS;
            diEvent.ki.wScan = S_ESC3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3esc_flag = 1;
            LOG_VERBOSE("Pressed: Escape 3 (Player 3 Start + Button 1 + Button 3)\n");
        }
    }
    else if (fioKeys.k3esc_flag == 1)
    {
        diEvent.ki.dwFlags = S_ESC3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_ESC3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3esc_flag = 0;
        LOG_VERBOSE("Released: Escape 3 (Player 3 Start + Button 1 + Button 3)\n");
    }
#endif // S_ESC3

#ifdef S_ESC4
    // Player 4 Start + Player 4 Button 1 + Player 4 Button 3
    if (buttonsData & 0x20 && buttonsData & 0x20000 && buttonsData & 0x200000)
    {
        if (fioKeys.k4esc_flag == 0)
        {
            diEvent.ki.dwFlags = S_ESC4_FLAGS;
            diEvent.ki.wScan = S_ESC4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4esc_flag = 1;
            LOG_VERBOSE("Pressed: Escape 4 (Player 4 Start + Button 1 + Button 3)\n");
        }
    }
    else if (fioKeys.k4esc_flag == 1)
    {
        diEvent.ki.dwFlags = S_ESC4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_ESC4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4esc_flag = 0;
        LOG_VERBOSE("Released: Escape 4 (Player 4 Start + Button 1 + Button 3)\n");
    }
#endif // S_ESC4

#ifdef S_COIN3
    // Coin 3
    if (buttonsData & 0x1)
    {
        if (fioKeys.k3coin_flag == 0)
        {
            diEvent.ki.dwFlags = S_COIN3_FLAGS;
            diEvent.ki.wScan = S_COIN3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3coin_flag = 1;
            LOG_VERBOSE("Pressed: Coin 3\n");
        }
    }
    else if (fioKeys.k3coin_flag == 1)
    {
        diEvent.ki.dwFlags = S_COIN3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_COIN3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3coin_flag = 0;
        LOG_VERBOSE("Released: Coin 3\n");
    }
#endif // S_COIN3

#ifdef S_COIN4
    // Coin 4
    if (buttonsData & 0x2)
    {
        if (fioKeys.k4coin_flag == 0)
        {
            diEvent.ki.dwFlags = S_COIN4_FLAGS;
            diEvent.ki.wScan = S_COIN4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4coin_flag = 1;
            LOG_VERBOSE("Pressed: Coin 4\n");
        }
    }
    else if (fioKeys.k4coin_flag == 1)
    {
        diEvent.ki.dwFlags = S_COIN4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_COIN4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4coin_flag = 0;
        LOG_VERBOSE("Released: Coin 4\n");
    }
#endif // S_COIN4

#ifdef S_SERVICE3
    // Service 3
    if (buttonsData & 0x4)
    {
        if (fioKeys.k3service_flag == 0)
        {
            diEvent.ki.dwFlags = S_SERVICE3_FLAGS;
            diEvent.ki.wScan = S_SERVICE3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3service_flag = 1;
            LOG_VERBOSE("Pressed: Service 3\n");
        }
    }
    else if (fioKeys.k3service_flag == 1)
    {
        diEvent.ki.dwFlags = S_SERVICE3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_SERVICE3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3service_flag = 0;
        LOG_VERBOSE("Released: Service 3\n");
    }
#endif // S_SERVICE3

#ifdef S_SERVICE4
    // Service 4
    if (buttonsData & 0x8)
    {
        if (fioKeys.k4service_flag == 0)
        {
            diEvent.ki.dwFlags = S_SERVICE4_FLAGS;
            diEvent.ki.wScan = S_SERVICE4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4service_flag = 1;
            LOG_VERBOSE("Pressed: Service 4\n");
        }
    }
    else if (fioKeys.k4service_flag == 1)
    {
        diEvent.ki.dwFlags = S_SERVICE4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_SERVICE4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4service_flag = 0;
        LOG_VERBOSE("Released: Service 4\n");
    }
#endif // S_SERVICE4

#ifdef P3_START
    // Player 3 Start
    if (buttonsData & 0x10)
    {
        if (fioKeys.k3start_flag == 0)
        {
            diEvent.ki.dwFlags = P3_START_FLAGS;
            diEvent.ki.wScan = P3_START;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3start_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Start\n");
        }
    }
    else if (fioKeys.k3start_flag == 1)
    {
        diEvent.ki.dwFlags = P3_START_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_START;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3start_flag = 0;
        LOG_VERBOSE("Released: Player 3 Start\n");
    }
#endif // P3_START

#ifdef P4_START
    // Player 4 Start
    if (buttonsData & 0x20)
    {
        if (fioKeys.k4start_flag == 0)
        {
            diEvent.ki.dwFlags = P4_START_FLAGS;
            diEvent.ki.wScan = P4_START;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4start_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Start\n");
        }
    }
    else if (fioKeys.k4start_flag == 1)
    {
        diEvent.ki.dwFlags = P4_START_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_START;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4start_flag = 0;
        LOG_VERBOSE("Released: Player 4 Start\n");
    }
#endif // P4_START

#ifdef S_TEST2
    // Test 2
    if (buttonsData & 0x40)
    {
        if (fioKeys.k2test_flag == 0)
        {
            diEvent.ki.dwFlags = S_TEST2_FLAGS;
            diEvent.ki.wScan = S_TEST2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2test_flag = 1;
            LOG_VERBOSE("Pressed: Test 2\n");
        }
    }
    else if (fioKeys.k2test_flag == 1)
    {
        diEvent.ki.dwFlags = S_TEST2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_TEST2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2test_flag = 0;
        LOG_VERBOSE("Released: Test 2\n");
    }
#endif // S_TEST2

#ifdef S_TILT2
    // Tilt 2
    if (buttonsData & 0x80)
    {
        if (fioKeys.k2tilt_flag == 0)
        {
            diEvent.ki.dwFlags = S_TILT2_FLAGS;
            diEvent.ki.wScan = S_TILT2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k2tilt_flag = 1;
            LOG_VERBOSE("Pressed: Tilt 2\n");
        }
    }
    else if (fioKeys.k2tilt_flag == 1)
    {
        diEvent.ki.dwFlags = S_TILT2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = S_TILT2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k2tilt_flag = 0;
        LOG_VERBOSE("Released: Tilt 2\n");
    }
#endif // S_TILT2

#ifdef P3_UP
    // Player 3 Up
    if (buttonsData & 0x100)
    {
        if (fioKeys.k3up_flag == 0)
        {
            diEvent.ki.dwFlags = P3_UP_FLAGS;
            diEvent.ki.wScan = P3_UP;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3up_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Up\n");
        }
    }
    else if (fioKeys.k3up_flag == 1)
    {
        diEvent.ki.dwFlags = P3_UP_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_UP;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3up_flag = 0;
        LOG_VERBOSE("Released: Player 3 Up\n");
    }
#endif // P3_UP

#ifdef P4_UP
    // Player 4 Up
    if (buttonsData & 0x200)
    {
        if (fioKeys.k4up_flag == 0)
        {
            diEvent.ki.dwFlags = P4_UP_FLAGS;
            diEvent.ki.wScan = P4_UP;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4up_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Up\n");
        }
    }
    else if (fioKeys.k4up_flag == 1)
    {
        diEvent.ki.dwFlags = P4_UP_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_UP;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4up_flag = 0;
        LOG_VERBOSE("Released: Player 4 Up\n");
    }
#endif // P4_UP

#ifdef P3_DOWN
    // Player 3 Down
    if (buttonsData & 0x400)
    {
        if (fioKeys.k3down_flag == 0)
        {
            diEvent.ki.dwFlags = P3_DOWN_FLAGS;
            diEvent.ki.wScan = P3_DOWN;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3down_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Down\n");
        }
    }
    else if (fioKeys.k3down_flag == 1)
    {
        diEvent.ki.dwFlags = P3_DOWN_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_DOWN;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3down_flag = 0;
        LOG_VERBOSE("Released: Player 3 Down\n");
    }
#endif // P3_DOWN

#ifdef P4_DOWN
    // Player 4 Down
    if (buttonsData & 0x800)
    {
        if (fioKeys.k4down_flag == 0)
        {
            diEvent.ki.dwFlags = P4_DOWN_FLAGS;
            diEvent.ki.wScan = P4_DOWN;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4down_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Down\n");
        }
    }
    else if (fioKeys.k4down_flag == 1)
    {
        diEvent.ki.dwFlags = P4_DOWN_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_DOWN;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4down_flag = 0;
        LOG_VERBOSE("Released: Player 4 Down\n");
    }
#endif // P4_DOWN

#ifdef P3_LEFT
    // Player 3 Left
    if (buttonsData & 0x1000)
    {
        if (fioKeys.k3left_flag == 0)
        {
            diEvent.ki.dwFlags = P3_LEFT_FLAGS;
            diEvent.ki.wScan = P3_LEFT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3left_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Left\n");
        }
    }
    else if (fioKeys.k3left_flag == 1)
    {
        diEvent.ki.dwFlags = P3_LEFT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_LEFT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3left_flag = 0;
        LOG_VERBOSE("Released: Player 3 Left\n");
    }
#endif // P3_LEFT

#ifdef P4_LEFT
    // Player 4 Left
    if (buttonsData & 0x2000)
    {
        if (fioKeys.k4left_flag == 0)
        {
            diEvent.ki.dwFlags = P4_LEFT_FLAGS;
            diEvent.ki.wScan = P4_LEFT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4left_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Left\n");
        }
    }
    else if (fioKeys.k4left_flag == 1)
    {
        diEvent.ki.dwFlags = P4_LEFT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_LEFT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4left_flag = 0;
        LOG_VERBOSE("Released: Player 4 Left\n");
    }
#endif // P4_LEFT

#ifdef P3_RIGHT
    // Player 3 Right
    if (buttonsData & 0x4000)
    {
        if (fioKeys.k3right_flag == 0)
        {
            diEvent.ki.dwFlags = P3_RIGHT_FLAGS;
            diEvent.ki.wScan = P3_RIGHT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3right_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Right\n");
        }
    }
    else if (fioKeys.k3right_flag == 1)
    {
        diEvent.ki.dwFlags = P3_RIGHT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_RIGHT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3right_flag = 0;
        LOG_VERBOSE("Released: Player 3 Right\n");
    }
#endif // P3_RIGHT

#ifdef P4_RIGHT
    // Player 4 Right
    if (buttonsData & 0x8000)
    {
        if (fioKeys.k4right_flag == 0)
        {
            diEvent.ki.dwFlags = P4_RIGHT_FLAGS;
            diEvent.ki.wScan = P4_RIGHT;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4right_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Right\n");
        }
    }
    else if (fioKeys.k4right_flag == 1)
    {
        diEvent.ki.dwFlags = P4_RIGHT_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_RIGHT;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4right_flag = 0;
        LOG_VERBOSE("Released: Player 4 Right\n");
    }
#endif // P4_RIGHT

#ifdef P3_BTN1
    // Player 3 Button 1
    if (buttonsData & 0x10000)
    {
        if (fioKeys.k3b1_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN1_FLAGS;
            diEvent.ki.wScan = P3_BTN1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b1_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 1\n");
        }
    }
    else if (fioKeys.k3b1_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b1_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 1\n");
    }
#endif // P3_BTN1

#ifdef P4_BTN1
    // Player 4 Button 1
    if (buttonsData & 0x20000)
    {
        if (fioKeys.k4b1_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN1_FLAGS;
            diEvent.ki.wScan = P4_BTN1;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b1_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 1\n");
        }
    }
    else if (fioKeys.k4b1_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN1_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN1;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b1_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 1\n");
    }
#endif // P4_BTN1

#ifdef P3_BTN2
    // Player 3 Button 2
    if (buttonsData & 0x40000)
    {
        if (fioKeys.k3b2_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN2_FLAGS;
            diEvent.ki.wScan = P3_BTN2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b2_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 2\n");
        }
    }
    else if (fioKeys.k3b2_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b2_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 2\n");
    }
#endif // P3_BTN2

#ifdef P4_BTN2
    // Player 4 Button 2
    if (buttonsData & 0x80000)
    {
        if (fioKeys.k4b2_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN2_FLAGS;
            diEvent.ki.wScan = P4_BTN2;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b2_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 2\n");
        }
    }
    else if (fioKeys.k4b2_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN2_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN2;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b2_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 2\n");
    }
#endif // P4_BTN2

#ifdef P3_BTN3
    // Player 3 Button 3
    if (buttonsData & 0x100000)
    {
        if (fioKeys.k3b3_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN3_FLAGS;
            diEvent.ki.wScan = P3_BTN3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b3_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 3\n");
        }
    }
    else if (fioKeys.k3b3_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b3_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 3\n");
    }
#endif // P3_BTN3

#ifdef P4_BTN3
    // Player 4 Button 3
    if (buttonsData & 0x200000)
    {
        if (fioKeys.k4b3_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN3_FLAGS;
            diEvent.ki.wScan = P4_BTN3;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b3_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 3\n");
        }
    }
    else if (fioKeys.k4b3_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN3_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN3;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b3_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 3\n");
    }
#endif // P4_BTN3

#ifdef P3_BTN4
    // Player 3 Button 4
    if (buttonsData & 0x400000)
    {
        if (fioKeys.k3b4_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN4_FLAGS;
            diEvent.ki.wScan = P3_BTN4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b4_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 4\n");
        }
    }
    else if (fioKeys.k3b4_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b4_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 4\n");
    }
#endif // P3_BTN4

#ifdef P4_BTN4
    // Player 4 Button 4
    if (buttonsData & 0x800000)
    {
        if (fioKeys.k4b4_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN4_FLAGS;
            diEvent.ki.wScan = P4_BTN4;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b4_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 4\n");
        }
    }
    else if (fioKeys.k4b4_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN4_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN4;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b4_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 4\n");
    }
#endif // P4_BTN4

#ifdef P3_BTN5
    // Player 3 Button 5
    if (buttonsData & 0x1000000)
    {
        if (fioKeys.k3b5_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN5_FLAGS;
            diEvent.ki.wScan = P3_BTN5;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b5_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 5\n");
        }
    }
    else if (fioKeys.k3b5_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN5_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN5;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b5_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 5\n");
    }
#endif // P3_BTN5

#ifdef P4_BTN5
    // Player 4 Button 5
    if (buttonsData & 0x2000000)
    {
        if (fioKeys.k4b5_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN5_FLAGS;
            diEvent.ki.wScan = P4_BTN5;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b5_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 5\n");
        }
    }
    else if (fioKeys.k4b5_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN5_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN5;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b5_flag = 0;
        LOG_VERBOSE("Pressed: Player 4 Button 5\n");
    }
#endif // P4_BTN5

#ifdef P3_BTN6
    // Player 3 Button 6
    if (buttonsData & 0x4000000)
    {
        if (fioKeys.k3b6_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN6_FLAGS;
            diEvent.ki.wScan = P3_BTN6;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b6_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 6\n");
        }
    }
    else if (fioKeys.k3b6_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN6_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN6;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b6_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 6\n");
    }
#endif // P3_BTN6

#ifdef P4_BTN6
    // Player 4 Button 6
    if (buttonsData & 0x8000000)
    {
        if (fioKeys.k4b6_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN6_FLAGS;
            diEvent.ki.wScan = P4_BTN6;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b6_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 6\n");
        }
    }
    else if (fioKeys.k4b6_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN6_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN6;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b6_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 6\n");
    }
#endif // P4_BTN6

#ifdef P3_BTN7
    // Player 3 Button 7
    if (buttonsData & 0x10000000)
    {
        if (fioKeys.k3b7_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN7_FLAGS;
            diEvent.ki.wScan = P3_BTN7;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b7_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 7\n");
        }
    }
    else if (fioKeys.k3b7_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN7_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN7;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b7_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 7\n");
    }
#endif // P3_BTN7

#ifdef P4_BTN7
    // Player 4 Button 7
    if (buttonsData & 0x20000000)
    {
        if (fioKeys.k4b7_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN7_FLAGS;
            diEvent.ki.wScan = P4_BTN7;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b7_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 7\n");
        }
    }
    else if (fioKeys.k4b7_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN7_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN7;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b7_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 7\n");
    }
#endif // P4_BTN7

#ifdef P3_BTN8
    // Player 3 Button 8
    if (buttonsData & 0x40000000)
    {
        if (fioKeys.k3b8_flag == 0)
        {
            diEvent.ki.dwFlags = P3_BTN8_FLAGS;
            diEvent.ki.wScan = P3_BTN8;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k3b8_flag = 1;
            LOG_VERBOSE("Pressed: Player 3 Button 8\n");
        }
    }
    else if (fioKeys.k3b8_flag == 1)
    {
        diEvent.ki.dwFlags = P3_BTN8_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P3_BTN8;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k3b8_flag = 0;
        LOG_VERBOSE("Released: Player 3 Button 8\n");
    }
#endif // P3_BTN8

#ifdef P4_BTN8
    // Player 4 Button 8
    if (buttonsData & 0x80000000)
    {
        if (fioKeys.k4b8_flag == 0)
        {
            diEvent.ki.dwFlags = P4_BTN8_FLAGS;
            diEvent.ki.wScan = P4_BTN8;
            SendInput(1, &diEvent, sizeof(INPUT));
            fioKeys.k4b8_flag = 1;
            LOG_VERBOSE("Pressed: Player 4 Button 8\n");
        }
    }
    else if (fioKeys.k4b8_flag == 1)
    {
        diEvent.ki.dwFlags = P4_BTN8_FLAGS | KEYEVENTF_KEYUP;
        diEvent.ki.wScan = P4_BTN8;
        SendInput(1, &diEvent, sizeof(INPUT));
        fioKeys.k4b8_flag = 0;
        LOG_VERBOSE("Released: Player 4 Button 8\n");
    }
#endif // P4_BTN8
}
#endif // P3P4_ENABLED


int main()
{
    HANDLE pseudoHProcess = GetCurrentProcess();
    HANDLE pseudoHMainThread = GetCurrentThread();

    if (!DuplicateHandle(pseudoHProcess, pseudoHMainThread, pseudoHProcess,
        &hMainThread, NULL, FALSE, DUPLICATE_SAME_ACCESS))
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get handle to main thread. Error code: %d\n", error);
        return 1;
    }

    if (!SetConsoleCtrlHandler(ctrlHandler, TRUE))
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to set control handler. Error code: %d\n", error);
        return 1;
    }

    if (!FIO_Open())
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to open iDmacDrv device. Error code: %d\n", error);
        return 1;
    }

    if (!assignPlayers())
    {
        LOG_ERROR("Failed to detect any I/O connection for any player\n");
        return 1;
    }

    FIOKEYS fioKeys = { 0 };

    INPUT diEvent = { 0 };
    diEvent.type = INPUT_KEYBOARD;

    // Determine the preferred timer resolution, within system capabilities.
    DWORD preferredTimerResolution;
    TIMECAPS timeCapabilities;
    if (timeGetDevCaps(&timeCapabilities, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        preferredTimerResolution = POLLING_INTERVAL;
    }
    else
    {
        preferredTimerResolution = min(
            max(timeCapabilities.wPeriodMin, POLLING_INTERVAL),
            timeCapabilities.wPeriodMax);
    }

    // Ask the system to start using the preferred timer resolution.
    if (timeBeginPeriod(preferredTimerResolution) != TIMERR_NOERROR)
    {
        LOG_VERBOSE("Timer resolution: Unknown\n");
    }
    else
    {
        LOG_VERBOSE("Timer resolution: %dms\n", preferredTimerResolution);
    }

    LOG_VERBOSE("Starting to poll input registers...\n");
    if (buttonsAddressP3P4 == NULL)
    {
        while (keepPolling)
        {
            pollP1P2(fioKeys, diEvent);
            Sleep(POLLING_INTERVAL);
        }
    }
    else
    {
        while (keepPolling)
        {
            pollP1P2(fioKeys, diEvent);
#ifdef P3P4_ENABLED
            pollP3P4(fioKeys, diEvent);
#endif
            Sleep(POLLING_INTERVAL);
        }
    }
    LOG_VERBOSE("Finished polling input registers\n");

    // Ask the system to stop using the preferred timer resolution.
    timeEndPeriod(preferredTimerResolution);

    return 0;
}
