#define REDE_RUNTIME_IMPLEMENTATION
#define REDE_STD_IMPLEMENTATION
#include "../RedeSTD.h"

#include <assert.h>

#include "testing.h"

void sumsNumbers() {
    char name[] = "sum";
    
    RedeVariable result;
    
    RedeVariable args[] = {
        {
            .type = RedeVariableTypeNumber,
            .data = {
                .number = 2.00f
            }
        },
        {
            .type = RedeVariableTypeNumber,
            .data = {
                .number = 3.00f
            }
        },
    };

    RedeFunctionArgs argsArray = {
        .values = args,
        .length = 2,
    };
    
    Rede_std(name, sizeof(name), &argsArray, &result, NULL);

    assert(result.type == RedeVariableTypeNumber);
    assert(result.data.number == 5.00f);
}

int main(void) {
    printf("\nSTD tests:\n");

    TEST(sumsNumbers);

    printf("\n");
    return 0;
}