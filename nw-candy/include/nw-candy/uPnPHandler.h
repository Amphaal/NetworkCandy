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

#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
#endif

#include <miniupnpc/miniupnpc.h>

#include <string>

#include "uPnPForwarder.h"

namespace NetworkCandy {

class uPnPHandler {
 public:
    uPnPHandler(const std::string &portToMap, const std::string &serviceDescription);

    bool ensurePortMapping();  // returns if port mapping is set
    void mayDeletePortMapping();
    ~uPnPHandler();

    const std::string externalIP() const;
    const std::string localIP() const;

 protected:
    static inline const std::string PROTOCOL = "TCP";
    const std::string& portToMap() const;

 private:
    static constexpr unsigned char _TTL = 2; /* defaulting to 2 */
    static constexpr int _LOCALPORT = UPNP_LOCAL_PORT_ANY;
    static constexpr int _DISCOVER_DELAY_MS = 2000;
    static inline const char * _LEASE_DURATION = "0";  // infinite lease

    uPnPForwarderImpl* _createAppropriateIGDImplementation();
    uPnPForwarderImpl* _impl = nullptr;

    #ifdef _WIN32
        WSADATA _wsaData;
        const WORD _requestedVersion = MAKEWORD(2, 2);
    #endif

    UPNPUrls _urls;
    IGDdatas _IGDData;
    bool _IGDFound = false;
    UPNPDev* _devicesList = nullptr;
    
    bool _hasRedirect = false;

    char _localIPAddress[64] = "unset"; /* my ip address on the LAN */
    char _externalIPAddress[40] = "unset"; /* my ip address on the WAN */

    const std::string _description;
    const std::string _targetPort;

    // returns error code if any
    int _discoverDevicesIPv4();
    int _discoverDevicesIPv6();
    int _discoverDevices(bool useIpV6, const char * protocolDescr);

    // returns if succeeded
    bool _getExternalIP();

    // returns if succeeded
    bool _getValidIGD();

    // returns if succeeded
    bool _initUPnP();

    bool _isIGDv2(const char * serviceType);
};

}  // namespace NetworkCandy
