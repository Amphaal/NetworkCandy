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

#include "ConnectivityManager.h"
#include <spdlog/spdlog.h>

#include <stdexcept>

NetworkCandy::CMEventHandler::CMEventHandler() {}
NetworkCandy::CMEventHandler::~CMEventHandler() {}
STDMETHODIMP NetworkCandy::CMEventHandler::QueryInterface(REFIID refIID, void** pIFace) {
    *pIFace = NULL;
    if(refIID == IID_IUnknown || refIID == __uuidof(INetworkListManagerEvents)) {
        *pIFace =  (IUnknown*)(INetworkListManagerEvents*)(this);
    }
    if (*pIFace == NULL) {
        return E_NOINTERFACE;
    }
    ((IUnknown*)*pIFace)->AddRef();

    return S_OK;
}
STDMETHODIMP_(ULONG) NetworkCandy::CMEventHandler::AddRef() {
    m_lRefCnt++;
    return m_lRefCnt;
}
STDMETHODIMP_(ULONG) NetworkCandy::CMEventHandler::Release() {
    m_lRefCnt--;
    if(m_lRefCnt == 0) {
        delete this;
        return (0);
    }
    return m_lRefCnt;
}
STDMETHODIMP NetworkCandy::CMEventHandler::ConnectivityChanged(NLM_CONNECTIVITY NewConnectivity) {
    //
    auto isConnected = (NewConnectivity & NLM_CONNECTIVITY_IPV6_INTERNET)
                    || (NewConnectivity & NLM_CONNECTIVITY_IPV4_INTERNET);

    //
    if (isConnected != _bInternet) {
        _bInternet = isConnected;
        _connectivityChanged(isConnected);
    }

    //
    return S_OK;
}

NetworkCandy::ConnectivityManager::ConnectivityManager() {}
NetworkCandy::ConnectivityManager::~ConnectivityManager() {}

bool NetworkCandy::ConnectivityManager::isConnectedToInternet() {
    VARIANT_BOOL isConnectedToInternet;
    auto result = _manager->IsConnectedToInternet(&isConnectedToInternet);
    if(!SUCCEEDED(result)) throw std::runtime_error("Could not know if connected to internet");
    return isConnectedToInternet;
}

void NetworkCandy::ConnectivityManager::initCOM() {
    {
        // start COM
        auto result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

        //
        if(!SUCCEEDED(result)) throw std::runtime_error("COM could not start");

        //
        spdlog::info("nw-candy : COM initialized...");
    }

    {
        // get manager
        auto result = CoCreateInstance(
            CLSID_NetworkListManager, NULL,
            CLSCTX_ALL, IID_INetworkListManager,
            (LPVOID *)&_manager
        );

        //
        if(!SUCCEEDED(result)) {
            CoUninitialize();
            throw std::runtime_error("Could not get NetworkListManager");
        }

        //
        spdlog::info("nw-candy : NetworkListManager fetched...");
    }

    {
        // get connection point container from manager
        auto result = _manager->QueryInterface(IID_IConnectionPointContainer, (void **)&_managerCPC);

        //
        if(!SUCCEEDED(result)) {
            _manager->Release();
            CoUninitialize();
            throw std::runtime_error("Could not get ConnectionPointContainer from NetworkListManager");
        }

        //
        spdlog::info("nw-candy : ConnectionPointContainer fetched...");
    }

    {
        // get connection point
        auto result = _managerCPC->FindConnectionPoint(IID_INetworkListManagerEvents, &_cp);

        //
        if(!SUCCEEDED(result)) {
            _managerCPC->Release();
            _manager->Release();
            CoUninitialize();
            throw std::runtime_error("Could not get ConnectionPoint");
        }

        //
        spdlog::info("nw-candy : ConnectionPoint found...");
    }

    {
        // bind advisor to sink
        auto result = _cp->Advise((IUnknown*)this, &_cookie);

        //
        if(!SUCCEEDED(result)) {
            _cp->Release();
            _managerCPC->Release();
            _manager->Release();
            CoUninitialize();
            throw std::runtime_error("Could not bind advisor to sink");
        }

        //
        spdlog::info("nw-candy : Sink bound... OK!");
    }
}

void NetworkCandy::ConnectivityManager::releaseCOM() {
    //
    spdlog::info("nw-candy : Releasing COM...");

    this->Release();
    _cp->Unadvise(_cookie);
    _cp->Release();
    _managerCPC->Release();
    _manager->Release();
    CoUninitialize();

    //
    spdlog::info("nw-candy : Releasing COM OK!");
}

void NetworkCandy::ConnectivityManager::listenForConnectivityChanges() {
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0 )) {
        //
        spdlog::info("nw-candy : COM message received! translating and dispatching...");

        //
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        //
        spdlog::info("nw-candy : COM message Dispatched !");
    }
}

void NetworkCandy::ConnectivityManager::_connectivityChanged(bool isConnectedToInternet) {
    spdlog::info("Connectivity changed : {}", isConnectedToInternet);
}
