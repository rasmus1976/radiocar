#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
#include <windows.h>
#include <pthread.h>
#include "stdbool.h"

#define MAX_X   64
#define MAX_Y   64

static void ErrorExit(LPSTR);
static bool KeyEventProc(KEY_EVENT_RECORD ker);

// Global variables are here for example, avoid that.
DWORD fdwSaveOldMode;
HANDLE hStdin;

char room_matrix[MAX_Y][MAX_X];

typedef enum key_cmd { 
    key_cmd_none = 0,
    key_cmd_fw,
    key_cmd_bw,
    key_cmd_right,
    key_cmd_left
} key_cmd_t;

typedef enum directon { 
    dir_none = 0,
    dir_north,
    dir_south,
    dir_east,
    dir_west,
} directon_t;

typedef struct car {
    key_cmd_t cmd ;
    directon_t dir ;
    struct {
        COORD currentPos ;
		COORD lastPos;
		int maxX;
        int maxY;
    } map ;
} car_t ;

car_t car = {   .cmd = key_cmd_none,
                .dir = dir_none, 
                .map.currentPos.X = 0,
                .map.currentPos.Y = 0,
                .map.maxX = 32,
                .map.maxY = 64,
              };

pthread_mutex_t car_lock;


void draw_room(int maxx, int maxy)
{
	//reset
	memset(&room_matrix[0], 32, MAX_Y * MAX_X);
	
	//first row...
	memset(&room_matrix[0][0], '-', maxx);
	room_matrix[0][0] = '|';
	room_matrix[0][maxx - 1] = '|';
	room_matrix[0][maxx] = '\n';
	room_matrix[0][maxx + 1] = 0;
	
	//then others...
	int row;
	for(row = 1; row < maxy; row ++)
	{
		// set space to avoid null term...
	//	memset(&room_matrix[row][0], 32, MAX_X);
		room_matrix[row][0] = '|';
		room_matrix[row][maxx - 1] = '|';
		room_matrix[row][maxx] = '\n' ;
		room_matrix[row][maxx + 1] = 0;
	}
	
	//and last one...
	memset(&room_matrix[maxy][0], '-', maxx);
	room_matrix[maxy][0] = '|';
	room_matrix[maxy][maxx - 1] = '|';
	room_matrix[maxy][maxx] = '\n';
	room_matrix[maxy][maxx + 1] = 0;
		
	system("cls");
	printf("Keys f = forward, b = back, r = right, l = left, q = quit/exit (max 60 seconds)\n\n");
	for(row = 0; row <= maxy; row ++)
		printf("%s", (char*)&room_matrix[row][0]);
	
	/*
	char line[64] ;
    char lineArea[64];
    memset(line, 0, sizeof(line));
    memset(line, '-', maxx);
    memset(lineArea, 32, sizeof(lineArea));
    line[0] = '|'; 
    lineArea[0] = '|';
    line[maxx - 1] = '|';
    lineArea[maxx - 1] = '|';
    //lineArea[maxx] = '\n';

    memset(&lineArea[maxx], 0, 64 - maxx) ;

	return ;

    printf("Keys f = forward, b = back, r = right, l = left, q = quit/exit (max 60 seconds)\n\n");
    printf("%s\n", (char*)line);

    for(int k = 1; k < maxy; k ++)
    {
        printf("%s\n", (char*)lineArea);
      //  Sleep(1000);
    }

    printf("%s\n", (char*)line);
*/
}

void set_car_params(directon_t dir, key_cmd_t cmd)
{
    pthread_mutex_lock(&car_lock);

    if(dir != dir_none)
    {
        car.dir = dir ;
    }

    if(cmd != key_cmd_none)
    {
     //   printf("setting cmd = %d\n", cmd);
        car.cmd = cmd ;
    }

    pthread_mutex_unlock(&car_lock);
}

void set_car_map(int maxX, int maxY, char dir)
{
    pthread_mutex_lock(&car_lock);

    if(maxX > MAX_X) maxX = MAX_X ;
    if(maxY > MAX_Y) maxY = MAX_Y ;

    car.map.maxX = maxX ;
    car.map.maxY = maxY ;

    car.map.currentPos.X = maxX / 2 ;
    car.map.currentPos.Y = maxY / 2 ;

    switch(dir)
    {
        case 'n': car.dir = dir_north ; break ;
        case 's': car.dir = dir_south ; break ;
        case 'e': car.dir = dir_east ; break ;
        case 'w': car.dir = dir_west ; break ;
        default: break ;
    }

    pthread_mutex_unlock(&car_lock);
}

static bool posWithinRoom(car_t* car)
{
	if(car == NULL) return false ;
	
	if(car->map.currentPos.X == 0) return false ;
	if(car->map.currentPos.Y == 0) return false ;
	if(car->map.currentPos.X >= car->map.maxX) return false ;
	if(car->map.currentPos.Y >= car->map.maxY) return false ;

	return true ;
}

void *runDraw(void* arg)
{
	bool changed = false ;
    HANDLE* out = (HANDLE*)arg ;
    COORD pos ;
    pos.X = 0 ;
    pos.Y = 0 ;

    int maxx = car.map.maxX, maxy = car.map.maxY ;

    if(out == NULL) return NULL;

	if(maxy < 2) maxy = 2;
	if(maxx < 2) maxx = 2;

	draw_room(maxx, maxy);
	    
   // Sleep(10000);

    car.map.currentPos.X = maxx / 2 ;
    car.map.currentPos.Y = maxy / 2 ;

    while(1)
    {
		changed = false ;
        pthread_mutex_lock(&car_lock);
		
		car.map.lastPos.X = car.map.currentPos.X ;
		car.map.lastPos.Y = car.map.currentPos.Y ;
		
        switch(car.cmd)
        {
            case key_cmd_bw : car.map.currentPos.Y ++ ; changed = true ; break ;
            case key_cmd_fw : car.map.currentPos.Y -- ; changed = true ; break ;
            case key_cmd_right : car.map.currentPos.X ++ ; changed = true ; break ;
            case key_cmd_left : car.map.currentPos.X -- ; changed = true ; break ;
			
			case key_cmd_none : 
            default: 
				break ;
        }
		
		if(!posWithinRoom(&car))
		{
			printf("Failure, hiting wall!\n");
			Sleep(3000);
			system("cls");
			exit(0);
		}		
	
		SetConsoleCursorPosition(out, car.map.lastPos);
		printf(" ");
		SetConsoleCursorPosition(out, car.map.currentPos);
		printf("*");
		
        pthread_mutex_unlock(&car_lock);
        Sleep(200);
    }
}

int main()
{
    int i = 0;
    char* s = "*";

    DWORD fdwMode, cNumRead;
    INPUT_RECORD irInBuf[8];
    DWORD bufferSize = 0;

    if (pthread_mutex_init(&car_lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 0;
    }

    int base = 0, hight = 0;
    char dir[4]; 
    printf("Enter size, first base: ");
    scanf("%d", &base);
    printf("And hight: ");
    scanf("%d", &hight);
    printf("Finally direction (n,s,e,w): ");
    scanf("%d", &dir);

    printf("base = %d and height = %d, direction = %s\n", base, hight, dir);

    set_car_map(base, hight, dir[0]);

    system("cls");

    hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if (hStdin==INVALID_HANDLE_VALUE){
        printf("Invalid handle value.\n");
        exit(EXIT_FAILURE);
    }

    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_PROCESSED_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    
    HANDLE hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

    pthread_t thread1 ;
    int iret1 = pthread_create( &thread1, NULL, runDraw, (void*)hConsoleOut);
    
    while (i < 6000) {
        // The goal of this program is to print a line of stars
        //printToCoordinates(i, 5, s);
        i++;

        GetNumberOfConsoleInputEvents(hStdin, &bufferSize);

        // ReadConsoleInput block if the buffer is empty
        if (bufferSize > 0) {
            if (! ReadConsoleInput(
                    hStdin,      // input buffer handle
                    irInBuf,     // buffer to read into
                    8, 		     // size of read buffer
                    &cNumRead) ) // number of records read
                ErrorExit("ReadConsoleInput");

            // This code is not rock solid, you should iterate over
            // irInBuf to get what you want, the last event may not contain what you expect
            // Once again you'll find an event constant list on Microsoft documentation
            if(cNumRead >= 1)
			{
				if (irInBuf[cNumRead-1].EventType == KEY_EVENT) {
					if(KeyEventProc(irInBuf[cNumRead-1].Event.KeyEvent))
					{
						 pthread_kill(thread1, 0);
						// pt	hread_cancel(thread1);
						goto end ;   
					}
				}
			}

        }

        Sleep(100);
    }
    // Setting the console back to normal
end:
    SetConsoleMode(hStdin, fdwSaveOldMode);
    CloseHandle(hStdin);

    printf("\nFIN\n");

    return 0;
}

static void ErrorExit (LPSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

static bool KeyEventProc(KEY_EVENT_RECORD ker)
{
   // printf("Key event: \"%c\" ", ker.uChar.AsciiChar);

    if(ker.uChar.AsciiChar == 'q') 
        return true ;

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

    return false ;
}
