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

IDGv2Forwarder::~IDGv2Forwarder() {}

int IDGv2Forwarder::portforwardExists(bool* isForwarded) {
    // always go for pinholing
    *isForwarded = false;
    return 0;
}

int IDGv2Forwarder::portforward(bool* isForwarded, const char* localIp, const char* leaseTime) {    
    auto result = UPNP_AddPinhole(
        _controlURL, 
        _servicetype, 
        "*",
        _portToForward.c_str(),
        localIp,
        _portToForward.c_str(),
        _protocol.c_str(),
        leaseTime,
        _wp_id
    );

    // if firewall is disabled, portforwarding is not
    if (result == 702) {
        spdlog::warn("UPNP AskRedirect : UPNP_AddPinhole() failed on 702 error : since firewall is not active, no problem !");
        *isForwarded = true;
        return 0;
    }

    // check if error
    else if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::info(
            "UPNP AskRedirect : UPNP_AddPinhole({}, {}) failed with code {} ({})",
            _portToForward, localIp, result, strupnperror(result)
        );
        return result;
    }

    // success !
    *isForwarded = true;
    return 0;
}

int IDGv2Forwarder::removePortforward(bool* isForwarded) {
    //
    if(_wp_id[0] == '\0') {
        spdlog::info("UPNP RemoveRedirect : UPNP_DeletePinhole() cannot be called since no pinhole ID is registered !");
        return -999;
    } 

    //
    auto result = UPNP_DeletePinhole(
        _controlURL,
        _servicetype,
        _wp_id
    );

    // check error
    if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::info("UPNP RemoveRedirect : UPNP_DeletePinhole() failed with code :{}", result);
        return result;
    }

    // success
    spdlog::info("UPNP RemoveRedirect : UPNP_DeletePinhole() succeeded !");
    *isForwarded = false;

    return 0;
}