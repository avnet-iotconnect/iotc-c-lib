/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for specific platforms or help implement custom approaches
 * for to IoTConnect discovery HTTP API.
 */

#ifndef IOTCONNECT_DISCOVERY_H
#define IOTCONNECT_DISCOVERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iotconnect_lib_config.h"

typedef enum {
    IOTCL_SR_OK = 0,
    IOTCL_SR_DEVICE_NOT_REGISTERED = 1,
    IOTCL_SR_AUTO_REGISTER,
    IOTCL_SR_DEVICE_NOT_FOUND,
    IOTCL_SR_DEVICE_INACTIVE,
    IOTCL_SR_DEVICE_MOVED,
    IOTCL_SR_CPID_NOT_FOUND,
    IOTCL_SR_UNKNOWN_DEVICE_STATUS = 20,
    IOTCL_SR_ALLOCATION_ERROR,
    IOTCL_SR_PARSING_ERROR,
} IotclSyncResult;

//NOTE: Append "sync?" to the base url or path in order to to do a sync POST
typedef struct IotclDiscoveryResponse {
    char *url;
    char *host; // parsed out host from the url
    char *path; // parsed out base ULR request path
} IotclDiscoveryResponse;

typedef struct IotclSyncResponseProtocolTopics {
    char *pub_topic;
    char *sub_topic;
} IotclSyncResponseProtocolTopics;

typedef struct IotclSyncResponseProtocol {
    char *client_id;
    char *name;
    char *host;
    char *user_name;
    char *pass;
    IotclSyncResponseProtocolTopics topics;
} IotclSyncResponseProtocol;

typedef struct IotclSyncResponseMeta {
    int at; // validated authorization type from the cloud
    char *cd; // validated authorization type from the cloud
} IotclSyncResponseMeta;

typedef struct IotclSyncResponse {
    IotclSyncResult ds;
    int ec;
    int ct;
    IotclSyncResponseMeta meta;
    IotclSyncResponseProtocol broker;
} IotclSyncResponse;

// You must free the response when done
// Returned NULL means that there was a memory allocation or a parsing error
IotclDiscoveryResponse *iotcl_discovery_parse_discovery_response(const char *response_data);

void iotcl_discovery_free_discovery_response(IotclDiscoveryResponse *response);

// This function returns NULL in case of allocation failure
// The user mast check the ds value for "OK". Corresponding error should be handled/reported and the response should be freed
int iotcl_discovery_parse_sync_response(const char *response_data, IotclSyncResponse **presponse);

void iotcl_discovery_free_sync_response(IotclSyncResponse *response);


#ifdef __cplusplus
}
#endif

#endif //IOTCONNECT_DISCOVERY_H
