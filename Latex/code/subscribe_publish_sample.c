...

static QueueHandle_t gpio_evt_queue = NULL;

// Interrupt Service Routine
static void gpio_isr_handler(void* arg)
{
    gpio_num_t gpio_num = (gpio_num_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

...

// Einbinden der Zertifikate fuer AWS Core IoT
extern const uint8_t aws_root_ca_pem_start[] 
    asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] 
    asm("_binary_aws_root_ca_pem_end");
extern const uint8_t certificate_pem_crt_start[] 
    asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] 
    asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] 
    asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] 
    asm("_binary_private_pem_key_end");

...

static void event_handler(
    void* handler_args, esp_event_base_t base, int32_t id, 
    void* event_data)
{
    ...
}

// Verarbeiten eingehender Nachrichten
void iot_subscribe_callback_handler(
    AWS_IoT_Client* pClient, char* topicName, uint16_t topicNameLen,
    IoT_Publish_Message_Params* params, void* pData)
{
    cJSON *root, *isCorrect = NULL;
    root = cJSON_ParseWithLength((char*)params->payload, 
        (size_t)params->payloadLen);
    isCorrect = cJSON_GetObjectItem(root, "isCorrect");
    if (isCorrect) {
        gpio_set_level(GPIO_OUTPUT_IO_0, 
            cJSON_GetNumberValue(isCorrect));
        gpio_set_level(GPIO_OUTPUT_IO_1, 
            !cJSON_GetNumberValue(isCorrect));
        vTaskDelay(pdMS_TO_TICKS(800));
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        gpio_set_level(GPIO_OUTPUT_IO_1, 0);
    }
}

void disconnectCallbackHandler(AWS_IoT_Client* pClient, void* data)
{
    ...
}

// Hauptprogramm 
void aws_iot_task(void* param)
{
    ...
    
    do {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if (SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", 
                rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while (SUCCESS != rc);

    ...

    const char* TOPIC = "test_topic/esp32";
    const int TOPIC_LEN = strlen(TOPIC);

    ESP_LOGI(TAG, "Subscribing...");
    rc = aws_iot_mqtt_subscribe(
        &client, TOPIC, TOPIC_LEN, QOS0, 
        iot_subscribe_callback_handler, NULL);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    }

    cJSON *root, *answer;
    root = cJSON_CreateObject();
    answer = cJSON_CreateString("A");
    cJSON_AddItemToObject(root, "answer", answer);

    sprintf(cPayload, "%s", cJSON_Print(root));

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void*)cPayload;
    paramsQOS0.isRetained = 0;

    // Senden von Nachrichten an den MQTT Endpunkt von AWS Core IoT
    while ((NETWORK_ATTEMPTING_RECONNECT == rc 
        || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

        ...

        gpio_num_t io_num;
        if (xQueueReceive(gpio_evt_queue, 
            &io_num, pdMS_TO_TICKS(100))) {
            const char answerLetter = 'A' + io_num - GPIO_INPUT_IO_0;
            cJSON_SetValuestring(answer, &answerLetter);
            sprintf(cPayload, "%s", cJSON_Print(root));
            paramsQOS0.payloadLen = strlen(cPayload);
            rc = aws_iot_mqtt_publish(
                &client, TOPIC, TOPIC_LEN, &paramsQOS0);
        }
    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");
    abort();
}

static void initialise_wifi(void)
{
    ...
}

// Setup-Code
void app_main()
{
    esp_log_level_set("*", ESP_LOG_NONE);

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; // disable interrupt
    io_conf.mode = GPIO_MODE_OUTPUT; // set as output mode
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL; // bit mask of the pins
    io_conf.pull_down_en = 0; // disable pull-down mode
    io_conf.pull_up_en = 0; // disable pull-up mode
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_POSEDGE; // interrupt of rising edge
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL; // bit mask of the pins
    io_conf.mode = GPIO_MODE_INPUT; // set as input mode
    io_conf.pull_down_en = 1; // enable pull-up mode
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(gpio_num_t));

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, 
        (void*)GPIO_INPUT_IO_0);
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, 
        (void*)GPIO_INPUT_IO_1);
    gpio_isr_handler_add(GPIO_INPUT_IO_2, gpio_isr_handler, 
        (void*)GPIO_INPUT_IO_2);
    gpio_isr_handler_add(GPIO_INPUT_IO_3, gpio_isr_handler, 
        (void*)GPIO_INPUT_IO_3);

    ...

    initialise_wifi();
    xTaskCreatePinnedToCore(
        &aws_iot_task, "aws_iot_task", 9216, NULL, 5, NULL, 1);
}
