* ngspice digital cells, Clifford Wolf, http://http://www.clifford.at/yosys

.SUBCKT BUF A Y
X1 A B NOT
X2 B Y NOT
.ENDS NOT

.SUBCKT NOT A Y
M1 Y A Vdd Vdd cmosp L=1u W=10u
M2 Y A Vss Vss cmosn L=1u W=10u
.ENDS NOT

.SUBCKT NAND A B Y
M1 Y A Vdd Vdd cmosp L=1u W=10u
M2 Y B Vdd Vdd cmosp L=1u W=10u
M3 Y A M34 Vss cmosn L=1u W=10u
M4 M34 B Vss Vss cmosn L=1u W=10u
.ENDS NAND

.SUBCKT NOR A B Y
M1 Y A M12 Vdd cmosp L=1u W=10u
M2 M12 B Vdd Vdd cmosp L=1u W=10u
M3 Y A Vss Vss cmosn L=1u W=10u
M4 Y B Vss Vss cmosn L=1u W=10u
.ENDS NOR

.SUBCKT DLATCH E D Q
X1 D E S NAND
X2 nD E R NAND
X3 S nQ Q NAND
X4 Q R nQ NAND
X5 D nD NOT
.ENDS DLATCH

.SUBCKT DFF C D Q
X1 nC D t DLATCH
X2 C t Q DLATCH
X3 C nC NOT
.ENDS DFF

.SUBCKT pmos D S G
M1 S G D Vdd cmosp L=8u W=2u
.ENDS pmos

.SUBCKT nmos D S G
M1 S G D Vss cmosn L=1u W=15u
.ENDS nmos
