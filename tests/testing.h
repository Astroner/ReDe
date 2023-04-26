#if !defined(TESTING_H)
#define TESTING_H

#define TEST(function)\
    function();\
    printf("%s - PASS", #function);\


#endif // TESTING_H
