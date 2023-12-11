#include "tests-new.h"

#include "tests.h"

#include "../RedeSTD.h"

DESCRIBE(std) {
    IT("sums number") {
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

        EXPECT(result.type) TO_BE(RedeVariableTypeNumber);
        EXPECT(result.data.number) TO_BE(5.00f);
    }

    IT("compares correctly") {
        char name[] = "eq";

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
                    .number = 2.00f
                }
            },
        };

        RedeFunctionArgs argsArray = {
            .values = args,
            .length = 2,
        };

        Rede_std(name, sizeof(name), &argsArray, &result, NULL);

        EXPECT(result.type) TO_BE(RedeVariableTypeBoolean);
        EXPECT(result.data.boolean) TO_BE(1);
    }
}