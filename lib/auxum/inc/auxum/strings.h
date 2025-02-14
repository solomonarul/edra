#pragma once
#ifndef AUXUM_STRINGS_H
#define AUXUM_STRINGS_H

/*
 *  NOTE: make sure that the result fits into dest, this is unsafe.
 */
void string_concat(char* const dest, char* const first, char* const second);

#endif
