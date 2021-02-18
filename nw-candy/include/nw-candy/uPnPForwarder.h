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

#include <string>

class uPnPForwarderImpl {
 public:
    uPnPForwarderImpl(const std::string& port, const std::string& PROTOCOL, const char * controlURL, const char * servicetype);

    // returns error code if any
    virtual int portforwardExists(bool* isForwarded) = 0;
    
    // returns error code if any, defaults leaseTime to 12 hours
    virtual int portforward(bool* isForwarded, const char* localIp, const char* leaseTime = "43200") = 0;

    // returns error code if any
    virtual int removePortforward(bool* isForwarded) = 0;

 protected:
    const std::string _portToForward;
    const std::string _protocol;
    const char * _controlURL;
    const char * _servicetype;
};

class IDGv1Forwarder : public uPnPForwarderImpl {
 public:
    IDGv1Forwarder(const std::string& port, const std::string& PROTOCOL, const char * controlURL, const char * servicetype, const char * description);

    int portforwardExists(bool* isForwarded) final;
    int portforward(bool* isForwarded, const char* localIp, const char* leaseTime) final;
    int removePortforward(bool* isForwarded) final;
 
 private:
    const char * _description;
};

class IDGv2Forwarder : public uPnPForwarderImpl {
 public:
    IDGv2Forwarder(const std::string& port, const std::string& PROTOCOL, const char * controlURL, const char * servicetype);

    int portforwardExists(bool* isForwarded) final;
    int portforward(bool* isForwarded, const char* localIp, const char* leaseTime) final;
    int removePortforward(bool* isForwarded) final;
 
 private:
    char _wp_id[16] = "\0";
};