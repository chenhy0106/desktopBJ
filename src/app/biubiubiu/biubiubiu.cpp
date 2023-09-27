#include "biubiubiu.h"
#include "biubiubiu_gui.h"
#include "sys/app_controller.h"
#include "common.h"

#include "ESP32Time.h"
#include "mqtt_client.h"

#define BIUBIUBIU_APP_NAME "Biubiubiu"

enum biu_event_Id
{
    SEND_LOVE,
    WIFI_HANDLE
};

#define BIUBIUBIU_CONFIG_PATH "/biubiubiu.cfg"
struct BIU_Config
{
    int display_times;
    int wifi_alive_interval;
    int timestamp_interval;
    int connect_server_interval;
    int mqtt_send_interval;
};

static void write_config(BIU_Config *cfg)
{
}

static void read_config(BIU_Config *cfg)
{
    cfg->display_times = 6;
    cfg->wifi_alive_interval = 10000;
    cfg->timestamp_interval = 120000;
    cfg->connect_server_interval = 2500;
    cfg->mqtt_send_interval = 2000;
}

#define MQTT_SENDLOVE_TOPIC "/biubiubiuProject/push_210106"
#define MQTT_RECVLOVE_TOPIC "/biubiubiuProject/pull_210106"

#define TIME_URL "http://api.m.taobao.com/rest/api3.do?api=mtop.common.gettimestamp"

struct BiubiubiuAppRunData {
    int display_times;
    int show_ok_times;
    esp_mqtt_client_handle_t client;

    bool wait_wifi_flag;
    bool connected_server_flag;
    bool send_flag;

    unsigned long wifi_alive_timestamp;
    unsigned long time_timestamp;
    unsigned long connect_server_timestamp;
    unsigned long mqtt_send_timestamp;

    long long preNetTimestamp;      // 上一次的网络时间戳
    long long preLocalTimestamp;    // 上一次的本地机器时间戳

    ESP32Time g_rtc;
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
            DPRINTF("Connect server ok. \n");
            // 订阅主题
            esp_mqtt_client_subscribe(client, MQTT_RECVLOVE_TOPIC, 0);         
            run_data->connected_server_flag = true;  
            break;
        // 客户端断开连接
        case MQTT_EVENT_DISCONNECTED:
            DPRINTF("Server disconnected. \n");
            esp_mqtt_client_reconnect(client);
            run_data->connected_server_flag = false;
            break;
        // 已收到订阅的主题消息
        case MQTT_EVENT_DATA:
            run_data->display_times = cfg_data.display_times * 8;
            break;
        // 客户端遇到错误
        case MQTT_EVENT_ERROR:
            DPRINTF("MQTT_EVENT_ERROR \n");
            break;
        default:
            Serial.printf("Other event id:%d \n", event->event_id);
            break;
    }
}

static void server_init() {
    esp_mqtt_client_config_t mqtt_cfg;
    memset(&mqtt_cfg, 0, sizeof(esp_mqtt_client_config_t));
    mqtt_cfg.host = "broker-cn.emqx.io";
    mqtt_cfg.port = 1883;

    run_data->client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(run_data->client, MQTT_EVENT_ANY, mqtt_event_handler, run_data->client);
    return;
}

static void close_server(AppController *sys) {
    esp_mqtt_client_stop(run_data->client);
    esp_mqtt_client_disconnect(run_data->client);
    esp_mqtt_client_destroy(run_data->client);
    sys->send_to(BIUBIUBIU_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_DISCONN, NULL, NULL);
}

static void send_love() {
    DPRINTF("[Function] send_love");
    esp_mqtt_client_publish(run_data->client, MQTT_SENDLOVE_TOPIC, NULL, 0, 0, 1);
    return;
}

static void update_time() {
    DPRINTF("[Function] update_time");

    if (WL_CONNECTED != WiFi.status())
        return;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(TIME_URL);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            DPRINTF(payload);
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            run_data->preNetTimestamp = atoll(time.c_str()) + 2 + TIMEZERO_OFFSIZE;
            run_data->preLocalTimestamp = millis();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
        run_data->preLocalTimestamp = millis();
    }
    http.end();
    
    return;
}

static void show_time() {
    DPRINTF("[Function] show_time");

    run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
    run_data->preLocalTimestamp = millis();

    struct TimeStr t;
    run_data->g_rtc.setTime(run_data->preNetTimestamp / 1000);
    t.year = run_data->g_rtc.getYear();
    t.month = run_data->g_rtc.getMonth() + 1;
    t.day = run_data->g_rtc.getDay();
    t.hour = run_data->g_rtc.getHour(true);
    t.minute = run_data->g_rtc.getMinute();
    
    display_biubiubiu_time(t);
    
    return;
}

static int biubiubiu_init(void) {
    biubiubiu_gui_init();
    read_config(&cfg_data);

    run_data = (BiubiubiuAppRunData *)malloc(sizeof(BiubiubiuAppRunData));
    run_data->wait_wifi_flag = false;
    run_data->connected_server_flag = false;
    run_data->send_flag = false;

    run_data->wifi_alive_timestamp = 0;
    run_data->connect_server_timestamp = 0;
    run_data->mqtt_send_timestamp = 0;
    run_data->time_timestamp = 0;

    run_data->preNetTimestamp = 0;      // 上一次的网络时间戳
    run_data->preLocalTimestamp = 0;    // 上一次的本地机器时间戳
    
    run_data->display_times = 0;
    run_data->show_ok_times = 0;

    server_init();

    return 0;
}

static void biubiubiu_process(AppController *sys,
                            const ImuAction *act_info) {

    if (RETURN == act_info->active) {
        close_server(sys);
        sys->app_exit();
        return;
    } else if (GO_FORWORD == act_info->active) {
        run_data->send_flag = true;
        delay(300);
    }

    if (run_data->display_times-- > 0) {
        display_biubiubiu();
        delay(30);
    } else if (run_data->show_ok_times-- > 0) {
        display_state(0);
        delay(2000);
    } else if (WiFi.status() != WL_CONNECTED) {
        display_state(1);
    } else {
        show_time();
    }

    if (WiFi.status() != WL_CONNECTED) {
        if (!run_data->wait_wifi_flag) {
            sys->send_to(BIUBIUBIU_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, (void *)WIFI_HANDLE, NULL);
            run_data->wait_wifi_flag = true;
        }
        return;
    }

    if (doDelayMillisTime(cfg_data.wifi_alive_interval, &run_data->wifi_alive_timestamp, false)) {
        sys->send_to(BIUBIUBIU_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_ALIVE, 0, NULL);
    }

    if (doDelayMillisTime(cfg_data.timestamp_interval, &run_data->time_timestamp, false) || run_data->time_timestamp == 0) {
        update_time();
        run_data->time_timestamp = millis();
    }

    if (!run_data->connected_server_flag) {
        if (doDelayMillisTime(cfg_data.connect_server_interval, &run_data->connect_server_timestamp, false) || (run_data->connect_server_timestamp == 0)) {
            esp_mqtt_client_start(run_data->client);
            run_data->connect_server_timestamp = millis();
        }
        return;
    }

    if (run_data->connected_server_flag && run_data->send_flag && (doDelayMillisTime(cfg_data.mqtt_send_interval, &run_data->mqtt_send_timestamp, false) || run_data->mqtt_send_timestamp == 0)) {
        send_love();
        run_data->show_ok_times = 1;
        run_data->mqtt_send_timestamp = millis();
        run_data->send_flag = false;
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
            case WIFI_HANDLE:
                DPRINTF("waitwifi callback");
                run_data->wait_wifi_flag = false;
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