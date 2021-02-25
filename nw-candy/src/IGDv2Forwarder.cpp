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

IGDv2Forwarder::IGDv2Forwarder(const std::string& port, const std::string& PROTOCOL, const char * controlURL, const char * servicetype) : 
    uPnPForwarderImpl(port, PROTOCOL, controlURL, servicetype) { }

IGDv2Forwarder::~IGDv2Forwarder() {}

int IGDv2Forwarder::portforwardExists(bool* isForwarded) {
    int firewallEnabled = 0, pinholingAllowed = 0;
    auto result = UPNP_GetFirewallStatus(_controlURL, _servicetype, &firewallEnabled, &pinholingAllowed);

    if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::warn("UPNP CheckRedirect : GetFirewallStatus() failed with code {} ({})", result, strupnperror(result));
        return result;
    }

    // if firewall is not enabled, no need to pinhole !
    if(!firewallEnabled) {
        spdlog::info("UPNP CheckRedirect : Firewall is disabled, no need to pinhole !");
        *isForwarded = true;
        return 0;
    } else if(!pinholingAllowed) {
        spdlog::warn("UPNP CheckRedirect : Firewall is active, and pinholing is not allowed !");
        return -123;
    }

    spdlog::info("UPNP CheckRedirect : Firewall active and allowing pinholing. Continuing...");
    
    // always go for pinholing
    *isForwarded = false;
    return 0;
}

int IGDv2Forwarder::portforward(bool* isForwarded, const char* localIp, const char* leaseTime) {    
    auto result = UPNP_AddPinhole(
        _controlURL, 
        _servicetype, 
        "*",
        _portToForward.c_str(),
        localIp,
        _portToForward.c_str(),
        _protocol.c_str(), // TODO forcing wildcard ?!
        leaseTime,
        _wp_id
    );

    // if firewall is disabled, portforwarding is not
    if (result == 702) {
        spdlog::info("UPNP AskRedirect : UPNP_AddPinhole() failed on 702 error : since firewall is not active, no problem !");
        *isForwarded = true;
        return 0;
    }

    // check if error
    else if (result != UPNPCOMMAND_SUCCESS) {
        spdlog::warn("UPNP AskRedirect : UPNP_AddPinhole({}, {}) failed with code {} ({})",
            _portToForward, localIp, result, strupnperror(result)
        );
        return result;
    }

    // success !
    *isForwarded = true;
    spdlog::info("UPNP AskRedirect : Redirection OK !");
    return 0;
}

int IGDv2Forwarder::removePortforward(bool* isForwarded) {
    //
    if(_wp_id[0] == '\0') {
        spdlog::warn("UPNP RemoveRedirect : UPNP_DeletePinhole() cannot be called since no pinhole ID is registered !");
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
        spdlog::warn("UPNP RemoveRedirect : UPNP_DeletePinhole() failed with code :{}", result);
        return result;
    }

    // success
    spdlog::info("UPNP RemoveRedirect : UPNP_DeletePinhole() succeeded !");
    *isForwarded = false;

    return 0;
}