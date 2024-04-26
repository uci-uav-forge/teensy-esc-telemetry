import serial
import base64
import time

ser = serial.Serial(
        '/dev/ttyAMA0', 
        baudrate = 115200, 
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE
)

buff = []

def update_crc8(crc, crc_seed):
    crc_u = 0
    crc_u = crc
    crc_u ^= crc_seed
    for i in range(8):
        crc_u = 0x7 ^ (crc_u<<1) if (crc_u & 0x80) else (crc_u<<1)
        crc_u = crc_u & 0xFF
    return crc_u

def crc8(buff: list) -> bool:
    '''Calculate crc8 of buffer'''
    crc = 0
    for i in range(len(buff)):
        crc = update_crc8(buff[i], crc)
    return crc

while 1:
    buff.append(ord(ser.read()))
    if len(buff) == 10:
        if crc8(buff[:-1]) == buff[-1]:
            temp = buff[0]
            voltage = (buff[1]<<8) + buff[2]
            current = (buff[3]<<8) + buff[4]
            consumption = (buff[5]<<8) + buff[6]
            rpm = (buff[7]<<8) + buff[8]

            t = int(time.time()*1e6)
            t_bytes = t.to_bytes((t.bit_length()+7)//8, byteorder='big')
            t_str = base64.b64encode(t_bytes).decode()

            with open("logs_esc_2.txt", "a") as f:
                f.write(f'{t_str} {temp} {voltage} {current} {consumption} {rpm}\n')

            buff = []
        else:
            del buff[0]
