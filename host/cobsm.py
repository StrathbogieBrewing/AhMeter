def encode(buffer):
    # Ensure the input is a bytearray
    if not isinstance(buffer, bytearray):
        raise TypeError("Input must be a bytearray")

    size = len(buffer)
    nul_index = size + 1
    zero_offset = size

    # Extend buffer to accommodate extra elements
    buffer.append(nul_index) 
    buffer.append(0) 
    
    buffer_index = size - 1

    while buffer_index >= 0:
        if buffer[buffer_index] == 0:
            buffer[zero_offset] = size - buffer_index
            zero_offset = buffer_index
            buffer[zero_offset] = nul_index
        buffer_index -= 1
    
    return buffer


def decode(buffer):
    # Ensure the input is a bytearray
    if not isinstance(buffer, bytearray):
        raise TypeError("Input must be a bytearray")

    size = len(buffer)
    nul_index = size - 1
    zero_offset = 0
    zero_index = 0

    while zero_index != nul_index - zero_offset - 1:
        zero_index = nul_index - zero_offset - 1
        if zero_index >= nul_index or zero_index < 0:
            return bytearray()
        if buffer[zero_index] == nul_index:
            buffer[zero_index] = 0
            break
        else:
            zero_offset = buffer[zero_index]
            buffer[zero_index] = 0

    return buffer[:-2]



 