//
// Created by Administrator on 2021/10/26.
//

#include <banner.h>

void show_banner()
{
    printf("当前版本：%s \n", VERSION);
    FILE *fp = fopen("banner.txt", "r");
    if (fp == nullptr)
    {
        perror("找不到banner文件！\n");
        return;
    }
    char buff[1024];
    while (!feof(fp))
    {
        if (fgets(buff, 500, fp) == nullptr)
        {
            continue;
        }
        printf("%s", buff);
    }
    printf("\n");
    fclose(fp);
}