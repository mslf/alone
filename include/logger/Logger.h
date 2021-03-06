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
 * @file Logger.h
 * @author mslf
 * @date 11 Oct 2016
 * @brief File containing #Logger and its stuff.
 */
#ifndef ALONE_LOGGER_H
#define ALONE_LOGGER_H

#include <stdbool.h>

/**
 * @brief Possible #Logger states.
 */
enum LoggerState {
    LOGGER_DISABLED,
    /**< #Logger is disabled, so no logs provided. */
    LOGGER_ENABLED_STDERR,
    /**< #Logger is enabled and all logs will be at stderr. */
    LOGGER_ENABLED_FILE
    /**< #Logger is enabled and alllogs will be saved to file. */
};

/**
 * @brief Logging subsystem.
 * @note Logging to file is not implemented yet.
 * @note Logger_saveUsedFlagAndSetToFalse() saves only one level of nesting.
 * // FIXME Consider using stack?
 */
struct Logger {
    enum LoggerState state;
    /**< Current #Logger state. */
    bool wasUsed;
    /**< Sets to true, when some logs was wrote. */
    bool lastWasUsed;
    /**< State of Logger#wasUsed before calling Logger_saveUsedFlagAndSetToFalse(). */
};

/**
 * @brief Function for logging formatted string. End line char will be added at the end of the string.
 * @param logger Pointer to a #Logger. Can be NULL.
 * @param format String with a log message format string. Can be NULL.
 * @note ... You can pass other parameters, like in printf and other such functions.
 */
void Logger_log(struct Logger* logger, const char* const format, ...);

/**
 * @brief Saves Logger#wasUsed to Logger#lastWasUsed and sets Logger#wasUsed to false.
 * @param logger Pointer to a #Logger, where flags will be set.
 */
void Logger_saveUsedFlagAndSetToFalse(struct Logger* logger);

/**
 * @brief Sets Logger#wasUsed to Logger#lastWasUsed.
 * @param logger Pointer to a #Logger, where flag will be set.
 */
void Logger_revertUsedFlag(struct Logger* logger);
#endif //ALONE_LOGGER_H
