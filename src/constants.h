#ifndef ASCIIROIDS_CONSTANTS_H
#define ASCIIROIDS_CONSTANTS_H

#define MAX_PATH 255
#define PI 3.14159265358979323846f
#define DEG_360 360.0f

// player constants
#define YAW_DEG_PER_SEC 180.0f
#define BOOST_ACCELERATION 20.0f

#define BULLET_SPEED 20.0f
#define BULLET_LIFE_FRAMES 60

// account for there being a difference in the height and width of characters.
// because chars are taller than they are wide, moving north/south is much faster than east/west.
// this factor accounts for that to make the speed seem smooth
#define CHAR_SIZE_FACTOR 2.5f

#define U32_MAX_DIGITS 10

#endif  // ASCIIROIDS_CONSTANTS_H
