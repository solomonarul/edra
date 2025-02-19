#pragma once
#ifndef AUXUM_STRINGS_H
#define AUXUM_STRINGS_H

/*
 *  Concatenates two strings and stores the result into dest.
 *  NOTE: make sure that the result actually fits into dest as this is unsafe.
 */
void string_concat(char* const dest, char* const first, char* const second);

/*
 *  Strips whitespaces from before and after the string.
 *  This returns the pointer to the string after the operations. Please note that this doesn't reallocate memory, it just moves pointers around.
 */
char* string_strip(char* const src);

#endif
