//
// Created by mslf on 8/23/16.
//
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
#ifndef ALONE_NANOSECTION_H
#define ALONE_NANOSECTION_H

enum NanoSectionType {
    Command,
    IntegerData,
    DoubleData,
    StringData
};

struct NanoSection {
    enum NanoSectionType type;
    char* sectionName;
    char* valueString;
};

struct NanoSection* NanoSection_construct(enum NanoSectionType nanoSectionType, const char* const name);
void NanoSection_destruct(struct NanoSection* nanoSection);

void NanoSection_setValue(struct NanoSection* nanoSection, const char* const value);

#endif //ALONE_NANOSECTION_H
