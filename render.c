#include "render.h"

// protoypes
static void draw_room(int maxx, int maxy);
static bool posWithinRoom(car_t* car);

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

void set_car_speed(int speed, bool lock)
{
	if(speed < 1) speed = 1;
	if(speed > 10) speed = 10;
	
	if(lock) pthread_mutex_lock(&car_lock);
	car.speed = speed ;
	if(lock) pthread_mutex_unlock(&car_lock);
}

//always mutex protected, continously called upon keyboard events from main
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

void set_car_map(int maxX, int maxY, char dir, bool lock)
{
    if(lock) pthread_mutex_lock(&car_lock);

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

    if(lock) pthread_mutex_unlock(&car_lock);
}

void *runDraw(void* arg)
{
    HANDLE* out = (HANDLE*)arg ;
 
    if(out == NULL) return NULL;

	if (pthread_mutex_init(&car_lock, NULL) != 0)
    {
        exit(0);
    }

	draw_room(car.map.maxX, car.map.maxY);
	    
	//run this thread until stopped/killed...
    while(true)
    {
        pthread_mutex_lock(&car_lock);
		
		car.map.lastPos.X = car.map.currentPos.X ;
		car.map.lastPos.Y = car.map.currentPos.Y ;
		
        switch(car.cmd)
        {
			// This would be more complex taking into account the startdir... (and the current dir afterwards...)
            // for now, simply move up/back/right/left...
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
			printf("Failure, hitting wall!");
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

static bool posWithinRoom(car_t* car)
{
	if(car == NULL) return false ;
	
	if(car->map.currentPos.X == 0) return false ;
	if(car->map.currentPos.Y == 0) return false ;
	if(car->map.currentPos.X >= car->map.maxX) return false ;
	if(car->map.currentPos.Y >= car->map.maxY) return false ;

	return true ;
}
