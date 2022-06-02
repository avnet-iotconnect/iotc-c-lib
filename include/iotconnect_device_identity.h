/* Copyright (C) 2022 Avnet - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for DDIM support
 *
 * DDIM Sequence:
 * 1) The device presents the bootstrap certificate to the server
 * 2) The server responds with certificate ID (cid), random number string (rn), and common name
 * for the operational certificate for which the device needs to create a CSR
 * 3) The device signs the CID string, the random number string and the CSR (binary)
 * with SHA265 hash using the operational identity (CSR's) private key.
 * 4) The device sends the CSR and the signature to the server.
 * 5) The server responds with the operational certificate that the device can use to connect
 * to the operational IoTHub.
 * 5) The device writes the newly obtained certificate into the secure element's storage.
 * 6) The device confirms that was able to store the certificate by exiting the "ack" server endpoint with
 * the CID obtained in step #2.
 */

#ifndef IOTCONNECT_DEVICE_IDENTITY_H
#define IOTCONNECT_DEVICE_IDENTITY_H

#ifdef __cplusplus
extern "C" {
#endif
 

typedef struct IotclDdimAuthRequest {
	char *bcert; // bootstrap certificate
	char *fmt; // format "hex" (der) by default
} IotclDdimAuthRequest;

typedef struct IotclDdimAuthResponse {
	char *rn; // random number
	char *cn; // common name
	char *cid; // certificate id
	char *message; // error message
	int ec; // error code
} IotclDdimAuthResponse;

typedef struct IotclDdimSignRequest {
	char *sig; // error message
	char *csr; // csr
	char *fmt; // format "hex" (der) by default
} IotclDdimSignRequest;

typedef struct IotclDdimSignResponse {
	char *cert; // operational cert
	char *message; // error message
	int ec; // error code
} IotclDdimSignResponse;

typedef struct IotclDdimAckRequest {
	char *cid;
} IotclDdimAckRequest;

typedef struct IotclDdimAckResponse {
	char *message; // error message
	int ec; // error code
} IotclDdimAckResponse;

const char * iotcl_ddim_auth_request_create_serialized_string(IotclDdimAuthRequest* request);
const char * iotcl_ddim_sign_request_create_serialized_string(IotclDdimSignRequest* request);
const char * iotcl_ddim_ack_request_create_serialized_string(IotclDdimAckRequest* request);

// Free a string returned by any of the *create_serialized calls
void iotcl_ddim_destory_serialized_string(const char *serialized_string);

IotclDdimAuthResponse * iotcl_ddim_parse_auth_response(const char* response_data);
void iotcl_ddim_free_auth_response(IotclDdimAuthResponse *response);

IotclDdimSignResponse* iotcl_ddim_parse_sign_response(const char* response_data);
void iotcl_ddim_free_sign_response(IotclDdimSignResponse *response);

IotclDdimAckResponse* iotcl_ddim_parse_ack_response(const char* response_data);
void iotcl_ddim_free_ack_response(IotclDdimAckResponse *response);

#ifdef __cplusplus
}
#endif

#endif //IOTCONNECT_DEVICE_IDENTITY_H
