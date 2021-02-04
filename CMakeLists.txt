cmake_minimum_required(VERSION 3.13)

project(DanmakuFactory)

add_executable(DanmakuFactory
        String/DanmakuFactoryString.c
        List/DanmakuFactoryList.c
        Config/Config.c
        TemplateFile/TemplateFile.c
        AssFile.c
        XmlFile.c
        JsonFile.c
        main.c safer_str_func_fix.h safer_str_func_fix.c)