import sys
import struct
import shutil
import os
import time

UF2_MAGIC_START0 = 0x0A324655 
UF2_MAGIC_START1 = 0x9E5D5157 
UF2_MAGIC_END    = 0x0AB16F30 

def convert_bin_to_uf2(bin_data, start_addr, family_id_hex):
    output = b""
    block_no = 0
    num_blocks = (len(bin_data) + 255) // 256
    
    for index in range(0, len(bin_data), 256):
        block = bin_data[index:index+256]
        while len(block) < 256:
            block += b"\x00"
            
        flags = 0x00002000 
        hd = struct.pack(
            "<IIIIIIII",
            UF2_MAGIC_START0, UF2_MAGIC_START1,
            flags, start_addr + index,
            256, block_no, num_blocks, family_id_hex
        )
        while len(hd) < 32:
            hd += b"\x00"
        ft = struct.pack("<I", UF2_MAGIC_END)
        
        output += hd + block + b"\x00" * (476 - len(block)) + ft
        block_no += 1
    return output

if __name__ == "__main__":
    input_bin = sys.argv[1]
    output_uf2 = sys.argv[2]
    target_drive = sys.argv[3]
    
    # Поиск и сброс платы
    try:
        import serial
        from serial.tools import list_ports
        import time
        
        ports = [p.device for p in list_ports.comports() if 'F411CE' in p.description]
        if ports:
            port = ports[0]
            print(f"Found active board on port {port}")
            
            # Открываем порт
            ser = serial.Serial(port, 115200, timeout=1)
            time.sleep(0.1)
            
            #Отправляем команду перезагрузки в режиме загрузчика
            ser.write(b"BOOT\n") 
            ser.flush()
            
            time.sleep(0.1)
            ser.close()
        else:
            print("Board serial device not active.")
    except Exception as e:
        print(f"Serial trigger failed: {e}")


    # --- ЭТАП 2: Конвертация прошивки в UF2 ---
    with open(input_bin, "rb") as f:
        bin_data = f.read()
    uf2_data = convert_bin_to_uf2(bin_data, 0x08010000, 0x57755a57)
    with open(output_uf2, "wb") as f:
        f.write(uf2_data)

    print(f"Waiting for drive {target_drive}...")
    drive_found = False
    
    # Ждем носитель в течение 5 секунд
    for i in range(50): 
        if os.path.exists(target_drive):
            drive_found = True
            break
        time.sleep(0.1) # Проверяем каждые 100 миллисекунд
        
    if not drive_found:
        print(f"ERROR: Drive {target_drive} did not appear!")
        sys.exit(1)

    # Копирование файла
    dest_path = os.path.join(target_drive, "firmware.uf2")
    try:
        shutil.copyfile(output_uf2, dest_path)
        print("SUCCESS: Firmware transferred successfully")
    except Exception as e:
        print(f"Copy failed: {e}")
        sys.exit(1)
