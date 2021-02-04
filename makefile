
TempPath := temp

DanmakuFactory: $(TempPath)/main.o $(TempPath)/string.o $(TempPath)/list.o $(TempPath)/config.o $(TempPath)/template.o $(TempPath)/ass.o $(TempPath)/json.o $(TempPath)/xml.o $(TempPath)/safer_str_func_fix.o
	gcc -m32 $(TempPath)/main.o $(TempPath)/string.o $(TempPath)/list.o $(TempPath)/config.o $(TempPath)/template.o $(TempPath)/ass.o $(TempPath)/json.o $(TempPath)/xml.o $(TempPath)/safer_str_func_fix.o -o DanmakuFactory

$(TempPath)/string.o: String/DanmakuFactoryString.c
	gcc -m32 -c String/DanmakuFactoryString.c -o $(TempPath)/string.o

$(TempPath)/list.o: List/DanmakuFactoryList.c
	gcc -m32 -c List/DanmakuFactoryList.c -o $(TempPath)/list.o

$(TempPath)/config.o: Config/Config.c
	gcc -m32 -c Config/Config.c -o $(TempPath)/config.o

$(TempPath)/template.o: TemplateFile/TemplateFile.c
	gcc -m32 -c TemplateFile/TemplateFile.c -o $(TempPath)/template.o

$(TempPath)/ass.o: AssFile.c
	gcc -m32 -c AssFile.c -o $(TempPath)/ass.o

$(TempPath)/xml.o: XmlFile.c
	gcc -m32 -c XmlFile.c -o $(TempPath)/xml.o

$(TempPath)/json.o: JsonFile.c
	gcc -m32 -c JsonFile.c -o $(TempPath)/json.o

$(TempPath)/safer_str_func_fix.o: safer_str_func_fix.c
	gcc -c safer_str_func_fix.c -o $(TempPath)/safer_str_func_fix.o

$(TempPath)/main.o: main.c
	gcc -m32 -c main.c -o $(TempPath)/main.o