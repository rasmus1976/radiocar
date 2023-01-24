#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <pthread.h>
#include "stdbool.h"
#include "render.h"

//function prototypes
//static void errorExit(char* lpszMessage, HANDLE hStdin, DWORD fdwSaveOldMode);
static void errorExit(char* lpszMessage);
static bool handleKeyInput(KEY_EVENT_RECORD ker);

//these could quite easily be made local vars
DWORD fdwSaveOldMode;
HANDLE hStdin;

int main()
{
	int i = 0;
    DWORD fdwMode, cNumRead;
    INPUT_RECORD irInBuf[8];
    DWORD bufferSize = 0;
	HANDLE hConsoleOut ;
	pthread_t thread1 ;

    //get user input...
    int base = 0, hight = 0, speed = 5;
    char dir[4]; 
    printf("Enter size, first base (min 12 max 64): ");
    scanf("%d", &base);
    printf("And hight (min 12, max 64): ");
    scanf("%d", &hight);
	printf("Speed (min 1, max 10): ");
    scanf("%d", &speed);
    //printf("Finally direction (n,s,e,w): ");
    //scanf("%d", &dir);
		
	
    set_car_map(base, hight, dir[0], false);
	set_car_speed(speed, false);

    system("cls");
	
	hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if (hStdin==INVALID_HANDLE_VALUE){
        errorExit("Invalid handle");
    }
	
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        errorExit("GetConsoleMode");

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        errorExit("SetConsoleMode");
    
    hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsoleOut==INVALID_HANDLE_VALUE){
        errorExit("Invalid output handle");
    }

	//ok, lets start a thread to do the actual rendering... send console out handle
	//this function/process will only handle input and forward it to rendering thread.
	//hence, all the "fun stuff" is done in runDraw(), passed as thread starting point
    (void)pthread_create( &thread1, NULL, runDraw, (void*)hConsoleOut);
    	
    while (i < 600) 
	{
        // very simple "timeout"... let program run max 600 loops of ~100 ms => 1 minute... 
		i++;

        GetNumberOfConsoleInputEvents(hStdin, &bufferSize);

        // ReadConsoleInput block if the buffer is empty
        if (bufferSize > 0) 
		{
            if (! ReadConsoleInput( hStdin, irInBuf, 8, &cNumRead) )
                errorExit("ReadConsoleInput");

            //Handle possible input and send to renderer... (through mutex protected variable "car")
			if(cNumRead >= 1)
			{
				if (irInBuf[cNumRead-1].EventType == KEY_EVENT) 
				{
					if(!handleKeyInput(irInBuf[cNumRead-1].Event.KeyEvent))
					{
						//quit gotten, exit
						pthread_kill(thread1, 0);
						goto end ;   
					}
				}
			}
        }

        Sleep(100);
    }
	
end:
    SetConsoleMode(hStdin, fdwSaveOldMode);
    CloseHandle(hStdin);
	CloseHandle(hConsoleOut);

    printf("DONE: success, exit.\n");

    return 0;
}

static void errorExit (char* lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

static bool handleKeyInput(KEY_EVENT_RECORD ker)
{
    if(ker.uChar.AsciiChar == 'q') //let calling function exit/quit app 
        return false ;
/*
    if(ker.bKeyDown)
        printf("key pressed\n");
    else printf("key released\n");
*/
    switch(ker.uChar.AsciiChar)
    {
        case 'f': set_car_params(dir_none, key_cmd_fw); break ;
        case 'b': set_car_params(dir_none, key_cmd_bw); break ;
        case 'r': set_car_params(dir_none, key_cmd_right); break ;
        case 'l': set_car_params(dir_none, key_cmd_left); break ;
        default: break ;    
    }

	// all good, continue...
    return true ;
}
