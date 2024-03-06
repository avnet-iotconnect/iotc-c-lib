/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <string.h>

#include "cJSON.h"

// MBEDTLS config file style - include your own to override the config. See iotcl_example_config.h
#if defined(IOTCL_USER_CONFIG_FILE)
#include IOTCL_USER_CONFIG_FILE
#endif

#include "iotcl_internal.h"
#include "iotcl_cfg.h"
#include "iotcl_log.h"
#include "iotcl_dra_url.h"

int iotcl_dra_url_init_with_slack(IotclDraUrlContext* c, size_t slack, const char *url) {
    const char *const HTTPS_HEADER = "https://";
    const char *const HTTP_HEADER = "http://";

    if (!url || 0 == strlen(url)) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA URL: URL is empty or NULL");
        return IOTCL_ERR_MISSING_VALUE;
    }

    char *url_start = strstr(url, HTTPS_HEADER);
    if (!url_start) {
        url_start = strstr(url, HTTP_HEADER);
        if (!url_start) {
            IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA URL: Unable to parse URL header from \"%s\"", url);
            return IOTCL_ERR_PARSING_ERROR;
        }
        c->is_https = false;
    } else {
        c->is_https = true;
    }

    // mark hostname start after the http(s)://
    const char *hostname_start = c->is_https ? &url[strlen(HTTPS_HEADER)] : &url[strlen(HTTP_HEADER)];

    // look for the slash after the hostname
    char *path_start = strstr(hostname_start, "/");

    if (!path_start) {
        IOTCL_ERROR(IOTCL_ERR_PARSING_ERROR, "DRA URL: Unable to parse URL %s", url);
        return IOTCL_ERR_PARSING_ERROR;
    }

    // add reasonable slack into a buffer so we don't have to reallocate the buffer in most cases
    c->buffer_length = strlen(url) + slack;
    c->url = iotcl_malloc(c->buffer_length + 1);
    if (!c->url) {
        c->buffer_length = 0;
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA URL: Out of memory while allocating the URL buffer!");
        return IOTCL_ERR_OUT_OF_MEMORY;
    }
    strcpy(c->url, url);

    size_t hostname_len = (size_t) (path_start - hostname_start); // pointer difference
    c->hostname = iotcl_malloc(hostname_len + 1);
    if (!c->hostname) {
        iotcl_free(c->url);
        c->url = NULL;
        c->buffer_length = 0;
        IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA URL: Out of memory while allocating the hostname buffer!");
        return IOTCL_ERR_OUT_OF_MEMORY;
    }

    memcpy(c->hostname, hostname_start, hostname_len);
    c->hostname[hostname_len] = 0; // terminate the string

    c->idx_suffix_start = strlen(url);
    c->idx_path = (int) (path_start - url); // pointer subtraction
    return IOTCL_SUCCESS;
}
int iotcl_dra_url_init(IotclDraUrlContext* c,const char *url) {
    return iotcl_dra_url_init_with_slack(c, 0, url);
}
const char *iotcl_dra_url_get_url(const IotclDraUrlContext* c) {
    if (!c || !c->url) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA URL: Url context argument invalid!");
        return NULL;
    }
    return c->url;
}

const char *iotcl_dra_url_get_hostname(const IotclDraUrlContext* c) {
    if (!c || !c->url) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA URL: Url context argument invalid!");
        return NULL;
    }
    return c->hostname;
}

const char *iotcl_dra_url_get_resource(const IotclDraUrlContext* c) {
    if (!c || !c->url) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA URL: Url context argument missing!");
        return NULL;
    }
    return &c->url[c->idx_path];
}
bool iotcl_dra_url_is_https(const IotclDraUrlContext *c) {
    if (!c || !c->url) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA URL: Url context argument missing!");
        return false; // not much we can do about the return
    }
    return c->is_https;
}

int iotcl_dra_url_use_suffix_path(IotclDraUrlContext *c, const char *suffix) {
    if (!c || !c->url || !suffix) {
        IOTCL_ERROR(IOTCL_ERR_MISSING_VALUE, "DRA URL: Context and suffix are required!");
        return IOTCL_ERR_MISSING_VALUE;
    }

    const size_t suffix_len = strlen(suffix);
    if (c->buffer_length - c->idx_suffix_start < suffix_len) {
        const size_t new_buffer_len = c->idx_suffix_start + suffix_len;
        char * new_url = iotcl_malloc(new_buffer_len + 1);
        if (!new_url) {
            IOTCL_ERROR(IOTCL_ERR_OUT_OF_MEMORY, "DRA URL: Out of memory while reallocating the buffer for suffix %s", suffix);
            return IOTCL_ERR_OUT_OF_MEMORY;
        }
        strcpy(new_url, c->url);
        iotcl_free(c->url);
        c->url = new_url;
        c->buffer_length = new_buffer_len;
    }
    strcpy(&c->url[c->idx_suffix_start], suffix);

    return IOTCL_SUCCESS;
}
void iotcl_dra_url_deinit(IotclDraUrlContext* c) {
    iotcl_free(c->url);
    iotcl_free(c->hostname);
    memset(c, 0, sizeof(IotclDraUrlContext));
}
