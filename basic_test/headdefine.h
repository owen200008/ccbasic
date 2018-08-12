#ifndef BASICTEST_HEADDEFINE_H
#define BASICTEST_HEADDEFINE_H

#define REPEAT_TIMES 5
#ifdef _DEBUG
#define TIMES_FAST 500000
#else
#define TIMES_FAST 2000000
#endif


//func call is success
#define PrintSuccessOrFail(Func)\
{\
    char szFormat[128] = {'/'};\
    int nRetValue = sprintf(szFormat, "F:%s", #Func);\
    if(nRetValue > 0)\
        for(int i = nRetValue;i < 64;i++)\
            szFormat[i] = '/';\
    printf(szFormat);\
    printf("\n");\
    if(Func())\
        nRetValue = sprintf(szFormat, "R:Success");\
    else\
        nRetValue = sprintf(szFormat, "R:Fail???");\
    if(nRetValue > 0)\
        for(int i = nRetValue;i < 64;i++)\
            szFormat[i] = '/';\
    printf(szFormat);\
    printf("\n");\
}




#endif
