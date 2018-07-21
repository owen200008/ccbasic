#ifndef BASICTEST_HEADDEFINE_H
#define BASICTEST_HEADDEFINE_H

#ifdef _DEBUG
#define TIMES_FAST 500000
#else
#define TIMES_FAST 5000000
#endif


//func call is success
#define PrintSuccessOrFail(Func)\
if(Func())\
    printf("Func: %s Success!\n", #Func);\
else\
    printf("Func: %s Fail???\n", #Func);

//use time
#define PrintUseTime(calcName, totalPerformance)\
[](DWORD dwUseTime, DWORD dwTotalUseTime){\
    printf("CalcTime:%s Perform:%.3f/ms TotalUseTime:%d LastUseTime:%d\n", calcName, (double)totalPerformance/dwTotalUseTime, dwTotalUseTime, dwUseTime);\
}


#endif