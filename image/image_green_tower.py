import numpy as np
import cv2
import pyperclip

# Converts an image to a C-style RGB565 array with optional transparent pixel background fill
def convert_img(file_name, img_name, width, height, bg_rgb=(72, 151, 41)):
    # Load image with alpha (transparency)
    img = cv2.imread(file_name, cv2.IMREAD_UNCHANGED).astype(np.uint16)
    img = cv2.resize(img, (width, height), interpolation=cv2.INTER_NEAREST)

    # Extract B, G, R (and A if present)
    if img.shape[2] == 4:
        b, g, r, a = cv2.split(img)
        mask = (a > 0).astype(np.uint16)
        # Fill transparent pixels with user-specified background color
        b = b * mask + (bg_rgb[2] * (1 - mask))  # blue
        g = g * mask + (bg_rgb[1] * (1 - mask))  # green
        r = r * mask + (bg_rgb[0] * (1 - mask))  # red
    else:
        b, g, r = cv2.split(img)

    # Convert to RGB565
    img_r = r >> 3
    img_g = g >> 2
    img_b = b >> 3
    img = (img_r << 11) + (img_g << 5) + img_b
    img = img.flatten()

    # Format as C array
    answer = ', '.join(f"0x{x:04X}" for x in img)
    answer = f"short unsigned int {img_name}[{width * height}] = {{{answer}}};"

    # Append image drawing functions
    answer += "\n\n" + generate_draw_pixel_code(img_name, width, height, img_name)

    # Copy to clipboard
    pyperclip.copy(answer)
    print("✅ Converted and copied to clipboard!")


# Generates C functions to draw and erase the image
def generate_draw_pixel_code(img_name, width, height, color):
    return f"""
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

# === Example usage ===
if __name__ == '__main__':
    convert_img(
        file_name='C:/Users/peter/Desktop/ece243-image/green_bottle_no_bg.png',  # <- your PNG with transparency
        img_name='tower1',
        width=26,
        height=42,
        bg_rgb=(130, 230, 5)  # default background for transparent pixels (greenish)
    )
