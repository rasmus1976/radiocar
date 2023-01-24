#include <stdlib.h>
#include "stdbool.h"

#define MAX_X   64 
#define MAX_Y   64

#define MIN_X	12 //no use drawing a room smaller than 12x12n pixels...
#define MIN_Y	12

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

//API functions ->
void *runDraw(void* arg);
void set_car_map(int maxX, int maxY, char dir);
void set_car_speed(int speed);
void set_car_params(directon_t dir, key_cmd_t cmd);

