// Documentation: https://github.com/Astroner/macros#tests

#if !defined(TESTS_TYPES)
    #define TESTS_TYPES
    typedef void TESTS_ActionFunction(void);\
    struct TESTS_infoType {\
        int failLine;\
        char* testLabel;\
        char* testText; \
        int isNot;\
        char* operatorText;\
        char* expectText;\
        int status;\
        TESTS_ActionFunction* beforeEach;\
        TESTS_ActionFunction* afterEach;\
    };
#endif // TESTS_TYPES

#if !defined(TESTS_H)
#define TESTS_H

#include <stdarg.h>
#include <stddef.h>

#if !defined(TESTS_STD_PRINT)
    #include <stdio.h>
    #define TESTS_STD_PRINT printf
#endif // TESTS_PRINT

#if !defined(TESTS_STD_STRLEN)
    #include <string.h>
    #define TESTS_STD_STRLEN strlen
#endif // TESTS_STD_STRLEN

#if !defined(TESTS_STD_STRCMP)
    #include <string.h>
    #define TESTS_STD_STRCMP strcmp
#endif // TESTS_STD_STRCMP

#if !defined(TESTS_STD_MEMCMP)
    #include <string.h>
    #define TESTS_STD_MEMCMP memcmp
#endif // TESTS_STD_MEMCMP

#define CREATE_PRINTF_LIKE_FUNCTION(NAME, BUFFER_LENGTH)\
    char NAME##__buffer[BUFFER_LENGTH + 1] = { '\0' };\
    size_t NAME##__length = 0;\
    char* NAME##__lastString = NAME##__buffer;\
    size_t NAME##__lastStringLength = 0;\
\
    int NAME(const char* format, ...);\
    int NAME(const char* format, ...) {\
        if(NAME##__length == BUFFER_LENGTH) {\
            return -1;\
        }\
        va_list argptr;\
        va_start(argptr, format);\
        vsnprintf(NAME##__buffer + NAME##__length, BUFFER_LENGTH - NAME##__length + 1, format, argptr);\
        va_end(argptr);\
\
        NAME##__lastString = NAME##__buffer + NAME##__length;\
\
        NAME##__buffer[BUFFER_LENGTH] = '\0';\
        NAME##__lastStringLength = TESTS_STD_STRLEN(NAME##__buffer + NAME##__length);\
        NAME##__length += NAME##__lastStringLength;\
\
        return NAME##__lastStringLength;\
    }\
    \
    void NAME##__reset(void);\
    void NAME##__reset(void) {\
        NAME##__length = 0;\
    }\
    \
    char* NAME##__nextString(void);\
    char* NAME##__nextString(void) {\
        return NAME##__buffer + NAME##__length;\
    }\

#define CREATE_PRINTF_LIKE_FUNCTION_SIGNATURES(NAME)\
    extern char NAME##__buffer[];\
    extern size_t NAME##__length;\
    extern char* NAME##__lastString;\
    extern size_t NAME##__lastStringLength;\
    int NAME(const char* format, ...);\
    void NAME##__reset(void);\
    char* NAME##__nextString(void);\


#define ADD_PRINTF_MOCK(BUFFER_LENGTH) CREATE_PRINTF_LIKE_FUNCTION(printf, BUFFER_LENGTH);


#if defined(WITH_BEFORE_EACH)
    #define BEFORE_EACH(LABEL) void LABEL##__beforeEach(void); void LABEL##__beforeEach(void)
    #define __ADD_BEFORE_EACH(LABEL) .beforeEach = LABEL##__beforeEach,
#else 
    #define __ADD_BEFORE_EACH(LABEL) .beforeEach = NULL,
#endif // WITH_BEFORE_EACH

#if defined(WITH_AFTER_EACH)
    #define AFTER_EACH(LABEL) void LABEL##__afterEach(void); void LABEL##__afterEach(void)
    #define __ADD_AFTER_EACH(LABEL) .afterEach = LABEL##__afterEach,
#else
    #define __ADD_AFTER_EACH(LABEL) .afterEach = NULL,
#endif // WITH_AFTER_EACH

#if defined(MULTI_TEST)
    #define DESCRIBE(LABEL) \
        void LABEL##__runTests(struct TESTS_infoType* TESTS_info);\
        int LABEL(void) {\
            TESTS_STD_PRINT("\x1B[1;33mDescribing '"#LABEL"'\x1B[0m\n");\
            struct TESTS_infoType info = {\
                .status = 0,\
                __ADD_BEFORE_EACH(LABEL)\
                __ADD_AFTER_EACH(LABEL)\
            };\
            LABEL##__runTests(&info);\
            if(info.status < 0) {\
                TESTS_STD_PRINT("\x1B[1;31mFailed:\n");\
                TESTS_STD_PRINT("    At %s:%d\n", __FILE__, info.failLine);\
                TESTS_STD_PRINT("    EXPECT(%s)%s%s", info.testText, info.isNot ? " NOT " : " ", info.operatorText);\
                if(info.expectText != NULL) {\
                    TESTS_STD_PRINT("(%s)", info.expectText);\
                }\
                TESTS_STD_PRINT("\x1B[0m\n");\
\
                return 1;\
            }\
    \
            return 0;\
        }\
        void LABEL##__runTests(struct TESTS_infoType* TESTS_info)
    
    #define RUN_TESTS(...)\
        typedef int TestSuit(void);\
        int main(void) {\
            TestSuit* suits[] = { __VA_ARGS__ };\
            int status = 0;\
            TESTS_STD_PRINT("\n\n");\
            for(size_t i = 0; i < sizeof(suits) / sizeof(suits[0]); i++) {\
                if(suits[i]() == 1) status = 1;\
                TESTS_STD_PRINT("\n");\
            }\
            TESTS_STD_PRINT("\n\n");\
            return status;\
        }
#else
    #define DESCRIBE(LABEL) \
        void runTests(struct TESTS_infoType* TESTS_info);\
        int main(void) {\
            TESTS_STD_PRINT("\n\n\x1B[1;33mDescribing '"#LABEL"'\x1B[0m\n");\
            struct TESTS_infoType info = {\
                .status = 0,\
                __ADD_BEFORE_EACH(LABEL)\
                __ADD_AFTER_EACH(LABEL)\
            };\
            runTests(&info);\
            if(info.status < 0) {\
                TESTS_STD_PRINT("\x1B[1;31mFailed:\n");\
                TESTS_STD_PRINT("    At %s:%d\n", __FILE__, info.failLine);\
                TESTS_STD_PRINT("    EXPECT(%s)%s%s", info.testText, info.isNot ? " NOT " : " ", info.operatorText);\
                if(info.expectText != NULL) {\
                    TESTS_STD_PRINT("(%s)", info.expectText);\
                }\
                TESTS_STD_PRINT("\x1B[0m\n\n\n");\
                return 1;\
            }\
    \
            TESTS_STD_PRINT("\n\n");\
            return 0;\
        }\
        void runTests(struct TESTS_infoType* TESTS_info)
#endif

#define IT(LABEL)\
    if(TESTS_info->beforeEach != NULL) {\
        TESTS_info->beforeEach();\
    }\
    TESTS_info->testLabel = LABEL;\
    TESTS_STD_PRINT("\x1B[1mit "LABEL"\x1B[0m\n");\
    for(int i = 0; i < 1; i += TESTS_info->afterEach ? (TESTS_info->afterEach(), 1) : 1)

#define IT_TODO(LABEL)\
    TESTS_info->status = 0;\
    TESTS_STD_PRINT("\x1B[1;35mTODO: it "LABEL"\x1B[0m\n");

#define MATCHER_VALUE localPassedValue
#define EXPECT(VALUE)\
    {\
        __typeof__((VALUE)) MATCHER_VALUE = VALUE;\
        TESTS_info->failLine = __LINE__;\
        TESTS_info->testText = #VALUE;\
        TESTS_info->isNot = 0;\

#define NOT\
    TESTS_info->isNot = 1;\


// Mather utils
#define CREATE_MATCHER(MATCHER_NAME, ...)\
    TESTS_info->operatorText = #MATCHER_NAME;\
    __VA_ARGS__;\
    };\

#define EXPECTED(string) string
#define NO_EXPECTED NULL
#define MATCHER_FAIL(EXPECTED_TEXT)\
    TESTS_info->status = -1;\
    TESTS_info->expectText = EXPECTED_TEXT;\
    return;\

#define MATHER_IS_NOT TESTS_info->isNot

#define PASSES_IF(CONDITION) !(CONDITION)
#define FAILS_IF(CONDITION) (CONDITION)

#define MATCHER_CONDITION(CONDITION) if((CONDITION) ^ MATHER_IS_NOT)
#define MATCHER_CONDITION_S(CONDITION, EXPECTED_TEXT)\
    MATCHER_CONDITION(CONDITION) MATCHER_FAIL(EXPECTED_TEXT);\

#define CREATE_MATCHER_S(MATCHER_NAME, CONDITION, EXPECTED_TEXT)\
    CREATE_MATCHER(MATCHER_NAME,\
        MATCHER_CONDITION_S(CONDITION, EXPECTED_TEXT)\
    )


// MATCHERS
#define TO_BE(VALUE) CREATE_MATCHER_S(TO_BE, PASSES_IF(MATCHER_VALUE == (VALUE)), EXPECTED(#VALUE))

#define TO_BE_TRUTHY CREATE_MATCHER_S(TO_BE_TRUTHY, FAILS_IF(!MATCHER_VALUE), NO_EXPECTED)

#define TO_BE_FALSY CREATE_MATCHER_S(TO_BE_FALSY, PASSES_IF(!MATCHER_VALUE), NO_EXPECTED)

#define TO_BE_STRING(EXPECTED_STRING)\
    CREATE_MATCHER_S(\
        TO_BE_STRING,\
        PASSES_IF(TESTS_STD_STRCMP(MATCHER_VALUE, EXPECTED_STRING) == 0),\
        EXPECTED(#EXPECTED_STRING)\
    )

#define TO_BE_NULL CREATE_MATCHER_S(TO_BE_NULL, PASSES_IF(MATCHER_VALUE == NULL), NO_EXPECTED)

#define TO_HAVE_BYTES(BYTES, LENGTH)\
    CREATE_MATCHER_S(\
        TO_HAVE_BYTES, \
        PASSES_IF(TESTS_STD_MEMCMP(MATCHER_VALUE, BYTES, LENGTH) == 0),\
        EXPECTED(#BYTES", "#LENGTH)\
    )

#define TO_HAVE_RAW_BYTES(BYTE, ...)\
    CREATE_MATCHER(TO_HAVE_RAW_BYTES,\
        unsigned char buffer[] = { BYTE, __VA_ARGS__ };\
        for(size_t i = 0; i < sizeof(buffer); i++) {\
            MATCHER_CONDITION(PASSES_IF(buffer[i] == MATCHER_VALUE[i])) {\
                printf("Got bytes mismatch at position %zu. Expected: %d, Got: %d\n", i, buffer[i], MATCHER_VALUE[i]);\
                MATCHER_FAIL("{ "#BYTE", "#__VA_ARGS__" }")\
            }\
        }\
    )
#endif // TESTS_H
