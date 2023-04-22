#if !defined(LOGS_H)
#define LOGS_H

#include <stdio.h>

#if defined(REDE_DO_LOGS)
    #define LOGS_SCOPE(name)\
        char* logs__scope__name = #name;\
        printf("LOGS '%s'\n", logs__scope__name);\

    #define LOG(...)\
        do {\
            printf("LOGS '%s' ", logs__scope__name);\
            printf(__VA_ARGS__);\
        } while(0);\
    
    #define LOG_LN(...)\
        do {\
            LOG(__VA_ARGS__);\
            printf("\n");\
        } while(0);\

    #define CHECK(condition, modifier, ...)\
        do {\
            int status = (condition);\
            if(status < 0) {\
                printf("LOGS '%s' Status: %d  ", logs__scope__name, status);\
                printf(__VA_ARGS__);\
                printf("\n");\
                return status + (modifier);\
            }\
        } while(0);\

    #define LOGS_ONLY(code) code

#else
    #define LOGS_SCOPE(name)
    #define LOG(...)
    #define LOG_LN(...)

    #define CHECK(condition, modifier, ...)\
        do {\
            int status = (condition);\
            if(status < 0) return status + (modifier);\
        } while(0);\

    #define LOGS_ONLY(code)

#endif // REDE_DO_LOGS


#endif // LOGS_H
