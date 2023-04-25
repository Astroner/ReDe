#if !defined(TESTING_H)
#define TESTING_H

#define TEST(function)\
    printf("%s - ", #function);\
    function();\
    printf("PASS\n");\


#endif // TESTING_H
