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

#include "uPnPForwarder.h"

#include <spdlog/spdlog.h>

#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

IDGv2Forwarder::IDGv2Forwarder(const std::string& port, const std::string& PROTOCOL, const char * controlURL, const char * servicetype) : 
    uPnPForwarderImpl(port, PROTOCOL, controlURL, servicetype) { }

int IDGv2Forwarder::portforwardExists(bool* isForwarded) {   
    throw std::logic_error("Unimplemented !");
}

int IDGv2Forwarder::portforward(bool* isForwarded, const char* localIp, const char* leaseTime) { 
    throw std::logic_error("Unimplemented !");
}

int IDGv2Forwarder::removePortforward(bool* isForwarded) {
    throw std::logic_error("Unimplemented !");
}