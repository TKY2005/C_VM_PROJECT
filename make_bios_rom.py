with open("bios.tky", "wb") as file:
    content = bytearray([0x01] * 0x8000)
    file.write(content)

