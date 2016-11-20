/*
	Copyright 2016 Golikov Vitaliy

	This file is part of Alone.

	Alone is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Alone is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Alone. If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file Logger.c
 * @author mslf
 * @date 11 Oct 2016
 * @brief File containing implementation of #Button.
 */
#include <stdio.h>
#include <stdarg.h>
#include "logger/Logger.h"

void Logger_log(struct Logger* logger, const char* const format, ...) {
    if (logger && format && logger->state == LOGGER_ENABLED_STDERR) {
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stderr, format, argptr);
        va_end(argptr);
        fprintf(stderr, "\n");
    }
}
