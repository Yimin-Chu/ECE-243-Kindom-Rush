# 该程序的使用指南

# 1. 安装依赖库
# 在运行此程序之前，请确保已安装以下Python库：
# - numpy
# - opencv-python
# - pyperclip
# 可以通过以下命令安装：
# pip install numpy opencv-python pyperclip

# 2. 准备输入图片
# 将需要转换的图片文件放置在程序目录下的 `images` 文件夹中。
# 确保图片文件的路径和名称与 `convert_img` 函数的 `file_name` 参数一致。

# 3. 修改参数
# 在 `if __name__ == '__main__':` 块中，修改以下参数：
# - `file_name`: 输入图片的路径（相对于程序目录）。
# - `img_name`: 转换后C语言数组的名称。
# - `width`: 调整后的图片宽度（像素）。
# - `height`: 调整后的图片高度（像素）。

# 4. 运行程序
# 在终端或命令行中运行程序：
# python convert_img.py

# 5. 获取输出
# 程序运行后会将生成的C语言数组和绘制函数代码复制到剪贴板。
# 打开任意文本编辑器（如记事本），粘贴剪贴板内容即可查看生成的代码。

# 6. 使用生成的代码
# 将生成的C语言代码复制到您的项目中。
# 使用 `plot_image_<img_name>` 函数绘制图片，使用 `erase_image_<img_name>` 函数擦除图片。

# 注意事项：
# - 输入图片会被调整为指定的宽度和高度，可能会导致失真。
# - 生成的C语言数组使用16位无符号整数表示颜色，格式为RGB565。

import numpy as np
import cv2
import pyperclip


# this function converts the image to the c-style array
# the image is first resized to the specified width and height
# the image is then converted to short unsigned int color with 5 bits for red, 6 bits for green, and 5 bits for blue
# the image is then flattened to a 1D array
# the image is then copied to the clipboard
def convert_img(file_name, img_name, width, height):
    img = cv2.imread(file_name).astype(np.uint16)
    img = cv2.resize(img, (width, height), interpolation=cv2.INTER_NEAREST)
    # img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img_r = img[:, :, 2] >> 3
    img_g = img[:, :, 1] >> 2
    img_b = img[:, :, 0] >> 3
    print(img_r[0, 0], img_g[0, 0], img_b[0, 0])
    img = (img_r << 11) + (img_g << 5) + img_b
    print(img[0, 0])
    img = img.flatten()
    answer = ', '.join(map(lambda x: f"0x{x:04X}", img))
    answer = f"short unsigned int {img_name}[{width * height}] = {{{answer}}};"

    answer += "\n\n"
    answer += generate_draw_pixel_code(img_name, width, height, img_name)

    pyperclip.copy(answer)


# This is the function that generates the plot_image function
# this function utilizes the plot_pixel function
# the signature of the plot_pixel function is assumed to be void plot_pixel(int x, int y, short unsigned int color);
# the signature of the plot_image function is assumed to be void plot_image_img_name(int x, int y);
def generate_draw_pixel_code(img_name, width, height, color):
    code = f"""
void plot_image_{img_name}(int x, int y) {{
    for (int i = 0; i < {height}; i++) {{
        for (int j = 0; j < {width}; j++) {{
            plot_pixel(x + j, y + i, {color}[i * {width} + j]);
        }}
    }}
}}


void erase_image_{img_name}(int x, int y) {{
    for (int i = 0; i < {height}; i++) {{
        for (int j = 0; j < {width}; j++) {{
            plot_pixel(x + j, y + i, 0);
        }}
    }}
}}
"""
    return code


if __name__ == '__main__':
    convert_img(
        file_name='C:/Users/peter/Desktop/ece243-image/3.png',
        img_name='sine',
        width=320,
        height=240
    )
