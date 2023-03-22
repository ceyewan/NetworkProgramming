#include <assert.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

struct conf_opts {
    char CGIRoot[128];          // CGI 根路径
    char DeFaultFile[128];      // 默认文件名称
    char DocumentRoot[128];     // 根文件路径
    char ConfigFile[128];       // 配置文件路径和名称
    int ListenPort;             // 侦听端口
    int MaxClient;              // 最大客户端数量
    int TimeOut;                // 超时时间
} opts;

static struct option long_options[] = {  
    {"CGIRoot", required_argument,NULL, 'c'},
    {"DeFaultFile", required_argument,NULL, 'd'},
    {"DocumentRoot", required_argument,NULL,'o'},
    {"ConfigFile", required_argument,NULL, 'f'},
    {"ListenPort", required_argument,NULL, 'l'},
    {"MaxClient", required_argument,NULL, 'm'},
    {"TimeOut", required_argument,NULL,'t'},
    {"Help", no_argument, NULL, 'h'},
}; 

void Init() {
    memset(&opts, 0, sizeof(opts));
}
void ShowHelp() {
    printf("Options:\n \
    -c --CGIRoot        CGI根路径\n \
    -d --DeFaultFile    默认文件名称\n \
    -o --DocumentRoot   根文件路径\n \
    -f --ConfigFile     配置文件路径和名称\n \
    -l --ListenPort     侦听端口\n \
    -m --MaxClient      最大客户端数量\n \
    -t --TimeOut        超时时间\n \
    -h --help           帮助\n");
}

void ShowOpts() {
    printf("Opts:\n \
    CGIRoot        %s\n \
    DeFaultFile    %s\n \
    DocumentRoot   %s\n \
    ConfigFile     %s\n \
    ListenPort     %d\n \
    MaxClient      %d\n \
    TimeOut        %d\n", \
    opts.CGIRoot, opts.DeFaultFile, opts.DocumentRoot, opts.ConfigFile, \
    opts.ListenPort, opts.MaxClient, opts.TimeOut);
}

void SwitchOpt(int opt) {
    switch (opt) {
        case 'c' :
            assert(optarg != NULL); strcpy(opts.CGIRoot, optarg);
            break;
        case 'd' :
            assert(optarg != NULL); strcpy(opts.DeFaultFile, optarg);
            break;
        case 'o' :
            assert(optarg != NULL); strcpy(opts.DocumentRoot, optarg);
            break;
        case 'f' :
            assert(optarg != NULL); strcpy(opts.ConfigFile, optarg);
            break;
        case 'l' :
            assert(optarg != NULL); opts.ListenPort = atoi(optarg);
            break;
        case 'm' :
            assert(optarg != NULL); opts.MaxClient = atoi(optarg);
            break;
        case 't' :
            assert(optarg != NULL); opts.TimeOut = atoi(optarg);
            break;
        case 'h' :
            ShowHelp();
            exit(-1);
        default:
            printf("Please input legal flag!\n");
            ShowHelp();
            exit(-1);
    }
}

void SwitchKey(char *key, char *value) {
    if (opts.CGIRoot[0] == '\0' && strcmp("CGIRoot", key) == 0) {
        strcpy(opts.CGIRoot, value);
    }
    if (opts.DeFaultFile[0] == '\0' && strcmp("DeFaultFile", key) == 0) {
        strcpy(opts.DeFaultFile, value);
    }
    if (opts.DocumentRoot[0] == '\0' && strcmp("DocumentRoot", key) == 0) {
        strcpy(opts.DocumentRoot, value);
    }
    if (opts.ConfigFile[0] == '\0' && strcmp("ConfigFile", key) == 0) {
        strcpy(opts.ConfigFile, value);
    }
    if (opts.ListenPort == 0 && strcmp("ListenPort", key) == 0) {
        opts.ListenPort = atoi(value);
    }
    if (opts.MaxClient == 0 && strcmp("MaxClient", key) == 0) {
        opts.MaxClient = atoi(value);
    }
    if (opts.TimeOut == 0 && strcmp("TimeOut", key) == 0) {
        opts.TimeOut = atoi(value);
    }
}

void ReadConf(int fd) {
    int count = 0;
    while (1) { 
        int flag = 1;
        char *buf = (char *)malloc(sizeof(char) * 1024);
        lseek(fd, count, SEEK_SET);
        int size = read(fd, buf, 1024);
        if (size == 0) break;
        if (buf[0] == '#') flag = 0;
        char *key = NULL, *value = NULL;
        for (int i = 0; i < size; i++) {
            if (buf[i] == ' ' && key == NULL) {
                key = buf;
                value = buf + i + 3;
                buf[i] = '\0';
            }
            if (buf[i] == '\n') {
                buf[i] = '\0';
                count += i + 1;
                break;
            }
        }  
        if (flag) SwitchKey(key, value);
        free(buf);
    }
}

void DefaultConf() {
    if (opts.CGIRoot[0] == '\0') {
        strcpy(opts.CGIRoot, "defaultCGIRoot");
    }
    if (opts.DeFaultFile[0] == '\0') {
        strcpy(opts.DeFaultFile, "defaultDeFaultFile");
    }
    if (opts.DocumentRoot[0] == '\0') {
        strcpy(opts.DocumentRoot, "defaultDocumentRoot");
    }
    if (opts.ConfigFile[0] == '\0') {
        strcpy(opts.ConfigFile, "defaultConfigFile");
    }
    if (opts.ListenPort == 0) {
        opts.ListenPort = 88;
    }
    if (opts.MaxClient == 0) {
        opts.MaxClient = 520;
    }
    if (opts.TimeOut == 0) {
        opts.TimeOut = 1314;
    }
}

int main(int argc, char *argv[]) {
    int opt;
    while((opt = getopt_long(argc, argv, "c:d:o:f:l:m:t:h", long_options, NULL)) != -1) {
        SwitchOpt(opt);
    }
    char *filename = (opts.ConfigFile[0] != '\0') ? opts.ConfigFile : "./SHTTPD.conf";
    int fd = open(filename, O_RDONLY);  
    ReadConf(fd);
    DefaultConf();
    ShowOpts();
    return 0;
}

/*
➜ gcc labA.c
➜ ./a.out --CGIRoot CGIFromLongOption -d DefaultFileFromShortOption -l 888
Opts:
     CGIRoot        CGIFromLongOption               长选项
     DeFaultFile    DefaultFileFromShortOption      短选项
     DocumentRoot   /usr/local/var/www/             从配置文件中读取
     ConfigFile     defaultConfigFile               默认值
     ListenPort     888                             短选项
     MaxClient      4                               从配置文件中读取
     TimeOut        1314                            默认值
*/
