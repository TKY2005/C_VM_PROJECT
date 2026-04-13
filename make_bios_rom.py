with open("bios.tky", "wb") as file:
    content = bytearray([0x01] * 0xffff)
    file.write(content)

