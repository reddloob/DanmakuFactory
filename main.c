/* 
Copyright 2019-2020 hkm(github:hihkm)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#ifdef _MSC_VER

#include <io.h>
#define F_OK 0
#define W_OK 2
#define R_OK 4

#else

#include <unistd.h>

#endif

#include <math.h>
#include "CDanmakuFactory.h"
#include "Define/CLIDef.h"

void printConfigInfo(CONFIG config);
void printHelpInfo();
int getArgNum(int argc, char **argv, const int optionIndex);
char *getFormat(char *outFormat, const char *const fileName, int maxLen);
char *getPath(char *outPath, const char *const fileName, int maxLen);
double getArgVal(int argc, char **argv, const int optIndex, const char *const optName, const double errorReturnValue);

void toPause(BOOL skip);
BOOL isContinue(BOOL skip);

static CONFIG defauleConfig =
{
    1920,           /* 分辨率宽 */
    1080,           /* 分辨率高 */ 
    12.0,           /* 滚动时间 */ 
    5.0,            /* 固定时间 */ 
    0,              /* 弹幕密度 */
    40,             /* 字号 */
                    /* 字体 */
    "Microsoft YaHei", 
    128,            /* 不透明度 */ 
    0,              /* 描边 */ 
    1,              /* 阴影 */
    FALSE,          /* 是否加粗 */ 
    1.00,           /* 显示区域 */ 
    1.00,           /* 滚动区域 */
    TRUE,           /* 是否保存屏蔽部分 */ 
    0,              /* 屏蔽模式 */ 
    0,              /* 统计模式 */
};

int main(int argc, char **argv)
{
    FINFO outfile;
    FINFO *infile = NULL;
    int infileNum = 0;
    int argCnt = 1;
    int cnt;
    BOOL showConfig = FALSE;
    BOOL saveConfig = FALSE;
    BOOL configFileErr = FALSE;
    BOOL ignoreWarnings = FALSE;
    CONFIG config;
    char tempStr[MAX_TEXT_LENGTH], *tempPtr;
    char programPath[MAX_TEXT_LENGTH];
    char configFilePath[MAX_TEXT_LENGTH];
    outfile.isSet = FALSE;

    /* 打印程序版本信息 */
    printf("DanamkuFactory "VERSION" "EDITION" by hkm (hkm@tikm.org)\n"
           "https://github.com/hihkm/DanmakuFactory\n"
          );
    
    /* 获取配置文件路径 */
    getPath(programPath, argv[0], MAX_TEXT_LENGTH);
    sprintf_s(configFilePath, MAX_TEXT_LENGTH, "%s%s", programPath, "\\"CONFIG_FILE_NAME);
    configFilePath[MAX_TEXT_LENGTH - 1] = '\0';
    if (strstr(configFilePath, CONFIG_FILE_NAME) == NULL)
    {
        printf("\nWARNING"
               "\nOut of buffer.");
        printf("\nNOTE"
               "\nFail to get config file path, because the path is too long\n");
        
        configFileErr = TRUE;
        toPause(ignoreWarnings);
    }
    
    /* 解析参数 */
    if (argc <= 1)
    {
        printHelpInfo();
        return 0;
    }
    else
    {
        /* 读配置文件 */
        config = readConfig(configFilePath, defauleConfig);
        
        /* 遍历参数 */ 
        while (argCnt < argc)
        {
            if (!strcmp("-h", argv[argCnt]) || !strcmp("--help", argv[argCnt]))
            {
                printHelpInfo();
                return 0;
            }
            else if (!strcmp("-c", argv[argCnt]) || !strcmp("--config", argv[argCnt]))
            {
                showConfig = TRUE;
                argCnt += 1;
            }
            else if (!strcmp("--save", argv[argCnt]))
            {
                saveConfig = TRUE;
                showConfig = TRUE;
                argCnt += 1;
            }
            else if (!strcmp("-o", argv[argCnt]) || !strcmp("--output", argv[argCnt]))
            {/* 输出文件名 */
                switch (getArgNum(argc, argv, argCnt))
                {
                    case 0:
                        fprintf(stderr, "\nERROR"
                                        "\nOutput file must be specified\n");
                        return 0;
                        break;
                    case 1:
                        strSafeCopy(outfile.fileName, argv[argCnt+1], FILENAME_LEN);
                        getFormat(outfile.template, outfile.fileName, FORMAT_LEN);
                        argCnt += 2;
                        break;
                    case 2:
                        strSafeCopy(outfile.template,   argv[argCnt+1], FORMAT_LEN);
                        strSafeCopy(outfile.fileName, argv[argCnt+2], FILENAME_LEN);
                        argCnt += 3;
                        break;
                    default:
                        /* 传入未知参数丢给下一轮获取选项时报错 */
                        argCnt += 3;
                        break;
                }
                
                deQuotMarks(outfile.fileName);
                deQuotMarks(outfile.template);
                outfile.isSet = TRUE;
                
                /* 合法性检查 */ 
                if (!ISFORMAT(outfile.template))
                {
                    fprintf(stderr, "\nERROR"
                                    "\nUnknow format %s\n", outfile.template);
                    
                    return 0;
                }
            }
            else if (!strcmp("-i", argv[argCnt]) || !strcmp("--input", argv[argCnt]))
            {/* 输入文件名 */ 
                int num = getArgNum(argc, argv, argCnt);
                for (cnt = 0; cnt < num; cnt++)
                {
                    if ((infile = (FINFO *)realloc(infile, (infileNum+1) * sizeof(FINFO))) == NULL)
                    {
                        fprintf(stderr, "\nERROR"
                                        "\nOut of memory\n");
                        return 0;
                    }
                    
                    strSafeCopy(tempStr, argv[argCnt + cnt + 1], MAX_TEXT_LENGTH);
                    deQuotMarks(tempStr);

                    if (ISFORMAT(tempStr))
                    {
                        strcpy(infile[infileNum].template, tempStr);
                    }
                    else
                    {
                        sprintf_s(infile[infileNum].template, FILENAME_LEN, "%stemplates\\%s.txt", programPath, tempStr);
                        (infile[infileNum].template)[FILENAME_LEN-1] = '\0';
                    }

                    if (ISFORMAT(tempStr) || access(infile[infileNum].template, F_OK) == 0)
                    {
                        cnt++;
                        if (cnt >= num)
                        {
                            fprintf(stderr, "\nERROR"
                                            "\nTemplate must be followed by a filename\n");
                            return 0;
                        }
                        
                        strcpy(infile[infileNum].fileName, argv[argCnt + cnt + 1]);
                    }
                    else
                    {
                        strcpy(infile[infileNum].fileName, argv[argCnt + cnt + 1]);
                        getFormat(infile[infileNum].template, infile[infileNum].fileName, FORMAT_LEN);

                        /* 合法性检查 */
                        if (!ISFORMAT(infile[infileNum].template))
                        {
                            fprintf(stderr, "\nERROR"
                                            "\nUnknow Template %s\n", infile[infileNum].template);
                            return 0;
                        }
                    }
                    
                    infile[infileNum].timeShift = 0.00;
                    deQuotMarks(infile[infileNum].fileName);

                    infileNum++;
                }
                
                argCnt += num + 1;
            }
            else if (!strcmp("-x", argv[argCnt]) || !strcmp("--resx", argv[argCnt]))
            {/* 分辨率宽 */
                double returnValue = getArgVal(argc, argv, argCnt, "Resolution width", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.resx = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-y", argv[argCnt]) || !strcmp("--resy", argv[argCnt]))
            {/* 分辨率高 */
                double returnValue = getArgVal(argc, argv, argCnt, "Resolution height", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.resy = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-s", argv[argCnt]) || !strcmp("--scrolltime", argv[argCnt]))
            {/* 滚动时间 */
                double returnValue = getArgVal(argc, argv, argCnt, "Scroll time", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.scrolltime = (float)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-f", argv[argCnt]) || !strcmp("--fixtime", argv[argCnt]))
            {/* 固定时间 */
                double returnValue = getArgVal(argc, argv, argCnt, "Fix time", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.fixtime = (float)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-d", argv[argCnt]) || !strcmp("--density", argv[argCnt]))
            {/* 弹幕密度 */
                double returnValue = getArgVal(argc, argv, argCnt, "Density", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.density = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-S", argv[argCnt]) || !strcmp("--fontsize", argv[argCnt]))
            {/* 字号 */
                double returnValue = getArgVal(argc, argv, argCnt, "Fontsize", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.fontsize = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-N", argv[argCnt]) || !strcmp("--fontname", argv[argCnt]))
            {/* 字号 */
                switch (getArgNum(argc, argv, argCnt))
                {
                    case 0:
                        fprintf(stderr, "\nERROR"
                                        "\nFontname must be specified\n");
                        return 0;
                        break;
                    case 1:
                        strSafeCopy(config.fontname, argv[argCnt+1], FONTNAME_LEN);
                        deQuotMarks(config.fontname);
                        break;
                    default:
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid argument %s\n", argv[argCnt+2]);
                        return 0;
                        break;
                }

                argCnt += 2; 
            }
            else if (!strcmp("-O", argv[argCnt]) || !strcmp("--opacity", argv[argCnt]))
            {/* 不透明度 */
                double returnValue = getArgVal(argc, argv, argCnt, "Opacity", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.opacity = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-L", argv[argCnt]) || !strcmp("--outline", argv[argCnt]))
            {/* 描边 */
                double returnValue = getArgVal(argc, argv, argCnt, "Outline", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.outline = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-D", argv[argCnt]) || !strcmp("--shadow", argv[argCnt]))
            {/* 阴影 */
                double returnValue = getArgVal(argc, argv, argCnt, "Shadow", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.shadow = (int)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-B", argv[argCnt]) || !strcmp("--bold", argv[argCnt]))
            {/* 是否保存屏蔽部分 */
                switch (getArgNum(argc, argv, argCnt))
                {
                    case 0:
                        fprintf(stderr, "\nERROR"
                                        "\nBold must be specified\n");
                        return 0;
                        break;
                    case 1:
                        break;
                    default:
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid argument %s\n", argv[argCnt+2]);
                        return 0; 
                        break;
                }
                
                deQuotMarks(argv[argCnt+1]);
                toLower(NULL, argv[argCnt+1]);
                if (!strcmp("true", argv[argCnt+1]))
                {
                    config.bold = TRUE;
                }
                else if (!strcmp("false", argv[argCnt+1]))
                {
                    config.bold = FALSE;
                }
                else
                {
                    fprintf(stderr, "\nERROR"
                                    "\nInvalid value %s\n", argv[argCnt+1]);
                    return 0;
                }
                
                argCnt += 2; 
            }
            else if (!strcmp("--displayarea", argv[argCnt]))
            {/* 显示区域 */
                double returnValue = getArgVal(argc, argv, argCnt, "Display area", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.displayarea = (float)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("--scrollarea", argv[argCnt]))
            {/* 滚动区域 */
                double returnValue = getArgVal(argc, argv, argCnt, "Scroll area", -256.00);
                if (fabs(returnValue - (-256.0)) < EPS)
                {
                    return 0;
                }
                config.scrollarea = (float)returnValue;
                
                argCnt += 2; 
            }
            else if (!strcmp("-b", argv[argCnt]) || !strcmp("--blockmode", argv[argCnt]))
            {/* 屏蔽模式 */
                switch (getArgNum(argc, argv, argCnt))
                {
                    case 0:
                        fprintf(stderr, "\nERROR"
                                        "\nBlockmode must be specified\n");
                        return 0;
                        break;
                    case 1:
                        break;
                    default:
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid argument %s\n", argv[argCnt+2]);
                        return 0; 
                        break;
                }
                
                config.blockmode = 0;
                if (!strcmp("null", argv[argCnt+1]))
                {
                    argCnt += 2;
                    continue;
                }
                
                cnt = 0;
                char *argPtr = argv[argCnt+1];
                while (*argPtr != '\0')
                {
                    tempPtr = tempStr;
                    while (*argPtr != '\0' && *argPtr != '-' && cnt < MAX_TEXT_LENGTH)
                    {
                        *tempPtr = *argPtr;
                        tempPtr++;
                        argPtr++;
                        cnt++;
                    }
                    if (*argPtr == '-')
                    {
                        argPtr++;
                    }
                    *tempPtr = '\0';
                    
                    deQuotMarks(tempStr);
                    toLower(NULL, tempStr);
                    if (!strcmp("l2r", tempStr))
                    {
                        config.blockmode += BLK_L2R;
                    }
                    else if (!strcmp("r2l", tempStr))
                    {
                        config.blockmode += BLK_R2L;
                    }
                    else if (!strcmp("top", tempStr))
                    {
                        config.blockmode += BLK_TOP;
                    }
                    else if (!strcmp("bottom", tempStr))
                    {
                        config.blockmode += BLK_BOTTOM;
                    }
                    else if (!strcmp("special", tempStr))
                    {
                        config.blockmode += BLK_SPECIAL;
                    }
                    else if (!strcmp("color", tempStr) || !strcmp("colour", tempStr))
                    {
                        config.blockmode += BLK_COLOR;
                    }
                    else if (!strcmp("repeat", tempStr))
                    {
                        config.blockmode += BLK_REPEAT;
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid type-name %s\n", tempStr);
                        return 0;
                    }
                }
                
                argCnt += 2;
            }
            else if (!strcmp("--statmode", argv[argCnt]))
            {/* 统计模式 */
                switch (getArgNum(argc, argv, argCnt))
                {
                    case 0:
                        fprintf(stderr, "\nERROR"
                                        "\nStatmode must be specified\n");
                        return 0;
                        break;
                    case 1:
                        break;
                    default:
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid argument %s\n", argv[argCnt+2]);
                        return 0; 
                        break;
                }
                
                config.statmode = 0;
                if (!strcmp("null", argv[argCnt+1]))
                {
                    argCnt += 2;
                    continue;
                }
                
                cnt = 0;
                char *argPtr = argv[argCnt+1];
                while (*argPtr != '\0')
                {
                    tempPtr = tempStr;
                    while (*argPtr != '\0' && *argPtr != '-' && cnt < MAX_TEXT_LENGTH)
                    {/* 拷贝 - 之前的字符 */
                        *tempPtr = *argPtr;
                        tempPtr++;
                        argPtr++;
                        cnt++;
                    }
                    if (*argPtr == '-')
                    {
                        argPtr++;
                    }
                    *tempPtr = '\0';
                    /* 字符串比对 */
                    deQuotMarks(tempStr);
                    toLower(NULL, tempStr);
                    if (!strcmp("table", tempStr))
                    {
                        config.statmode += TABLE;
                    }
                    else if (!strcmp("histogram", tempStr))
                    {
                        config.statmode += HISTOGRAM;
                    }
                    else
                    {
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid type-name %s\n", tempStr);
                        return 0; 
                    }
                }
                
                argCnt += 2; 
            }
            else if (!strcmp("--saveblocked", argv[argCnt]))
            {/* 是否保存屏蔽部分 */
                switch (getArgNum(argc, argv, argCnt))
                {
                    case 0:
                        fprintf(stderr, "\nERROR"
                                        "\nSaveblocked must be specified\n");
                        return 0;
                        break;
                    case 1:
                        break;
                    default:
                        fprintf(stderr, "\nERROR"
                                        "\nInvalid argument %s\n", argv[argCnt+2]);
                        return 0; 
                        break;
                }
                
                deQuotMarks(argv[argCnt+1]);
                toLower(NULL, argv[argCnt+1]);
                if (!strcmp("true", argv[argCnt+1]))
                {
                    config.saveBlockedPart = TRUE;
                }
                else if (!strcmp("false", argv[argCnt+1]))
                {
                    config.saveBlockedPart = FALSE;
                }
                else
                {
                    fprintf(stderr, "\nERROR"
                                    "\nInvalid value %s\n", argv[argCnt+1]);
                    return 0;
                }
                
                argCnt += 2; 
            }
            else if (!strcmp("-t", argv[argCnt]) || !strcmp("--timeshift", argv[argCnt]))
            {
                argCnt++;
                while (argCnt < argc && ISNUMBERIC(argv[argCnt]))
                {
                    argCnt++;
                }
            }
            else if (!(strcmp("--ignore-warnings", argv[argCnt])))
            {// 跳过全部警告
                ignoreWarnings = TRUE;
                argCnt++;
            }
#ifdef GUI_SUPPORT
            /* 为GUI提供程序版本确认 如果是错误的版本将会报非法参数错误 */
            else if (!(strcmp("--check-version-"VERSION, argv[argCnt])))
            {
                argCnt++;
            }
#endif
            else
            {
                fprintf(stderr, "\nERROR"
                                "\nInvalid argument %s\n", argv[argCnt]);
                return 0;
            }
        }
        
        /* 寻找并解析时间平移量 */ 
        for (argCnt = 0; argCnt < argc; argCnt++)
        {
            if (!strcmp("-t", argv[argCnt]) || !strcmp("--timeshift", argv[argCnt]))
            {
                argCnt++;
                for (cnt = 0; cnt < infileNum; cnt++)
                {
                    if (argCnt >= argc || !ISNUMBERIC(argv[argCnt]))
                    {
                        fprintf(stderr, "\nERROR"
                                        "\nToo few values in option timeshift(-t, --timeshift)");
                        fprintf(stderr, "\nNOTE"
                                        "\n%d filenames are provided, but only %d timeshift value.\n",
                               infileNum, cnt);
                        return 0;
                    }
                    
                    strSafeCopy(tempStr, argv[argCnt], MAX_TEXT_LENGTH);
                    deQuotMarks(tempStr);
                    infile[cnt].timeShift = atof(tempStr);
                    
                    argCnt++;
                }
                
                /* 有更多的数字意味着语法错误 */
                if (argCnt < argc && ISNUMBERIC(argv[argCnt]))
                {
                    fprintf(stderr, "\nERROR"
                                    "\nToo many values in option timeshift(-t, --timeshift)");
                    fprintf(stderr, "\nNOTE"
                                    "\nOnly %d filename(s) are provided, but more than %d timeshift values.\n",
                           infileNum, infileNum);
                    return 0;
                }
                
                /* 解析完毕即跳出循环 */
                break;
            }
        }  
    }
    
    /* 显示配置信息 */
    if (showConfig == TRUE)
    {
        printConfigInfo(config);
    }

    /* 配置项合法性检查 */
    {
        /* 分辨率宽 */
        if (config.resx <= 0)
        {
            fprintf(stderr, "\nERROR"
                            "\nResolution width must be an integer greater than 0");
            return 0;
        }
        /* 分辨率高 */
        if (config.resy <= 0)
        {
            fprintf(stderr, "\nERROR"
                            "\nResolution height must be an integer greater than 0");
            return 0;
        }
        /* 滚动时间 */
        if (config.scrolltime < EPS)
        {
            fprintf(stderr, "\nERROR"
                            "\nScroll time must be a real number greater than 0.00");
            return 0;
        }
        /* 固定时间 */
        if (config.fixtime < EPS)
        {
            fprintf(stderr, "\nERROR"
                            "\nFix time must be a real number greater than 0.00");
            return 0;
        }
        /* 密度 */
        if (config.density < -1)
        {
            fprintf(stderr, "\nERROR"
                            "\nDensity must be an integer greater than or equal to -1");
            return 0;
        }
        /* 字号 */
        if (config.fontsize <= 0)
        {
            fprintf(stderr, "\nERROR"
                            "\nFontsize must be an integer greater than 0");
            return 0;
        }
        /* 不透明度 */
        if (config.opacity <= 0 || config.opacity > 255)
        {
            fprintf(stderr, "\nERROR"
                            "\nOpacity must be an integer greater than 0 and less than or equal to 255");
            return 0;
        }
        /* 描边 */
        if (config.outline < 0 || config.outline > 4)
        {
            fprintf(stderr, "\nERROR"
                            "\nOutline must be an integer greater than or equal to 0 and less than or equal to 4");
            return 0;
        }
        /* 阴影 */
        if (config.shadow < 0 || config.shadow > 4)
        {
            fprintf(stderr, "\nERROR"
                            "\nShadow must be an integer greater than or equal to 0 and less than or equal to 4");
            return 0;
        }
        /* 显示区域 */
        if (config.displayarea < EPS || config.displayarea > 1.0 + EPS)
        {
            fprintf(stderr, "\nERROR"
                            "\nDisplay area must be a real number greater than 0.0 and less than or equal to 1.0");
            return 0;
        }
        /* 滚动区域 */
        if (config.scrollarea < EPS || config.scrollarea > 1.0 + EPS)
        {
            fprintf(stderr, "\nERROR"
                            "\nScroll area must be a real number greater than 0.0 and less than or equal to 1.0");
            return 0;
        }
        /* 字体 */
        tempPtr = config.fontname;
        while (*tempPtr != '\0')
        {/* 验证是否全部为可打印的ascii字符 */
            if (*tempPtr < 0x20 || *tempPtr > 0x7e)
            {/* ascii 非可打印字符范围 */
                printf("\nWARNING"
                    "\nSome characters of fontname are non-ASCII characters, which may cause the garbled problem\n");
                toPause(ignoreWarnings);
                break;
            }

            tempPtr++;
        }
    }
    
    /* 保存配置文件 */
    if (saveConfig == TRUE && configFileErr == FALSE)
    {
        if (writeConfig(configFilePath, config) == TRUE)
        {
            printf("\nConfiguration file had been saved successfully!\n");
        }
        else
        {
            printf("\nWARNING"
                   "\nFailed to write the configuration file!\n");
            
            toPause(ignoreWarnings);
        }
    }
    
    if (outfile.isSet == FALSE && infileNum == 0)
    {
        return 0;
    }

    /* 显示文件信息 */
    if (outfile.isSet == FALSE)
    {
        fprintf(stderr, "\nERROR"
                        "\nOutput file must be specified");
        fprintf(stderr, "\nNOTE"
                        "\nUse -o or --output to specify.\n");
        return 0;
    }
    if (infileNum == 0)
    {
        fprintf(stderr, "\nERROR"
                        "\nInput file must be specified");
        fprintf(stderr, "\nNOTE"
                        "\nUse -i or --input to specify.\n");
        return 0;
    }
    
    printf("\nInput file(s):");
    printf("\nNumber|Template     |TimeShift|FileName\n");
    for (cnt = 0; cnt < infileNum; cnt++)
    {
        strSafeCopy(tempStr, infile[cnt].template, MAX_TEXT_LENGTH);
        if (strlen(infile[cnt].template) > 10)
        {
            strcpy(tempStr + 10, "...");
            
        }
        printf("%6d|%-13s|%8.3fs|%s\n", cnt+1, tempStr, infile[cnt].timeShift, infile[cnt].fileName);
    }
    
    printf("\nOutput file:");
    printf("\nFormat|FileName");
    printf("\n%6s|%s\n", outfile.template, outfile.fileName);
    
    /* 读取文件 */
    printf("\nLoading files...\n");
    
    int returnValue;
    STATUS status;
    DANMAKU *danmakuPool = NULL;
    
    status.totalNum = 0;
    for (cnt = 0; cnt < infileNum; cnt++)
    {
        printf("Loading file %s\n", infile[cnt].fileName);
        /* 检查文件是否存在 */
        if (access(infile[cnt].fileName, F_OK) != 0)
        {
            fprintf(stderr, "\nERROR"
                            "\nNo such file.\n");
            return 0;
        }

        /* 权限检查 */
        if (access(infile[cnt].fileName, R_OK) != 0)
        {
            fprintf(stderr, "\nERROR"
                            "\nPermission denied.\n");
            return 0;
        }
        
        if (!strcmp("xml", infile[cnt].template))
        {
            returnValue = readXml(infile[cnt].fileName, &danmakuPool, "a", infile[cnt].timeShift, &status);
            
            switch (returnValue)
            {
                case 0:
                    break;
                case 1:
                    fprintf(stderr, "\nERROR [code rx%d]"
                                    "\nFailed to open file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
                case 2:
                case 3:
                case 4:
                    fprintf(stderr, "\nERROR [code rx%d]"
                                    "\nFailed to read file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
                case 5:
                case 6:
                case 7:
                    fprintf(stderr, "\nERROR [code rx%d]"
                                    "\nOut of memory\n",
                           returnValue
                          );
                    return 0;
                    break;
                case 8:
                    printf("\nWARNING [code rx%d]"
                           "\nIncorrect file format, contiune or exit?",
                           returnValue
                          );
                    printf("\nNOTE"
                           "\nCould not load file %s as a xml file.\n", infile[cnt].fileName);
                    if (isContinue(ignoreWarnings) == FALSE)
                    {
                        return 0;
                    }
                    break;
                default:
                    fprintf(stderr, "\nERROR [code rx%d]"
                                    "\nUndefined Error\n",
                           returnValue
                          );
                    return 0;
                    break;
            }
        }
        else if (!strcmp("json", infile[cnt].template))
        {
            returnValue = readJson(infile[cnt].fileName, &danmakuPool, "a", infile[cnt].timeShift, &status);
            
            switch (returnValue)
            {
                case 0:
                    break;
                case 1:
                    fprintf(stderr, "\nERROR [code rj%d]"
                                    "\nFailed to open file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
                case 2:
                case 3:
                case 4:
                    fprintf(stderr, "\nERROR [code rj%d]"
                                    "\nFailed to read file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
                case 5:
                case 6:
                    fprintf(stderr, "\nERROR [code rj%d]"
                                    "\nOut of memory\n",
                           returnValue
                          );
                    return 0;
                    break;
                case 7:
                    printf("\nWARNING [code rj%d]"
                           "\nIncorrect file format, contiune or exit?",
                           returnValue
                          );
                    printf("\nNOTE"
                           "\nCould not load file %s as a json file.\n", infile[cnt].fileName);
                    if (isContinue(ignoreWarnings) == FALSE)
                    {
                        return 0;
                    }
                    break;
                default:
                    fprintf(stderr, "\nERROR [code rj%d]"
                                    "\nUndefined Error\n",
                           returnValue
                          );
                    return 0;
                    break;
            }
        }
        else if (!strcmp("ass", infile[cnt].template))
        {
            returnValue = readAss(infile[cnt].fileName, &danmakuPool, "a", NULL, infile[cnt].timeShift, &status);
            /* 解析十位错误码 */
            switch (returnValue / 10)
            {
                case 0:
                    break;
                case 1:
                    fprintf(stderr, "\nERROR [code ra%d]"
                                    "\nFailed to open file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
                case 2:
                case 3:
                case 4:
                    fprintf(stderr, "\nERROR [code ra%d]"
                                    "\nOut of memory\n",
                           returnValue
                          );
                    return 0;
                    break;
                default:
                    fprintf(stderr, "\nERROR [code ra%d]"
                                    "\nUndefined Error\n",
                           returnValue
                          );
                    return 0;
                    break;
            }
            /* 解析个位 */
            switch (returnValue % 10)
            {
                case 0:
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    fprintf(stderr, "\nERROR [code ra%d]"
                                    "\nOut of memory\n",
                           returnValue
                          );
                    return 0;
                    break;
                default:
                    fprintf(stderr, "\nERROR [code ra%d]"
                                    "\nUndefined Error\n",
                           returnValue
                          );
                    return 0;
                    break;
            }
        }
        else
        {/* 使用用户自定义模板 */
            readAss(infile[cnt].fileName, &danmakuPool, "a", NULL, infile[cnt].timeShift, &status);
            returnValue = readTemplateFile(infile[cnt].fileName, infile[cnt].template, &danmakuPool, "a", infile[cnt].timeShift,
                             &status, tempStr, MAX_TEXT_LENGTH);
            
            if (returnValue > 0)
            {
                switch (returnValue)
                {
                case 1:
                case 3:
                    /* 程序内部错误 */
                    fprintf(stderr, "\nERROR [code rt%d]"
                                    "\nFailed to open template file %s\n",
                            returnValue, infile[cnt].template
                            );
                    return 0;
                    break;
                case 2:
                case 4:
                case 5:
                    /* 程序内部错误 */
                    fprintf(stderr, "\nERROR [code rt%d]"
                                    "\nOut of memory\n",
                            returnValue
                            );
                    return 0;
                    break;
                default:
                    /* 程序内部错误 */
                    fprintf(stderr, "\nERROR [code rt%d]"
                                    "\nUndefined Error\n",
                            returnValue
                            );
                    return 0;
                    break;
                }
            }
            else if (returnValue < 0)
            {
                /* 模板文件语法错误 */
                /* 程序内部错误 */
                fprintf(stderr, "\nERROR [code rt%d]"
                                "\nFormat File Syntax error"
                                "\nFilename: %s"
                                "\n%s",
                        returnValue, infile[cnt].template, tempStr
                        );
                printf("\nexit...");
                return 0;
            }
        }
        
    }

    /* 屏蔽 */
    blockByType(danmakuPool, config.blockmode, NULL);
    
    /* 读完成提示 */
    printf("\nFile Loading Complete");
    if (status.totalNum != 0)
    {
        printf(". Danmaku Pool: %d\n", status.totalNum);
    }
    else
    {
        printf(" and nothing was read.\nexit...\n");
        return 0;
    }
    
    /* 排序 */
    printf ("\nSorting...\n");
    returnValue = sortList(&danmakuPool, NULL);
    switch (returnValue)
    {
        case 0:
            break;
        case 2:
            fprintf(stderr, "\nERROR [code s%d]"
                            "\nOut of memory\n",
                   returnValue
                  );
            return 0;
            break;
        default:
            fprintf(stderr, "\nERROR [code s%d]"
                            "\nUndefined Error\n",
                   returnValue
                  );
            break;
    }
    
    /* 写文件 */
    printf ("\nWritting file %s\n", outfile.fileName);
    if (access(outfile.fileName, F_OK) == 0)
    {/* 检查文件是否存在 */
        printf ("\nWARNING"
                "\nFile %s already exists, it will be overwritten when continue.\n", outfile.fileName);
        if (isContinue(ignoreWarnings) == FALSE)
        {
            return 0;
        }
        
        /* 权限检查 */
        if (access(outfile.fileName, W_OK) != 0)
        {
            fprintf(stderr, "\nERROR"
                            "\nPermission denied.\n");
            return 0;
        }
    }
    
    if (!strcmp("ass", outfile.template))
    {
        returnValue = writeAss(outfile.fileName,/* 输出文件名 */
                               danmakuPool,/* 弹幕池的头指针 */
                               config,
                               NULL,/* 字幕部分 */ 
                               NULL/* 状态 */
                              );
        /* 解析百位 */
        switch (returnValue / 100)
        {
            case 0:
                break;
            case 100:
                fprintf(stderr, "\nERROR [code wa%d]"
                                "\nFailed to create file %s\n",
                       returnValue, outfile.fileName
                      );
                return 0;
                break;
            default:
                fprintf(stderr, "\nERROR [code wa%d]"
                                "\nUndefined Error\n",
                       returnValue
                      );
                break;
        }
        /* 解析十位 */
        returnValue -= returnValue % 100;
        switch (returnValue % 10)
        {
            case 0:
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                fprintf(stderr, "\nERROR [code wa%d]"
                                "\nOut of memory\n",
                       returnValue
                      );
                return 0;
                break;
            case 9:
                    fprintf(stderr, "\nERROR [code wa%d]"
                                    "\nFailed to write file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
            default:
                fprintf(stderr, "\nERROR [code wa%d]"
                                "\nUndefined Error\n",
                       returnValue
                      );
                break;
        }
        /* 个位函数错误信息已经被以上过滤完毕 无需重新报错 */
    }
    else if (!strcmp("xml", outfile.template))
    {
        returnValue = writeXml(outfile.fileName, danmakuPool, NULL);
        switch (returnValue)
        {
            case 0:
                break;
            case 2:
                fprintf(stderr, "\nERROR [code wx%d]"
                                "\nFailed to create file %s\n",
                       returnValue, outfile.fileName
                      );
                return 0;
                break;
            case 3:
                    fprintf(stderr, "\nERROR [code wx%d]"
                                    "\nFailed to write file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
            default:
                fprintf(stderr, "\nERROR [code wx%d]"
                                "\nUndefined Error\n",
                       returnValue
                      );
                break;
        }
    }
    else if (!strcmp("json", outfile.template))
    {
        returnValue = writeJson(outfile.fileName, danmakuPool, NULL);
        switch (returnValue)
        {
            case 0:
                break;
            case 2:
                fprintf(stderr, "\nERROR [code wj%d]"
                                "\nFailed to create file %s\n",
                       returnValue, outfile.fileName
                      );
                return 0;
                break;
            case 3:
                    fprintf(stderr, "\nERROR [code wj%d]"
                                    "\nFailed to write file %s\n",
                           returnValue, infile[cnt].fileName
                          );
                    return 0;
                    break;
            default:
                fprintf(stderr, "\nERROR [code wj%d]"
                                "\nUndefined Error\n",
                       returnValue
                      );
                break;
        }
    }
    
    printf("\nDone!\n");
    
    return 0;
}

/* 打印配置信息 */
void printConfigInfo(CONFIG config)
{
    printf("\n"
           "configuration:\n"
           "ResX: %d | ResY: %d | ScrollTime: %.3f | FixTime: %.3f | Density: %d",
           config.resx, config.resy, config.scrolltime, config.fixtime, config.density
          );
    if (config.density == -1)
    {
        printf("(non-overlap)");
    }
    else if (config.density == 0)
    {
        printf("(unlimit)");
    }
    
    printf(" | Fontname %s | Fontsize: %d | Opacity: %d | Outline: %d",
           config.fontname, config.fontsize, config.opacity, config.outline
          );
    if (config.outline == 0)
    {
        printf("(disable)");
    }
    
    printf(" | Shadow: %d", config.shadow);
    if (config.shadow == 0)
    {
        printf("(disable)");
    }

    printf(" | Bold: ");
    if (config.bold == FALSE)
    {
        printf("false");
    }
    else
    {
        printf("true");
    }
    
    printf(" | DisplayArea: %.3f", config.displayarea);
    if (fabs(config.displayarea - 1) < EPS)
    {
        printf("(full)");
    }
    
    printf(" | ScrollArea: %.3f", config.scrollarea);
    if (fabs(config.scrollarea - 1) < EPS)
    {
        printf("(full)");
    }

    printf(" | SaveBlocked: ");
    if (config.saveBlockedPart == FALSE)
    {
        printf("false");
    }
    else
    {
        printf("true");
    }
    
    printf("\nBlockMode: ");
    if (config.blockmode == 0)
    {
        printf("null(disable)");
    }
    else
    {
        if (config.blockmode & BLK_L2R)
        {
            printf("L2R(left to right) ");
        }
        if (config.blockmode & BLK_R2L)
        {
            printf("R2L(right to left) ");
        }
        if (config.blockmode & BLK_TOP)
        {
            printf("top ");
        }
        if (config.blockmode & BLK_BOTTOM)
        {
            printf("bottom ");
        }
        if (config.blockmode & BLK_SPECIAL)
        {
            printf("special ");
        }
        if (config.blockmode & BLK_COLOR)
        {
            printf("color ");
        }
        if (config.blockmode & BLK_REPEAT)
        {
            printf("repeat ");
        }
    }
    
    printf("\nStatMode:  ");
    if (config.statmode == 0)
    {
        printf("null(disable)");
    }
    else
    {
        if (config.statmode & TABLE)
        {
            printf("table ");
        }
        if (config.statmode & HISTOGRAM)
        {
            printf("histogram ");
        }
    }
    
    printf("\n");
    return;
}

/* 打印帮助信息 */
void printHelpInfo()
{
    printf("\n"
           "./DanmakuFactory -o [outfile_format] outfile -i [infile1_template] infile1 [infile2_template] infile2 ... "
           "[-t infile1_timeshift ...] [configuration value] ...\n"
           "\n"
           "format:\n"
           "xml, json, ass\n"
           "template:\n"
           "xml, json, ass or a template name"
           "\n"
           "configurations:\n"
           "-x, --resx          specify the value of resolution width\n"
           "-y, --resy          specify the value of resolution height\n"
           "-s, --scrolltime    specify the time of rolling danmaku to across the screen\n"
           "-f, --fixtime       specify the time of fix danmaku show on the screen\n"
           "-d, --density       specify the maximum number of danmaku could show on the screen at the same time\n"
           "                    special value: -1 non-overlap, 0 unlimit\n"
           "\n"
           "-S, --fontsize      specify the fontsize of general danmaku\n"
           "-N, --fontname      specify the fontname of general danmaku\n"
           "-O, --opacity       specify the opacity of danmaku EXCEPT the special danmaku(range: 1-255)\n"
           "-L, --outline       specify the width of outline for each danmaku(range: 0-4)\n"
           "-D, --shadow        specify the depth of shadow for each danmaku(range: 0-4)\n"
           "-B, --Bold          specify whether the font should be boldface\n"
           "                    available value: TRUE, FALSE\n"
           "\n"
           "--displayarea       specify the percent of display area on the screen(range: 0.0-1.0)\n"
           "--scrollarea        specify the percent of scroll area of rolling danmaku on the screen(range: 0.0-1.0)\n"
           "\n"
           "-b, --blockmode     specify the type of danmaku which will not show on the screen\n"
           "                    use '-' to connect the type-name, like L2R-TOP-BOTTOM\n"
           "                    available value: L2R, R2L, TOP, BOTTOM, SPECIAL, COLOR, REPEAT\n"
           "\n" 
           "--statmode          specify the type of statistic box which will show on the screen\n"
           "                    use '-' to connect the type-name, like TABLE-HISTOGRAM\n"
           "                    available value: TABLE, HISTOGRAM\n"
           "\n"
           "other options:\n"
           "-h, --help          display this help and version information than exit\n"
           "-c, --config        display configuration information and exit\n"
           "--save              save configuration as current command settings\n"
           "\n"
           "example:\n"
           "./DanmakuFactory -o ass \"outfile.ass\" -i xml \"infile1.xml\"\n"
           "./DanmakuFactory -o \"outfile.ass\" -i \"infile1.xml\"\n"
           "./DanmakuFactory -o \"outfile.ass\" -i \"infile1.xml\" \"infile2.json\" \"infile3.ass\" -t 0.0 5.0 7.0\n"
          );
    
    return;
}

/* 获取指定下标选项参数个数 */
int getArgNum(int argc, char **argv, const int optionIndex)
{
    int cnt;
    for (cnt = optionIndex + 1; cnt < argc; cnt++)
    {
        if (argv[cnt][0] == '-')
        {
            break;
        }
    }
    return cnt - optionIndex - 1;
}

/* 获取文件格式 */
char *getFormat(char *outFormat, const char *const fileName, int maxLen)
{
    char *ptr;
    ptr = (char *)&fileName[strlen(fileName)];
    
    while (*ptr != '.')
    {
        if (ptr < fileName)
        {
            strcpy(outFormat, "");
            return outFormat;
        }
        ptr--;
    }
    ptr++;
    
    strSafeCopy(outFormat, ptr, maxLen);
    
    return outFormat;
}

/*  
 * 获取文件路径部分（包含最后一个斜杠） 
 * 参数：输出文件路径/完整文件名/最大长度 
 * 返回值：输出文件路径
  */
char *getPath(char *outPath, const char *const fileName, int maxLen)
{
    int pathLen;
    char *inPtr = (char *)fileName;
    char *outPtr = outPath;

    /* 移动指针到末尾 */
    while (*inPtr != '\0')
    {
        inPtr++;
    }

    /* 向前寻找斜杠 */
    while (inPtr != fileName)
    {
        if (*inPtr == '\\' || *inPtr == '/')
        {
            break;
        }
        inPtr--;
    }
    
    if (inPtr == fileName)
    {
        outPath[0] = '\0';
        return outPath;
    }
    
    pathLen = inPtr - fileName + 2;/* 包含斜杠与结束符 */

    strSafeCopy(outPtr, fileName, maxLen > pathLen ? pathLen : maxLen);

    return outPath;
}

/* 获取一个实数参数 */
double getArgVal(int argc, char **argv, const int optIndex, const char *const optName, const double errorReturnValue)
{
    double value;
    if (optIndex+1 >= argc)
    {
        fprintf(stderr, "ERROR\n"
                        "%s must be specified\n", optName);
        return errorReturnValue;
    }
    
    if (!ISNUMBERIC(argv[optIndex+1]))
    {
        fprintf(stderr, "ERROR\n"
                        "Invalid argument %s\n", argv[optIndex+1]);
        return errorReturnValue;
    }
    
    switch (getArgNum(argc, argv, optIndex))
    {
        /* 0个代表后一个参数为负数被解释成选项 */
        case 0:
        case 1:
            deQuotMarks(argv[optIndex+1]);
            value = atof(argv[optIndex+1]);
            break;
        default:
            fprintf(stderr, "ERROR\n"
                            "Invalid argument %s\n", argv[optIndex+2]);
            return errorReturnValue;
            break;
    }
    
    return value;
}

/* 暂停 按回车继续 */
void toPause(BOOL skip)
{
    if (skip)
    {
        printf("\nSkip!\n");
        return;
    }
    printf("\nPress ENTER to continue.\n");
    fflush(stdin);
    getchar();
}

/* 是否继续 */
BOOL isContinue(BOOL skip)
{
    if (skip)
    {
        printf("\nSkip!\n");
        return TRUE;
    }
    printf("\nPress 'Y' or 'y' to continue, any other key to exit.\n");
    printf("> ");
    fflush(stdin);
    char ch = getchar();
    if (ch == 'Y' || ch == 'y')
    {
        return TRUE;
    }
    
    return FALSE;
}
