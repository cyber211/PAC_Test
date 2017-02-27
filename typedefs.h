#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include "QString.h"

typedef enum
{
    Port_Equipment_5520A = 0,
    Port_Equipment_6270 = 1,
    Port_Equipment_7252I = 2,
    Port_UUT_1 = 3,
    Port_UUT_2 = 4,
    Port_UUT_3 = 5,
    Port_UUT_4 = 6,
    Port_UUT_5 = 7,
    Port_UUT_6 = 8,
    Port_UUT_7 = 9,
    Port_UUT_8 = 10
} PortType;

typedef enum
{
    Step_UUT_1 = 0,
    Step_UUT_2 = 1,
    Step_UUT_3 = 2,
    Step_UUT_4 = 3,

} Step_UUT;



#endif // TYPEDEFS_H
