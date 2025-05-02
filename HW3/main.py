import zlib
import BI, Activity, Zip
def readFile(addr: str):
    with open(addr, 'rb') as file:
        return file.read(None)


data = readFile('./data')
zdata = zlib.compress(data)

data_bus = Activity.toParallelBus(data, 8)
zdata_bus = Activity.toParallelBus(zdata, 8)

data_tot = 0
for bus in data_bus:
    data_tot += Activity.calcTotal(bus)

zdata_tot = 0
for bus in zdata_bus:
    zdata_tot += Activity.calcTotal(bus)

data_bi = BI.calcTotal(data_bus)
zdata_bi = BI.calcTotal(zdata_bus)

print("Total Bit Flips :")
print('RawData + Normal BUS(8) : ' , data_tot / data_tot)
print('RawData + BUS(8) Inverting : ' , data_bi / data_tot)
print('Compressed + Normal BUS(8) : ' , zdata_tot / data_tot)
print('Compressed + BUS(8) Inverting : ' , zdata_bi / data_tot)
