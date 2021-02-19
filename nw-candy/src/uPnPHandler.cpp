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

#include "uPnPHandler.h"

#include <spdlog/spdlog.h>

#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

NetworkCandy::uPnPHandler::uPnPHandler(const std::string &portToMap, const std::string &serviceDescription) :
    _targetPort(portToMap), _description(serviceDescription) {}

// returns if port mapping is set
bool NetworkCandy::uPnPHandler::ensurePortMapping() {
    //
    spdlog::info("UPNP run : Starting uPnP port mapping on port {} for [{}] ...", _targetPort, _description);

    //
    try {
        // init uPnP...
        auto initOK = this->_initUPnP();
        if (!initOK) return false;

        // use appropriate implementation
        if(!_impl) 
            _impl = _createAppropriateIGDImplementation();
        
        // check if has redirection already done
        auto errCode = _impl->portforwardExists(&_hasRedirect);
        if (errCode) return false;
        if (_hasRedirect) return true;

        // no redirection set, try to ask for one
        auto redirectCode = _impl->portforward(&_hasRedirect, _localIPAddress);
        if (redirectCode) return false;

    } catch(...) {
        // log on exception
        spdlog::warn("UPNP run : exception caught while processing");
    }
    
    return _hasRedirect;
}

uPnPForwarderImpl* NetworkCandy::uPnPHandler::_createAppropriateIGDImplementation() {
    // assume hole punching is available ?
    auto &FC_st = _IGDData.IPv6FC.servicetype;
    if(FC_st[0] != '\0') {
        spdlog::info("UPNP run : FirewallControl service existing, trying IDGv2 implementation.");
        return new IDGv2Forwarder(
            _targetPort,
            PROTOCOL,
            _urls.controlURL_6FC,
            FC_st
        );
    }

    // use default impl
    spdlog::info("UPNP run : no FirewallControl service found, using IDGv1 implementation.");
    return new IDGv1Forwarder(
        _targetPort,
        PROTOCOL,
        _urls.controlURL,
        _IGDData.first.servicetype,
        _description.c_str()
    );
}

void NetworkCandy::uPnPHandler::mayDeletePortMapping() {
    if (_hasRedirect && _impl) {
        this->_impl->removePortforward(&_hasRedirect);
    }  
}

NetworkCandy::uPnPHandler::~uPnPHandler() {
    /*free*/
    if(_IGDFound) FreeUPNPUrls(&_urls);
    if(_devicesList) freeUPNPDevlist(_devicesList);

    /*End websock*/
    #ifdef _WIN32
        WSACleanup();
    #endif

    //
    if(_impl) delete _impl;
}

const std::string NetworkCandy::uPnPHandler::externalIP() const {
    return _externalIPAddress;
}

const std::string NetworkCandy::uPnPHandler::localIP() const {
    return _localIPAddress;
}

const std::string& NetworkCandy::uPnPHandler::portToMap() const {
    return _targetPort;
}

int NetworkCandy::uPnPHandler::_discoverDevicesIPv4() {
    return _discoverDevices(false, "with IPv4");
}

int NetworkCandy::uPnPHandler::_discoverDevicesIPv6() {
    return _discoverDevices(true, "with IPv6");
}

// returns error code if any
int NetworkCandy::uPnPHandler::_discoverDevices(bool useIpV6, const char * protocolDescr) {
    // not used
    char* _multicastif = nullptr;
    char* _minissdpdpath = nullptr;

    // discover
    spdlog::info("UPNP Inst : starting discovery {}...", protocolDescr);
    int error;
    _devicesList = upnpDiscover(
        _DISCOVER_DELAY_MS,
        _multicastif,
        _minissdpdpath,
        _LOCALPORT,
        useIpV6,
        _TTL,
        &error
    );

    // if error
    if(error) {
        spdlog::info("UPNP Inst : upnpDiscover() {} error code= {}", protocolDescr, error);
        return error;
    }

    // if not devices found, most probably a timeout
    if(!_devicesList) {
        spdlog::info("UPNP Inst : upnpDiscover() {} has most probably timed out, no devices found !", protocolDescr);
        return -998;
    }

    // iterate through devices discovered
    UPNPDev* device;
    spdlog::info("UPNP Inst : List of {} UPNP devices found on the network :", protocolDescr);
    for (device = _devicesList; device; device = device->pNext) {
        // log each
        spdlog::info("UPNP Inst : -> desc: {} st: {}", device->descURL, device->st);
    }

    // succeeded !
    return 0;
}

// returns if succeeded
bool NetworkCandy::uPnPHandler::_getExternalIP() {
    // request
    int r = UPNP_GetExternalIPAddress(
        _urls.controlURL,
        _IGDData.first.servicetype,
        _externalIPAddress
    );

    // if failed
    if (r != UPNPCOMMAND_SUCCESS) {
        spdlog::info("UPNP GetExternalIPAddress : No IGD UPnP Device");
        return false;
    }

    // succeeded !
    spdlog::info("UPNP GetExternalIPAddress : ext. IP address = {}", _externalIPAddress);
    return true;
}

// returns if succeeded
bool NetworkCandy::uPnPHandler::_getValidIGD() {
    // request
    spdlog::info("UPNP Inst : Fetching UPNP Internet Gateway Devices...");
    auto result = UPNP_GetValidIGD(
        _devicesList, 
        &_urls, 
        &_IGDData, 
        _localIPAddress, 
        sizeof(_localIPAddress)
    );

    // handle returns
    switch (result) {
        case 0: {
            spdlog::info("UPNP Inst : No valid UPNP Internet Gateway Device found.");
            return false;
        }
        break;
        case 1:
            spdlog::info("UPNP Inst : Found valid IGD : {}", _urls.controlURL);
            break;
        case 2:
            spdlog::info("UPNP Inst : Found a (not connected?) IGD : {}", _urls.controlURL);
            spdlog::info("UPNP Inst : Trying to continue anyway");
            break;
        case 3:
            spdlog::info("UPNP Inst : UPnP device found. Is it an IGD ? : {}", _urls.controlURL);
            spdlog::info("UPNP Inst : Trying to continue anyway");
            break;
        default:
            spdlog::info("UPNP Inst : Found device (igd ?) : {}", _urls.controlURL);
            spdlog::info("UPNP Inst : Trying to continue anyway");
            break;
    }

    //
    spdlog::info("UPNP Inst : Local LAN ip address {}", _localIPAddress);

    // succeeded !
    _IGDFound = true;
    return true;
}

// returns if succeeded
bool NetworkCandy::uPnPHandler::_initUPnP() {
    /* start websock if Windows platform */
    #ifdef _WIN32
        auto nResult = WSAStartup(_requestedVersion, &_wsaData);
        if (nResult != NO_ERROR) {
            spdlog::info("UPNP Inst : Cannot init socket with WSAStartup !");
            return false;
        }
    #endif

    /* discover devices from IPv4 */
    if (_discoverDevicesIPv6() != 0) {
        /* discover devices IPv6 */
        if(_discoverDevicesIPv4() != 0) {
            // fails !
            spdlog::info("UPNP Inst : No IGD UPnP Device found on the network !");
            return false;
        }
    }

    /* get IGD */
    if(!_getValidIGD()) {
        return false;
    }

    /* get external IP */
    if(!_getExternalIP()) {
        return false;
    }

    // succeeded !
    return true;
}
