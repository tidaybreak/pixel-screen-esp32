Import("env")

# Custom HEX from ELF
# 下面代码中使用的--chip -o -ff -fm -fs等均为esptool.py的参数命令
# 详细esptool.py使用方法介绍：https://blog.csdn.net/espressif/article/details/105028809
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", 
        "--chip esp32 elf2image ", # 设置目标环境
        "-o Test.bin ", # 该行为在项目根目录下输出Test.bin二进制烧录文件
        "-ff 40m ", # SPI速率 
        "-fm dio ", # SPI模式 　　
        "-fs 4MB ", # FLASH大小
        "$BUILD_DIR/${PROGNAME}.elf", 
    ]), "Building $BUILD_DIR/${PROGNAME}.hex")
)