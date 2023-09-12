/* Copyright (C) 2020 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdbool.h>
#include <stdlib.h>

#include <string.h>
#include <cJSON.h>

#include "iotconnect_common.h"
#include "iotconnect_device_identity.h"

const char * iotcl_ddim_auth_request_create_serialized_string(IotclDdimAuthRequest* request) {
	const char* ret = NULL;
    cJSON *request_object = cJSON_CreateObject();
    if (!request_object) return NULL;
    if (!cJSON_AddStringToObject(request_object, "fmt", request->fmt)) goto cleanup;;
    if (!cJSON_AddStringToObject(request_object, "bcert", request->bcert)) goto cleanup;
    ret = cJSON_PrintUnformatted(request_object);

    cleanup:
    cJSON_free(request_object);
    return ret;
}


const char * iotcl_ddim_sign_request_create_serialized_string(IotclDdimSignRequest* request) {
    const char* ret = NULL;
    cJSON *request_object = cJSON_CreateObject();
	if (!request_object) return NULL;
    if(!cJSON_AddStringToObject(request_object, "fmt", request->fmt)) goto cleanup;
    if(!cJSON_AddStringToObject(request_object, "sig", request->sig)) goto cleanup;
    if(!cJSON_AddStringToObject(request_object, "csr", request->csr)) goto cleanup;;
    ret = cJSON_PrintUnformatted(request_object);

    cleanup:
    cJSON_free(request_object);
    return ret;
}

const char * iotcl_ddim_ack_request_create_serialized_string(IotclDdimAckRequest* request) {
	const char* ret = NULL;
    cJSON *request_object = cJSON_CreateObject();
    if (!request_object) return NULL;
    if (!cJSON_AddStringToObject(request_object, "cid", request->cid)) goto cleanup;
    ret = cJSON_PrintUnformatted(request_object);

    cleanup:
    cJSON_free(request_object);
    return ret;
}


void iotcl_ddim_destory_serialized_string(const char *serialized_string) {
    cJSON_free((char *) serialized_string);
}

IotclDdimAuthResponse * iotcl_ddim_parse_auth_response(const char* response_data) {
    IotclDdimAuthResponse *response = (IotclDdimAuthResponse *) calloc(1, sizeof(IotclDdimAuthResponse));
    if (NULL == response) {
        return NULL;
    }

    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        response->message = "IOTCL Parsing Error";
        return response;
    }
    response->rn = safe_get_string_and_strdup(json_root, "rn");
    response->cn = safe_get_string_and_strdup(json_root, "cn");
    response->cid = safe_get_string_and_strdup(json_root, "cid");
    response->message = safe_get_string_and_strdup(json_root, "message");
    response->ec = get_numeric_value_or_default(json_root, "ec", -1);

    if (!response->rn || !response->cn || !response->cid || !response->message) {
    	// out of ram possibly. The fields re required.
    	iotcl_ddim_free_auth_response(response);
    	response = NULL;
    }

    // we have duplicated the strings, so we can now free the result
    cJSON_Delete(json_root);
    return response;
}
IotclDdimSignResponse* iotcl_ddim_parse_sign_response(const char* response_data) {
	IotclDdimSignResponse *response = (IotclDdimSignResponse *) calloc(1, sizeof(IotclDdimSignResponse));
    if (NULL == response) {
        return NULL;
    }

    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        response->message = "IOTCL Parsing Error";
        return response;
    }
    response->cert = safe_get_string_and_strdup(json_root, "cert");
    response->message = safe_get_string_and_strdup(json_root, "message");
    response->ec = get_numeric_value_or_default(json_root, "ec", -1);

    if (!response->cert || !response->message) {
    	// out of ram possibly. The fields re required.
    	iotcl_ddim_free_sign_response(response);
    	response = NULL;
    }
    // we have duplicated the strings, so we can now free the result
    cJSON_Delete(json_root);
    return response;
}

IotclDdimAckResponse* iotcl_ddim_parse_ack_response(const char* response_data) {
	IotclDdimAckResponse *response = (IotclDdimAckResponse *) calloc(1, sizeof(IotclDdimAckResponse));
    if (NULL == response) {
        return NULL;
    }

    cJSON *json_root = cJSON_Parse(response_data);
    if (!json_root) {
        response->message = "IOTCL Parsing Error";
        return response;
    }
    response->message = safe_get_string_and_strdup(json_root, "message");
    response->ec = get_numeric_value_or_default(json_root, "ec", -1);

    // we have duplicated the strings, so we can now free the result
    cJSON_Delete(json_root);
    return response;
}


void iotcl_ddim_free_auth_response(IotclDdimAuthResponse *response) {
    if (response) {
        free(response->rn);
        free(response->cn);
        free(response->cid);
        if (response->message) {
            free(response->message);
        }
        free(response);
    }
}

void iotcl_ddim_free_sign_response(IotclDdimSignResponse *response) {
    if (response) {
        free(response->cert);
        if (response->message) {
            free(response->message);
        }
        free(response);
    }
}

void iotcl_ddim_free_ack_response(IotclDdimAckResponse *response) {
    if (response) {
        if (response->message) {
            free(response->message);
        }
        free(response);
    }
}
