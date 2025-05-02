import Activity
def calcTotal(bus :list[bytes]) -> int:
    totRes = 0
    for i in range(0,len(bus),2):
        if len(bus) == i + 1: # last bus
            totRes += Activity.calcTotal(bus[i])
        else:
            lastX = 0
            lastY = 0
            lastZ = 0
            for j in range(len(bus[i]) * 8):
                x = Activity.getBit(bus[i], j)
                y = Activity.getBit(bus[i + 1], j)
                z = lastZ
                opt1 = (x^z^lastX) + (y^z^lastY)
                z = 1 - lastZ
                opt2 = 1 + (x^z^lastX) + (y^z^lastY)
                totRes += min(opt1 , opt2)
                if opt1 <= opt2:
                    z = lastZ
                else:
                    z = 1 - lastZ
                lastX = x ^ z
                lastY = y ^ z
                lastZ = z
    return totRes
