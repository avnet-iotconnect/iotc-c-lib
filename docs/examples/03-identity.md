This example shows how to configure the library with discovery/identity HTTP REST API. 

In this example shows common basic steps that are required to use the library:
* Configure the library module with IOTCL_DCT_CUSTOM.
* Construct the discovery URL and run HTTP GET for discovery.
* Parse the HTTP response and obtain the Base API URL.
* Build the identity API URL on top of the Base URL and run HTTP GET for identity.
* Configure the library with the HTTP response from Identity.
* Destroy the URLs.

```c
int run_http_identity(IotConnectConnectionType ct, const char *cpid, const char *env, const char* duid) {
    IotclDraUrlContext discovery_url = {0};
    IotclDraUrlContext identity_url = {0};
    IotclClientConfig config;

    iotcl_init_client_config(&config);
    // just tell the library that we will use custom mqtt configuration with identity response
    config.device.instance_type = IOTCL_DCT_CUSTOM;
    iotcl_init(&config);
    
    iotcl_dra_discovery_init_url_aws(&discovery_url, cpid, env);

    // run HTTP GET with your http client with application/json content type
    my_http_client_http_get(iotcl_dra_url_get_url(&discovery_url));

    // parse the REST API base URL from discovery response
    iotcl_dra_discovery_parse(&identity_url, 0, my_http_client_body());

    // build  the actual identity API REST url on top of the base URL
    iotcl_dra_identity_build_url(&identity_url, duid);
    
    // run HTTP GET with your http client with application/json content type
    my_http_client_http_get(iotcl_dra_url_get_url(&identity_url));

    // pass the body of the response to configure the MQTT library
    iotcl_dra_identity_configure_library_mqtt(my_http_client_body());    
    
    // from here on you can call iotcl_mqtt_get_config() and use the iotcl mqtt functions 

    iotcl_dra_url_deinit(&discovery_url);
    iotcl_dra_url_deinit(&identity_url);

}
```

This basic example is simplified for readability purposes. 
Keep the following things in mind while implementing code based on the example above:
* Check the return codes for each of the DRA module function calls.
* Check the results from your HTTP client.
* Only HTTPS is supported for Device REST API on the back end, so you will need to 
configure your HTTP client as such. Use the oDaddy Secure Server Certificate (Intermediate Certificate) - G2
as server CA, which is available as C string in iotcl_certs.h and as gdig2.pem file in the util/server-ca-cert-files
directory in this repo.
