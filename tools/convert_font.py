import os
import sys

def convert_vlw_to_h(vlw_filename):
    # Проверяем, существует ли исходный файл
    if not os.path.exists(vlw_filename):
        print(f"Ошибка: Файл '{vlw_filename}' не найден в текущей директории!")
        return

    try:
        # Извлекаем базовое имя файла без расширения (из 'h1.vlw' получим 'h1')
        base_name = os.path.splitext(os.path.basename(vlw_filename))[0]
        
        # Генерируем имена на основе базового
        output_h_name = f"{base_name}.h"     # Например: 'h1.h'
        array_name = f"{base_name}_vlw"       # Например: 'h1_vlw'

        with open(vlw_filename, "rb") as f:
            vlw_data = f.read()
        
        hex_bytes = [f"0x{b:02X}" for b in vlw_data]
        
        bytes_per_line = 12
        lines = []
        for i in range(0, len(hex_bytes), bytes_per_line):
            chunk = ", ".join(hex_bytes[i:i+bytes_per_line])
            lines.append("    " + chunk)
        
        h_content = (
            f"#pragma once\n"
            f"#include <stdint.h>\n\n"
            f"inline const uint8_t {array_name}[] = {{\n"
            + ",\n".join(lines) + "\n"
            f"}};\n"
        )
        
        # Записываем в файл (режим 'w' автоматически очищает старые данные)
        with open(output_h_name, "w", encoding="utf-8") as f:
            f.write(h_content)
            
        print(f"Успешно! Файл '{output_h_name}' перезаписан.")
        print(f"Имя массива: {array_name}[], размер: {len(vlw_data)} байт.")

    except Exception as e:
        print(f"Произошла ошибка при конвертации: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Использование: python convert_font.py <имя_файла.vlw>")
        print("Пример: python convert_font.py h1.vlw")
    else:
        vlw_name = sys.argv[1]
        convert_vlw_to_h(vlw_name)
