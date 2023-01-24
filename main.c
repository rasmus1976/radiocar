#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <pthread.h>
#include "stdbool.h"

#define MAX_X   64 
#define MAX_Y   64

#define MIN_X	12 //no use drawing a room smaller than 12x12n pixels...
#define MIN_Y	12

//function prototypes
//static void errorExit(char* lpszMessage, HANDLE hStdin, DWORD fdwSaveOldMode);
static void errorExit(char* lpszMessage);
static bool handleKeyInput(KEY_EVENT_RECORD ker);

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
    directon_t dir ; //starting dir, would affect coordinates (e.g. how turning would affect position)
    int speed ; // 1-10 where 10 is the fastest
	struct {
        COORD currentPos ;
		COORD lastPos;
		int maxX;
        int maxY;
    } map ;
} car_t ;

//global vars...

char room_matrix[MAX_Y + 1][MAX_X + 1]; // y == rows and thus x == lines...

car_t car = {   .cmd = key_cmd_none,
                .dir = dir_none,
				.speed = 5,
                .map.currentPos.X = 0,
                .map.currentPos.Y = 0,
                .map.maxX = 64,
                .map.maxY = 64,
              };

pthread_mutex_t car_lock;

//these could quite easily be made local vars
DWORD fdwSaveOldMode;
HANDLE hStdin;

static void draw_room(int maxx, int maxy)
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
	for(row = 1; row < maxy - 1; row ++)
	{
		// set space to avoid null term...
		room_matrix[row][0] = '|';
		room_matrix[row][maxx - 1] = '|';
		room_matrix[row][maxx] = '\n' ;
		room_matrix[row][maxx + 1] = 0;
	}
	
	//and last one...
	memset(&room_matrix[maxy - 1][0], '-', maxx);
	room_matrix[maxy - 1][0] = '|';
	room_matrix[maxy - 1][maxx - 1] = '|';
	room_matrix[maxy - 1][maxx] = '\n';
	room_matrix[maxy - 1][maxx + 1] = 0;
		
	system("cls");
	//cannot print all at once (i.e. &room_matrix[0][0] since each row is nulled and will mess up rendering...
	//well, we only do this once anyway (for now...)
	for(row = 0; row < maxy; row ++)
		printf("%s", (char*)&room_matrix[row][0]);
	
	printf("\nKeys f = forward, b = back, r = right, l = left, q = quit/exit (max 60 seconds)\n\n");

}

void set_car_speed(int speed)
{
	if(speed < 1) speed = 1;
	if(speed > 10) speed = 10;
	
	pthread_mutex_lock(&car_lock);
	car.speed = speed ;
	pthread_mutex_unlock(&car_lock);
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
        car.cmd = cmd ;
    }

    pthread_mutex_unlock(&car_lock);
}

void set_car_map(int maxX, int maxY, char dir)
{
    pthread_mutex_lock(&car_lock);

	if(maxY < MIN_Y) maxY = MIN_Y;
	if(maxX < MIN_X) maxX = MIN_X;
	if(maxY >= MAX_Y) maxY = MAX_Y - 1 ; // 0-indexed array...
	if(maxX >= MAX_X) maxX = MAX_X - 1 ; // 0-indexed array...

	car.map.maxY = maxY;
	car.map.maxX = maxX;
	
    car.map.currentPos.X = car.map.maxX / 2 ;
    car.map.currentPos.Y = car.map.maxY / 2 ;

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
    HANDLE* out = (HANDLE*)arg ;
 
    if(out == NULL) return NULL;

	draw_room(car.map.maxX, car.map.maxY);
	    
	//run this thread until stopped/killed...
    while(true)
    {
        pthread_mutex_lock(&car_lock);
		
		car.map.lastPos.X = car.map.currentPos.X ;
		car.map.lastPos.Y = car.map.currentPos.Y ;
		
        switch(car.cmd)
        {
            case key_cmd_bw : car.map.currentPos.Y ++ ; break ;
            case key_cmd_fw : car.map.currentPos.Y -- ; break ;
            case key_cmd_right : car.map.currentPos.X ++ ; break ;
            case key_cmd_left : car.map.currentPos.X -- ; break ;
			
			case key_cmd_none : 
            default: 
				break ;
        }
		
		if(!posWithinRoom(&car))
		{
			printf("Failure, hitting wall!\n");
			Sleep(3000);
			system("cls");
			exit(0);
		}		
	
		//first, set "blank" at previous position (otherwise we'll get a snake game ;) 
		SetConsoleCursorPosition(out, car.map.lastPos);
		printf(" ");
		SetConsoleCursorPosition(out, car.map.currentPos);
		printf("*");
		
        pthread_mutex_unlock(&car_lock);
        Sleep(1000 / car.speed);
    }
}

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
	
	if (pthread_mutex_init(&car_lock, NULL) != 0)
    {
        errorExit("mutex init failed");
    }
	
    set_car_map(base, hight, dir[0]);
	set_car_speed(speed);

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
