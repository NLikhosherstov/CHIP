import os
import sys
from PIL import Image

def sanitize_name(filename):
    """Очищает имя файла для соответствия синтаксису C++ (заменяет дефисы и пробелы)"""
    base = os.path.splitext(os.path.basename(filename))[0]
    # Заменяем пробелы и дефисы на нижнее подчеркивание
    sanitized = base.replace(" ", "_").replace("-", "_")
    return sanitized

def process_all_pngs(source_dir, output_h_name="icons.h"):
    if not os.path.exists(source_dir):
        print(f"Ошибка: Директория '{source_dir}' не найдена!")
        return

    # Находим все .png файлы в указанной папке
    png_files = [f for f in os.listdir(source_dir) if f.lower().endswith('.png')]
    
    if not png_files:
        print(f"В папке '{source_dir}' не найдено файлов .png")
        return

    # Заголовок файла .h
    h_content = (
        ""
    )

    processed_count = 0

    for png_file in png_files:
        full_path = os.path.join(source_dir, png_file)
        array_name = sanitize_name(png_file)
        
        try:
            # Открываем изображение и извлекаем RGBA каналы
            img = Image.open(full_path).convert("RGBA")
            width, height = img.size
            
            alpha_data = []
            for y in range(height):
                for x in range(width):
                    _, _, _, a = img.getpixel((x, y))
                    # Сжимаем 8 бит альфа-канала (0-255) в 4 бита (0-15)
                    alpha_data.append(a >> 4)
                    
            # Упаковываем каждые два 4-битных пикселя в один байт uint8_t
            packed_bytes = []
            for i in range(0, len(alpha_data), 2):
                p1 = alpha_data[i]
                p2 = alpha_data[i+1] if (i+1) < len(alpha_data) else 0
                packed_byte = (p1 << 4) | p2
                packed_bytes.append(f"0x{packed_byte:02X}")

            # Форматируем вывод в строки по 12 байт
            bytes_per_line = 12
            lines = []
            for i in range(0, len(packed_bytes), bytes_per_line):
                chunk = ", ".join(packed_bytes[i:i+bytes_per_line])
                lines.append("    " + chunk)

            # Добавляем метаданные размеров и сам массив в общий файл
            h_content += f"// Иконка: {png_file} ({width}x{height}px)\n"
            h_content += f"static const uint8_t {array_name}_vlw[] = {{\n"
            h_content += ",\n".join(lines) + "\n"
            h_content += "};\n\n"
            
            print(f"Обработан файл: {png_file} -> {array_name}_vlw[] ({width}x{height})")
            processed_count += 1

        except Exception as e:
            print(f"Ошибка при обработке файла {png_file}: {e}")

    # Записываем итоговый файл icons.h рядом со скриптом
    with open(output_h_name, "w", encoding="utf-8") as f:
        f.write(h_content)
        
    print(f"\nУспешно! Создан файл '{output_h_name}'. Всего обработано иконок: {processed_count}.")

if __name__ == "__main__":
    # Если аргумент передан, используем его, иначе ищем в папке "../res" по умолчанию
    target_dir = sys.argv[1] if len(sys.argv) > 1 else "./res"
    
    process_all_pngs(target_dir)
