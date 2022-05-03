#include "biubiubiu.h"
#include "biubiubiu_gui.h"
#include "sys/app_controller.h"
#include "common.h"


#define BIUBIUBIU_APP_NAME "Biubiubiu"

// 相册的持久化配置
#define BIUBIUBIU_CONFIG_PATH "/biubiubiu.cfg"
struct BIU_Config
{
    unsigned char server_ip[4];
    unsigned int  server_port;
};

static void write_config(BIU_Config *cfg)
{
    char ServerIP[4];
    unsigned int ServerPort;
    snprintf(ServerIP, 4, "%x\n", cfg->server_ip);
    snprintf((char*)&ServerPort, 4, "%x\n", cfg->server_port);
    String w_data;
    w_data += ServerIP;
    w_data += ServerPort;
    g_flashCfg.writeFile(BIUBIUBIU_CONFIG_PATH, w_data.c_str());
}

static void read_config(BIU_Config *cfg)
{
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(BIUBIUBIU_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0) {
        cfg->server_ip[0] = 0;
        cfg->server_ip[1] = 0;
        cfg->server_ip[2] = 0;
        cfg->server_ip[3] = 0;
        cfg->server_port = 0;
        write_config(cfg);
    } else {
        char *param;
        analyseParam(info, 1, &param);
        cfg->server_ip[0] = *param;
        analyseParam(info+1, 1, &param);
        cfg->server_ip[1] = *param;
        analyseParam(info+2, 1, &param);
        cfg->server_ip[2] = *param;
        analyseParam(info+3, 1, &param);
        cfg->server_ip[3] = *param;
        analyseParam(info+4, 1, &param);
        cfg->server_port = atoi((const char *)param);
    }
}


struct BiubiubiuAppRunData {
    unsigned char server_ip[4];
    unsigned int  server_port;

};

static BIU_Config cfg_data;
static struct BiubiubiuAppRunData *run_data = NULL;

static File_Info *get_next_file(File_Info *p_cur_file, int direction) {
    // 得到 p_cur_file 的下一个 类型为FILE_TYPE_FILE 的文件（即下一个非文件夹文件）
    if (!p_cur_file) {
        return NULL;
    }

    File_Info *pfile = direction == 1 ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file)
    {
        if (FILE_TYPE_FILE == pfile->file_type) {
            break;
        }
        pfile = direction == 1 ? pfile->next_node : pfile->front_node;
    }
    
    return pfile;
}

static int biubiubiu_init(void) {
    biubiubiu_gui_init();

    run_data = (BiubiubiuAppRunData *)calloc(1, sizeof(BiubiubiuAppRunData));

}

static void biubiubiu_process(AppController *sys,
                            const ImuAction *act_info) {

    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    display_biubiubiu();
    delay(30);
}

static int biubiubiu_exit_callback(void *param)
{
    biubiubiu_gui_del();

    free(run_data);
    run_data = NULL;
}

static void biubiubiu_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info) {
    switch (type)
    {
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