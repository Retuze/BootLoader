'''
Author        : Retuze 
Date          : 2023-12-02 01:07:33
LastEditors   : Retuze 
LastEditTime  : 2023-12-02 01:07:33
Description   : 
'''
import os
import glob
import re
import subprocess
from tabulate import tabulate

# 遍历当前路径寻找链接脚本
ld = None
LDSCRIPT_LIST = glob.glob(r"./**/*.ld", recursive=True)
if LDSCRIPT_LIST:
    for LDSCRIPT in LDSCRIPT_LIST:
        ld = LDSCRIPT
        if os.path.basename(LDSCRIPT) == "ldscript.ld":
            break
else:
    print("No matching .ld files found.")
    exit(-1)
# 从链接脚本拿到芯片的flash和ram大小
with open(ld, 'r', encoding="utf-8") as f:
    # 使用正则表达式匹配MEMORY{}内的内容
    pattern = r"MEMORY\s*\{([\s\S]*?)\}"
    match = re.search(pattern, f.read())
    if match:
        memory_area = match.group(1)
    else:
        print("No match found.")

# 使用splitlines()将文本分割成行，然后使用split()将每一行分割成各个部分
memory_parts = memory_area.splitlines()

# 初始化变量来存储RAM、CCMRAM和FLASH的信息
ram_length = None
ccmram_length = None
flash_length = None
ram_start = None
ccmram_start = None
flash_start = None

# 遍历内存信息
for part in memory_parts:
    if "RAM" in part:
        match = re.search(r'(\d+)K$', part)  # 搜索以大写字母K结尾的数字
        if match:
            ram_length = int(match.group(1))
        match = re.search(r"0x[0-9a-fA-F]{8}", part)
        if match:
            ram_start = match.group()
    elif "CCMRAM" in part:
        match = re.search(r'(\d+)K$', part)  # 搜索以大写字母K结尾的数字
        if match:
            ccmram_length = int(match.group(1))
        match = re.search(r"0x[0-9a-fA-F]{8}", part)
        if match:
            ccmram_start = match.group()
    elif "FLASH" in part:
        match = re.search(r'(\d+)K$', part)  # 搜索以大写字母K结尾的数字
        if match:
            flash_length = int(match.group(1))
        match = re.search(r"0x[0-9a-fA-F]{8}", part)
        if match:
            flash_start = match.group()

# 遍历当前路径寻找固件
ELF = glob.glob(r".\build\*.elf")
if ELF:
    ELF = ELF[0]
else:
    print("No matching .elf files found.")

if os.path.exists(ELF):
    size_out = subprocess.run(
        "arm-none-eabi-size " + ELF, shell=True, stdout=subprocess.PIPE
    ).stdout
    # 使用 split() 方法将行拆分成字段
    fields = size_out.decode().split()
    # 提取所需的字段
    text_size = int(fields[6])
    data_size = int(fields[7])
    bss_size = int(fields[8])
    dec_size = int(fields[9])
    need_flash = int(text_size) + int(data_size)
    need_ram = int(data_size) + int(bss_size)


FLASH_Size = float(flash_length)
FLASH_Used = float(need_flash/1000)
FLASH_Free = float(FLASH_Size-FLASH_Used)
FLASH_Usage = float(FLASH_Used/FLASH_Size*100)
FLASH_Start_Address = ''
FLASH_End_Address = ''

RAM_Size = float(ram_length)
RAM_Used = float(need_ram/1000)
RAM_Free = float(RAM_Size-RAM_Used)
RAM_Usage = float(RAM_Used/RAM_Size*100)
RAM_Start_Address = ''
RAM_End_Address = ''

# 根据不同的占用比例输出不同的颜色
FLASH_Color = ''
if FLASH_Usage < 30:
    FLASH_Color = '\033[32m'  # 绿色
elif FLASH_Usage < 60:
    FLASH_Color = '\033[36m'  # 蓝色
elif FLASH_Usage < 90:
    FLASH_Color = '\033[33m'  # 黄色
else:
    FLASH_Color = '\033[91m'  # 红色

# 根据不同的占用比例输出不同的颜色
RAM_Color = ''
if RAM_Usage < 30:
    RAM_Color = '\033[32m'  # 绿色
elif RAM_Usage < 60:
    RAM_Color = '\033[36m'  # 蓝色
elif RAM_Usage < 90:
    RAM_Color = '\033[33m'  # 黄色
else:
    RAM_Color = '\033[91m'  # 红色

Color_End = '\033[0m'

FLASH_Size_String = f"{FLASH_Color}{float(FLASH_Size):.3f}{Color_End}KB"
FLASH_Used_String = f"{FLASH_Color}{float(FLASH_Used):.3f}{Color_End}KB"
FLASH_Free_String = f"{FLASH_Color}{float(FLASH_Free):.3f}{Color_End}KB"
FLASH_Usage_String = f"{FLASH_Color}{float(FLASH_Usage):.2f}{Color_End}%"

RAM_Size_String = f"{RAM_Color}{float(RAM_Size):.3f}{Color_End}KB"
RAM_Used_String = f"{RAM_Color}{float(RAM_Used):.3f}{Color_End}KB"
RAM_Free_String = f"{RAM_Color}{float(RAM_Free):.3f}{Color_End}KB"
RAM_Usage_String = f"{RAM_Color}{float(RAM_Usage):.2f}{Color_End}%"

d = [["FLASH", FLASH_Size_String, FLASH_Free_String, FLASH_Used_String, FLASH_Usage_String],
     ["RAM", RAM_Size_String, RAM_Free_String, RAM_Used_String, RAM_Usage_String]]

print(tabulate(d, headers=["Region", "Size", "Free", "Used", "Usage(%)"]))
