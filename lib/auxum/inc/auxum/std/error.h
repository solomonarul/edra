#pragma once
#ifndef AUXUM_ERROR_H
#define AUXUM_ERROR_H

#define DEFINE_RESULT(ResultType, ErrorType, Name) typedef struct { bool ok : 1; union { ResultType result; ErrorType error; }; } Name
#define DEFINE_MAYBE(ErrorType, Name) typedef struct { bool ok : 1; ErrorType error; } Name
#define IS_OK(Result) ((Result).ok)
#define RESULT_GET_VALUE(Result) ((Result).result)
#define RESULT_GET_ERROR(Result) ((Result).error)
#define RESULT_GET_OR(Result, Other) (IS_OK(Result) ? RESULT_GET_VALUE(Result) : (Other));

DEFINE_MAYBE(char*, maybe_t);

#endif
