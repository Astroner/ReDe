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

    #define CHECK_ELSE(condition, elseCode, ...)\
        do {\
            int CONDITION_VALUE = (condition);\
            if(CONDITION_VALUE < 0) {\
                printf("LOGS '%s' Status: %d  ", logs__scope__name, CONDITION_VALUE);\
                printf(__VA_ARGS__);\
                printf("\n");\
                elseCode;\
            }\
        } while(0);

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

    #define CHECK_ELSE(condition, elseCode, ...)\
        do {\
            int CONDITION_VALUE = (condition);\
            if(CONDITION_VALUE < 0) {\
                elseCode;\
            }\
        } while(0);

#endif // REDE_DO_LOGS


#endif // LOGS_H