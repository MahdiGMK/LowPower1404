<div dir='rtl'>

# مقایسه روش های BusInverting و فشرده سازی
مهدی بهرامیان 401171593

برای مقایسه این روش ها کافیست توابع مورد نیازمان را برای 
شبیهسازی BusInverting پیاده کنیم و 
به آن داده های فایل را به صورت خام و فشرده شده توسط zlib 
بدهیم و در هر ۴ حالت مقایسه کنیم.
برای این آزمون یک فایل data فشرده نشده را در نظر گرفتیم که 
کتابخانه zlib موثر باشد.

</div>


```python
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
print('RawData + Normal BUS(8) : ' , (data_tot / data_tot - 1) * 100, '%')
print('RawData + BUS(8) Inverting : ' ,  (data_tot / data_bi - 1) * 100, '%')
print('Compressed + Normal BUS(8) : ' , (data_tot / zdata_tot - 1) * 100, '%')
print('Compressed + BUS(8) Inverting : ' , (data_tot / zdata_bi - 1) * 100, '%')
```

    Total Bit Flips :
    RawData + Normal BUS(8) :  0.0 %
    RawData + BUS(8) Inverting :  32.149226050360234 %
    Compressed + Normal BUS(8) :  184.14110429447854 %
    Compressed + BUS(8) Inverting :  279.1649611133852 %


<div dir='rtl'>

# نتایج آزمون
همانطور که مشاهده میکنید، حالت بهینه با اختلاف زیاد استفاده از
هر دو روش است و در این مثال خاص ۲۸۰ درصد بهبود داشته است.
در نتیجه دو روش بر یکدیگر عمود هستند و به شکل مستقل بهبود ایجاد میکنند.
</div>
