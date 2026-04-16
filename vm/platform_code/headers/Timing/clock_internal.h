#if defined(PLT_WIN)
    #include<win/Timing/win_clock.c>

#elif defined(PLT_LINUX)
    #include<linux/Timing/linux_clock.c>
#endif