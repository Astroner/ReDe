const fs = require("fs");
const path = require("path");

console.log();
console.log("Generating STD HashTable");

const content = fs.readFileSync(path.resolve(__dirname, "core", "src", "RedeStd.c"));

/**
 * 
 * @param {String} str 
 * @returns {BigInt}
 */
const hash = (str) => {
    let hash = BigInt(5381);
    
    for(let ch of str) {
        hash = ((hash << BigInt(5)) + hash) + BigInt(ch.charCodeAt(0)); /* hash * 33 + c */
    }

    return hash;
}

const functions = Array.from(content.toString().matchAll(/int Rede_std_(\w+)\(const/g)).map(item => item[1]);

let size = BigInt(functions.length);

const indexToName = new Map();

while(true) {
    let success = 1;
    for(let funcName of functions) {
        const funcIndex = hash(funcName) % size;
        if(indexToName.has(funcIndex)) {
            indexToName.clear();
            size++;
            success = false;
            break;
        }
        indexToName.set(funcIndex, funcName);
    }
    if(success) break;
}

console.log(`HashTable size: ${size}`);
console.log(`Indexes:`);
console.log(indexToName);
console.log();

const tableItems = Array.from(indexToName.entries()).map(([index, name]) => ({ name, index }));

const result = `#include "RedeStdTable.h"

${tableItems.map(item => `int Rede_std_${item.name}(const RedeFunctionArgs* args, RedeVariable* result);`).join("\n")}

size_t Rede_std_functions_size = ${size};

RedeStdFunction Rede_std_functions[${size}] = {
    ${tableItems.map(item => `[${item.index}] = { "${item.name}", Rede_std_${item.name} }`).join(",\n    ")}
};
`

fs.writeFileSync(path.resolve(__dirname, "./core/RedeStdTable.gen.c"), result);