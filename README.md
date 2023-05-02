# Hi there!
This is a simple scripting language ReDe

# Table of content
 - [Syntax](#syntax)
     - [Variables](#variables)
     - [Functions](#functions)


# Syntax
Rede simply consists of 4 things:
 - variables
 - numbers
 - strings
 - booleans
 - functions

## Variables
To define a variable just write its name and assign a value to it:
```ts
numberVariable = 33
stringVariable = "Hi!"
anotherStringVariable = 'Hello!'
truthy = true
falsy = false
```

## Functions
In ReDe you cant define your own functions, you can only use provided functions from the environment, but function calls themselves are easy:
```ts
sum = sum(1 2)
log(sum)
```
As you see, the function arguments are separated by any whitespace in any numbers. For example, the code below is also valid code:
```ts
log(   2222    3     
4  5
)
```