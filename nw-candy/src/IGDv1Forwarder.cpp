// NetworkCandy
// Network system notifications and uPnP shenanigans
// Copyright (C) 2020-2021 Guillaume Vara

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

IGDv1Forwarder::IGDv1Forwarder(const std::string& port, const std::string& PROTOCOL, const char * controlURL, const char * servicetype, const char * description) : 
    uPnPForwarderImpl(port, PROTOCOL, controlURL, servicetype), _description(description) { }

IGDv1Forwarder::~IGDv1Forwarder() {}

int IGDv1Forwarder::portforwardExists(bool* isForwarded) {
    // getter args
    char intClient[16];
    char intPort[6];
    char duration[16];

    // request
    auto result = UPNP_GetSpecificPortMappingEntry(
        _controlURL,
        _servicetype,
        _portToForward.c_str(),
        _protocol.c_str(),
        "*" /*remoteHost*/,
        intClient,
        intPort,
        NULL /*desc*/,
        NULL /*enabled*/,
        duration
    );

    // no redirect acked
    if(result == 714) {
        spdlog::info("UPNP CheckRedirect : GetSpecificPortMappingEntry() found no existing entry");
        *isForwarded = false;
        return 0;
    }

    // if any code
    if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::warn("UPNP CheckRedirect : GetSpecificPortMappingEntry() failed with code {} ({})", result, strupnperror(result));
        return result;
    }

    // else, has redirect
    spdlog::info("UPNP CheckRedirect : {}[{}] is redirected to internal {} : {} (duration={})",
        _portToForward, _protocol, intClient, intPort, duration
    );
    *isForwarded = true;
    return 0;
}

int IGDv1Forwarder::portforward(bool* isForwarded, const char* localIp, const char* leaseTime) {
    auto result = UPNP_AddPortMapping(
        _controlURL,
        _servicetype,
        _portToForward.c_str(),
        _portToForward.c_str(),
        localIp,
        _description,
        _protocol.c_str(),
        NULL /*remoteHost*/,
        leaseTime
    );

    // Action failed, most possibly on already existing mapping
    if (result == 501) {
        spdlog::warn("UPNP AskRedirect : AddPortMapping() failed on 501 error, but considering that mapping already exist");
        *isForwarded = true;
        return 0;
    }

    // check if error
    if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::warn("UPNP AskRedirect : AddPortMapping({},{}, {}) failed with code {} ({})",
            _portToForward, _portToForward, localIp, result, strupnperror(result)
        );
        return result;
    }

    // success !
    *isForwarded = true;
    spdlog::info("UPNP AskRedirect : Redirection OK !");
    return 0;
}

int IGDv1Forwarder::removePortforward(bool* isForwarded) {
    // request
    auto result = UPNP_DeletePortMapping(
        _controlURL,
        _servicetype,
        _portToForward.c_str(),
        _protocol.c_str(),
        NULL /*remoteHost*/
    );

    // check error
    if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::warn("UPNP RemoveRedirect : UPNP_DeletePortMapping() failed with code :{}", result);
        return result;
    }

    // success
    spdlog::info("UPNP RemoveRedirect : UPNP_DeletePortMapping() succeeded !");
    *isForwarded = false;

    return 0;
}