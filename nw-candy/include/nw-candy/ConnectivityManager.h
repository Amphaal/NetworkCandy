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

#include <windows.h>
#include <netlistmgr.h>
#include <ocidl.h>

namespace NetworkCandy {

class CMEventHandler : public INetworkListManagerEvents {
 public:
    CMEventHandler();
    virtual ~CMEventHandler();
    STDMETHODIMP QueryInterface (REFIID refIID, void** pIFace);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP ConnectivityChanged(NLM_CONNECTIVITY NewConnectivity);

 protected:
    virtual void _connectivityChanged(bool isConnectedToInternet) = 0;

 private:
    bool _bInternet = false;
    ULONG m_lRefCnt;
    DWORD m_dwCookie;
};

class ConnectivityManager : private CMEventHandler {
 public:
    ConnectivityManager();
    ~ConnectivityManager();

    bool isConnectedToInternet();

    void startListening();

 protected:
    void _connectivityChanged(bool isConnectedToInternet) override;

 private:
    INetworkListManager* _manager = nullptr;
    IConnectionPointContainer* _managerCPC = nullptr;
    IConnectionPoint* _cp = nullptr;
    DWORD _cookie;
};

}  // namespace NetworkCandy
