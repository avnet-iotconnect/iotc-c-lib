This example shows basic telemetry and library configuration. We are showcasing a dedicated AWS instance
configuration in this example with iotcl_telemetry_example_entry() being the example's starting call. 

In this example shows common basic steps that are required to use the library:
* Configure the library module once. Configure my_transport_send() as mqtt_send_cb.
* Connect the MQTT client (example: mqtt_client_* functions) to the endpoint.
* Obtain the c2d topic name and and subscribe to the c2d topic.
* Read your sensor device (example: somesensor* functions) and set values with iotcl_telemetry_set functions.
* With my_transport_send(), tell the library to send the data JSON via your configured my_transport_send function.
* Destroy the telemetry message with iotcl_telemetry_destroy() and deinit the library when it's not longer needed.

```c
void iotcl_telemetry_example_entry(void) {
    IotclClientConfig config;
    iotcl_init_client_config(&config);
    config.device.instance_type = IOTCL_DCT_AWS_DEDICATED;
    config.device.duid = "mydevice";
    config.mqtt_send_cb = my_transport_send;
    iotcl_init(&config);
    
    mqtt_client_connect(...); // connect using your client
    
    // subscribe to the C2D topic:
    IotclDeviceConfig *c = iotcl_mqtt_get_config();

    // subscribe to the topic using your mqtt client. In this example, the subscribing mechanism
    // is a callback function which will be called when the MQTT client receives data.
    mqtt_client_subscribe(c->sub_c2d, on_mqtt_client_topic_data);
    
    while (mqtt_client_is_connected()) {
        // In this example we are using an mqtt client that needs periodic polling.
        // polling will trigger the configured on_mqtt_client_topic_data() callback.
        mqtt_client_loop_and_poll();
        sleep(1000);
    }
    
    iotcl_deinit();
}

static void my_transport_send(const char *topic, size_t topic_len, const char *json_str) {
    // example mqtt hook:
    mqtt_client_send(topic, topic_len, json_str, strlen(json_str)); 
}
```