/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

/*
 * This file contains functions that aid in developing SDKs for specific platforms or help implement custom approaches
 * for to IoTConnect discovery HTTP API.
 * This URL module allows for easy construction, re-construction of URLs and splitting the URL
 * into host and resource path without having to maintain memory allocations for each piece.
 */

#ifndef IOTCL_DRA_URL_H
#define IOTCL_DRA_URL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Ensure to clear this struct before passing it to iotcl_dra_url_init* functions.
// The user should not use this structure directly.
// It contains the cloned original URL and work state variables for maintaining base URL and suffix paths that
// can be appended to the base URL.
// Pass this context to iotcl_dra_url functions to build different URLs and obtain their forms.
typedef struct {
    char *url;          // for a given example full URL "https://apihost.io/baseurl/path":

    char *hostname;     // apihost.io, cloned from the original URL

    // If 5 bytes are are added as slack, then the buffer (string) length will be the original URL + 5.
    // If reallocated, to fit a suffix, the length will update
    size_t buffer_length;

    // https://apihost.io/baseurl/path
    //                   ^ points here
    size_t idx_path;       //

    // https://apihost.io/baseurl/path
    //                                ^ points here. We can write and overwrite leaves here
    size_t idx_suffix_start;

    bool is_https;      // true if starts with https://
} IotclDraUrlContext;

// url is a full url prefixed with https:// or http://
// Uses zero slach for iotcl_dra_url_use_suffix_path operations
int iotcl_dra_url_init(IotclDraUrlContext *c, const char *url);

// adds slack number of bytes to the URL buffer. See iotcl_dra_url_use_suffix_path()
int iotcl_dra_url_init_with_slack(IotclDraUrlContext *c, size_t slack, const char *url);

// NOTE: After calling iotcl_dra_url_use_suffix_path(), the returned reference may become invalid
const char *iotcl_dra_url_get_url(const IotclDraUrlContext *c);

const char *iotcl_dra_url_get_hostname(const IotclDraUrlContext *c);

// NOTE: After calling iotcl_dra_url_use_suffix_path(), the returned reference may become invalid
const char *iotcl_dra_url_get_resource(const IotclDraUrlContext *c);

// Can be used to set the port, but we generally use https URls.
bool iotcl_dra_url_is_https(const IotclDraUrlContext *c);

// It will append a suffix to the original URL and ALSO potentially invalidate returned references from
// iotcl_dra_url_get_url() or iotcl_dra_url_get_resource().
// For example, if original (base) URL passed into the init function is https://myhost.com/api/v4.6 ,
// calling iotcl_dra_url_use_suffix_path(c, "/my/path") will append to the url, iotcl_dra_url_get_url
// resulting in  https://myhost.com/api/v4.6/my/path being returned by
// Calling iotcl_dra_url_use_suffix_path subsequently will overwrite the suffix part of the URL with the new one
// If the suffix does not fit into the URL buffer, the original buffer will be reallocated to fit the string
// If the URL was initialized with iotcl_dra_url_init_with_slack(), the slack amount would allow this function
// to potentially re-use the slack, and not have to reallocate the buffer.
// You can also reset the URL back tot he original base URL by calling this function with an empty string.
int iotcl_dra_url_use_suffix_path(IotclDraUrlContext *c, const char *suffix);

void iotcl_dra_url_deinit(IotclDraUrlContext *c);


#ifdef __cplusplus
}
#endif

#endif // IOTCL_DRA_URL_H
