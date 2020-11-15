// NetworkCandy
// Network system notifications and uPnP shenanigans
// Copyright (C) 2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include <nw-candy/uPnPHandler.h>
#include <nw-candy/ConnectivityManager.h>

void _uPnPTest() {
    NetworkCandy::uPnPHandler uPnPHandler("31137", "RPGRPZ");
    uPnPHandler.ensurePortMapping();
    uPnPHandler.mayDeletePortMapping();
}

int main() {
    NetworkCandy::ConnectivityManager cm;
    cm.startListening();
    return 0;
}
