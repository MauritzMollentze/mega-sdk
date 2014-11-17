//
//  DelegateMEGAGlobalListener.m
//
//  Created by Javier Navarro on 08/10/14.
//  Copyright (c) 2014 MEGA. All rights reserved.
//

#import "DelegateMEGAGlobalListener.h"
#import "MEGAUserList+init.h"
#import "MEGANodeList+init.h"

using namespace mega;

DelegateMEGAGlobalListener::DelegateMEGAGlobalListener(MEGASdk *megaSDK, id<MEGAGlobalDelegate>listener) {
    this->megaSDK = megaSDK;
    this->listener = listener;
}

id<MEGAGlobalDelegate> DelegateMEGAGlobalListener::getUserListener() {
    return listener;
}

void DelegateMEGAGlobalListener::onUsersUpdate(mega::MegaApi *api, mega::MegaUserList *userList) {
    if (listener !=nil) {
        MegaUserList *tempUserList = NULL;
        if (userList) {
            tempUserList = userList->copy();
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [listener onUsersUpdate:this->megaSDK userList:(tempUserList ? [[MEGAUserList alloc] initWithUserList:tempUserList cMemoryOwn:YES] : nil)];
        });
        
    }
}

void DelegateMEGAGlobalListener::onNodesUpdate(mega::MegaApi *api, mega::MegaNodeList *nodeList) {
    if (listener !=nil) {
        MegaNodeList *tempNodesList = NULL;
        if (nodeList) {
            tempNodesList = nodeList->copy();
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [listener onNodesUpdate:this->megaSDK nodeList:(tempNodesList ? [[MEGANodeList alloc] initWithNodeList:tempNodesList cMemoryOwn:YES] : nil)];
        });
    }
}

void DelegateMEGAGlobalListener::onReloadNeeded(mega::MegaApi* api) {
    if (listener !=nil) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [listener onReloadNeeded:this->megaSDK];
        });
    }
}