#include "biubiubiu.h"
#include "biubiubiu_gui.h"
#include "sys/app_controller.h"
#include "common.h"

#include "mqtt_client.h"

#define BIUBIUBIU_APP_NAME "Biubiubiu"

enum biu_event_Id
{
    SEND_LOVE,
    WIFICONN
};

#define BIUBIUBIU_CONFIG_PATH "/biubiubiu.cfg"
struct BIU_Config
{
    int display_times;
    int frame_num;
    int wifi_alive_interval;
    int connect_server_interval;
};

static void write_config(BIU_Config *cfg)
{
    // g_flashCfg.writeFile(BIUBIUBIU_CONFIG_PATH, w_data.c_str());
}

static void read_config(BIU_Config *cfg)
{
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(BIUBIUBIU_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0) {
        cfg->display_times = 10;
        cfg->frame_num = 8;
        cfg->wifi_alive_interval = 500;
        cfg->connect_server_interval = 2500;
        write_config(cfg);
    } else {
    }
}

typedef enum {INIT, WAITWIFI, WAITSERVER, ALLDONE} state_t;

struct BiubiubiuAppRunData {
    bool connected_server;
    int display_times;
    esp_mqtt_client_handle_t client;
    state_t state;
    unsigned long wifi_alive_timestamp;
    unsigned long connect_server_timestamp;
};

static BIU_Config cfg_data;
static struct BiubiubiuAppRunData *run_data = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) 
    {
        // 建立连接成功
        case MQTT_EVENT_CONNECTED:
            Serial.println("Connect server ok. \n");
            // 订阅主题
            esp_mqtt_client_subscribe(client, "pull210106", 0);         
            run_data->connected_server = true;  
            break;
        // 客户端断开连接
        case MQTT_EVENT_DISCONNECTED:
            Serial.println("Server disconnected. \n");
            esp_mqtt_client_reconnect(client);
            run_data->connected_server = false;
            break;
        // 已收到订阅的主题消息
        case MQTT_EVENT_DATA:
            // Serial.printf("mqtt received topic: %.*s \n",event->topic_len, event->topic);
            // Serial.printf("topic data: %.*s\r\n", event->data_len, event->data);
            run_data->display_times = cfg_data.display_times * cfg_data.frame_num;
            break;
        // 客户端遇到错误
        case MQTT_EVENT_ERROR:
            Serial.println("MQTT_EVENT_ERROR \n");
            break;
        default:
            Serial.printf("Other event id:%d \n", event->event_id);
            break;
    }
}

static void mqtt_init() {
    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(esp_mqtt_client_config_t));
    mqtt_cfg.host = "47.111.117.220";
    mqtt_cfg.port = 1883;

    run_data->client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(run_data->client, MQTT_EVENT_ANY, mqtt_event_handler, run_data->client);
    return;
}

static void close_server(AppController *sys) {
    esp_mqtt_client_disconnect(run_data->client);
    sys->send_to(BIUBIUBIU_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_DISCONN, NULL, NULL);
}

static void send_love() {
    if (run_data->state != ALLDONE) {
        return;
    }
}

static int biubiubiu_init(void) {
    biubiubiu_gui_init();
    read_config(&cfg_data);

    run_data = (BiubiubiuAppRunData *)calloc(1, sizeof(BiubiubiuAppRunData));
    run_data->state = INIT;
    run_data->connected_server = false;
    run_data->wifi_alive_timestamp = millis();

    mqtt_init();

    return 0;
}

static void biubiubiu_process(AppController *sys,
                            const ImuAction *act_info) {

    if (RETURN == act_info->active) {
        close_server(sys);
        sys->app_exit();
        return;
    } else if (GO_FORWORD == act_info->active) {
        send_love();
        delay(300);
    }

    if (run_data->connected_server) {
        run_data->state = ALLDONE;
    }

    if (run_data->state == INIT) {
        sys->send_to(BIUBIUBIU_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, (void *)WIFICONN, NULL);
        run_data->state = WAITWIFI;
        return;
    } else if (run_data->state == WAITWIFI) {
        return;
    } else if (run_data->state > WAITWIFI && WiFi.status() != WL_CONNECTED) {
        run_data->state = INIT;
    } else if (doDelayMillisTime(cfg_data.wifi_alive_interval, &run_data->wifi_alive_timestamp, false)) {
        sys->send_to(BIUBIUBIU_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_ALIVE, 0, NULL);
    }

    if (run_data->state == WAITSERVER && doDelayMillisTime(cfg_data.connect_server_interval, &run_data->connect_server_timestamp, false)) {
        Serial.println("start client");
        esp_mqtt_client_start(run_data->client);
    }

    if (run_data->display_times-- > 0) {
        display_biubiubiu();
        delay(30);
    }
}

static int biubiubiu_exit_callback(void *param)
{
    biubiubiu_gui_del();

    free(run_data);
    run_data = NULL;
    return 0;
}

static void biubiubiu_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info) {
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN: {
        int event_id = (int)message;
        switch (event_id) {
            case WIFICONN:
                Serial.println("waitwifi callback");
                if (WiFi.status() == WL_CONNECTED) {
                    run_data->state = WAITSERVER;
                } else {
                    run_data->state = INIT;
                }
                break;
        }
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {

    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {

    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ biubiubiu_app = {BIUBIUBIU_APP_NAME, &app_biubiubiu, "",
                       biubiubiu_init, biubiubiu_process,
                       biubiubiu_exit_callback, biubiubiu_message_handle};