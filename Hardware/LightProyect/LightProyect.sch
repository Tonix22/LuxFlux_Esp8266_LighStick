EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:PWR_FLAG #FLG01
U 1 1 5EE98E5D
P 2250 1450
F 0 "#FLG01" H 2250 1525 50  0001 C CNN
F 1 "PWR_FLAG" H 2250 1623 50  0000 C CNN
F 2 "" H 2250 1450 50  0001 C CNN
F 3 "~" H 2250 1450 50  0001 C CNN
	1    2250 1450
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG02
U 1 1 5EE98FDA
P 3400 3600
F 0 "#FLG02" H 3400 3675 50  0001 C CNN
F 1 "PWR_FLAG" H 3350 3750 50  0000 C CNN
F 2 "" H 3400 3600 50  0001 C CNN
F 3 "~" H 3400 3600 50  0001 C CNN
	1    3400 3600
	-1   0    0    1   
$EndComp
$Comp
L power:VCC #PWR01
U 1 1 5EE9959C
P 1950 1500
F 0 "#PWR01" H 1950 1350 50  0001 C CNN
F 1 "VCC" H 1950 1700 50  0000 C CNN
F 2 "" H 1950 1500 50  0001 C CNN
F 3 "" H 1950 1500 50  0001 C CNN
	1    1950 1500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5EE99A46
P 2700 3600
F 0 "#PWR02" H 2700 3350 50  0001 C CNN
F 1 "GND" H 2705 3427 50  0000 C CNN
F 2 "" H 2700 3600 50  0001 C CNN
F 3 "" H 2700 3600 50  0001 C CNN
	1    2700 3600
	1    0    0    -1  
$EndComp
$Comp
L RF_Module:ESP-12E U1
U 1 1 5EE9A169
P 5500 3150
F 0 "U1" H 5500 4131 50  0000 C CNN
F 1 "ESP-12E" H 5500 4040 50  0000 C CNN
F 2 "RF_Module:ESP-12E" H 5500 3150 50  0001 C CNN
F 3 "http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf" H 5150 3250 50  0001 C CNN
	1    5500 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 3600 2900 3600
Connection ~ 3400 3600
$Comp
L LightProyect-rescue:DC_DC_3_5_module-DC_DC_3_5_V_Module M2
U 1 1 5EE9F569
P 3800 1450
F 0 "M2" H 3912 1815 50  0000 C CNN
F 1 "DC_DC_3_5_module" H 3912 1724 50  0000 C CNN
F 2 "LightProyect:DC_DC_5V_3V_module" H 3900 1800 50  0001 C CNN
F 3 "" H 3900 1800 50  0001 C CNN
	1    3800 1450
	1    0    0    -1  
$EndComp
$Comp
L LightProyect-rescue:Regulator_x_3_3-VoltageRegulator_X_3.3 M1
U 1 1 5EEA2C27
P 3700 2350
F 0 "M1" H 3837 2765 50  0000 C CNN
F 1 "Regulator_x_3_3" H 3837 2674 50  0000 C CNN
F 2 "LightProyect:YP8AMS1117_5v_3_3_V" H 3650 2700 50  0001 C CNN
F 3 "" H 3650 2700 50  0001 C CNN
	1    3700 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 2400 3050 2400
Wire Wire Line
	3050 2400 3050 3600
Connection ~ 3050 3600
Wire Wire Line
	3050 3600 3400 3600
Wire Wire Line
	3400 3600 4350 3600
Wire Wire Line
	4150 2400 4350 2400
Wire Wire Line
	4350 2400 4350 3600
Wire Wire Line
	3250 1350 3150 1350
Wire Wire Line
	2900 1350 2900 3600
Connection ~ 2900 3600
Wire Wire Line
	2900 3600 3050 3600
Wire Wire Line
	4350 1350 4350 950 
Wire Wire Line
	4350 950  3150 950 
Wire Wire Line
	3150 950  3150 1350
Connection ~ 3150 1350
Wire Wire Line
	3150 1350 2900 1350
Wire Wire Line
	4350 1550 4350 1800
Wire Wire Line
	4350 1800 3150 1800
Wire Wire Line
	3150 1800 3150 2250
Connection ~ 4350 3600
$Comp
L LightProyect-rescue:MPU6050_lite-MPU6050_compressed M4
U 1 1 5EEA9A03
P 6900 3150
F 0 "M4" H 7178 3071 50  0000 L CNN
F 1 "MPU6050_lite" H 7178 2980 50  0000 L CNN
F 2 "LightProyect:GY-521" H 6900 3400 50  0001 C CNN
F 3 "" H 6900 3400 50  0001 C CNN
	1    6900 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 3350 6600 3350
Wire Wire Line
	6600 3150 6600 2350
Wire Wire Line
	6600 2350 5500 2350
Wire Wire Line
	6600 3450 6300 3450
Wire Wire Line
	6300 3450 6300 2750
Wire Wire Line
	6300 2750 6100 2750
$Comp
L power:GND #PWR06
U 1 1 5EEB087B
P 6600 3250
F 0 "#PWR06" H 6600 3000 50  0001 C CNN
F 1 "GND" V 6605 3122 50  0000 R CNN
F 2 "" H 6600 3250 50  0001 C CNN
F 3 "" H 6600 3250 50  0001 C CNN
	1    6600 3250
	0    1    1    0   
$EndComp
Wire Wire Line
	2700 3600 2700 2250
Wire Wire Line
	2700 2250 2200 2250
Connection ~ 2700 3600
Wire Wire Line
	2200 2150 2200 1550
Wire Wire Line
	1950 1550 1950 1500
Wire Wire Line
	1950 1550 2200 1550
Connection ~ 2200 1550
Wire Wire Line
	2200 1550 2250 1550
Wire Wire Line
	2250 1400 2250 1450
Connection ~ 2250 1550
Wire Wire Line
	2250 1550 3250 1550
Connection ~ 2250 1450
Wire Wire Line
	2250 1450 2250 1550
Wire Wire Line
	5500 2350 4550 2350
Wire Wire Line
	4550 2350 4550 2250
Wire Wire Line
	4150 2250 4550 2250
Connection ~ 5500 2350
Wire Wire Line
	5500 3850 4350 3850
Wire Wire Line
	4350 3600 4350 3850
Wire Wire Line
	6100 2550 6750 2550
Wire Wire Line
	7150 2550 7200 2550
$Comp
L power:GND #PWR07
U 1 1 5EEC104A
P 7200 2550
F 0 "#PWR07" H 7200 2300 50  0001 C CNN
F 1 "GND" V 7205 2422 50  0000 R CNN
F 2 "" H 7200 2550 50  0001 C CNN
F 3 "" H 7200 2550 50  0001 C CNN
	1    7200 2550
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5EEBFDC3
P 6950 2550
F 0 "SW1" H 6950 2835 50  0000 C CNN
F 1 "SW_Push" H 6950 2744 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_TL3305C" H 6950 2750 50  0001 C CNN
F 3 "~" H 6950 2750 50  0001 C CNN
	1    6950 2550
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x03 J3
U 1 1 5EEC38D8
P 7400 2800
F 0 "J3" H 7480 2842 50  0000 L CNN
F 1 "Screw_Terminal_01x03" H 7480 2751 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 7400 2800 50  0001 C CNN
F 3 "~" H 7400 2800 50  0001 C CNN
	1    7400 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 2700 7200 2550
Connection ~ 7200 2550
Wire Wire Line
	7200 2800 6950 2800
Wire Wire Line
	6950 2800 6950 2650
Wire Wire Line
	6950 2650 6100 2650
Wire Wire Line
	6750 2900 6750 2850
Wire Wire Line
	6750 2850 6100 2850
Wire Wire Line
	6750 2900 7200 2900
$Comp
L Connector:Screw_Terminal_01x02 J1
U 1 1 5EECC20A
P 1250 2450
F 0 "J1" H 1168 2125 50  0000 C CNN
F 1 "Screw_Terminal_01x02" H 1168 2216 50  0000 C CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 1250 2450 50  0001 C CNN
F 3 "~" H 1250 2450 50  0001 C CNN
	1    1250 2450
	-1   0    0    1   
$EndComp
Wire Wire Line
	1450 2350 1450 1550
Wire Wire Line
	1450 1550 1950 1550
Connection ~ 1950 1550
Wire Wire Line
	1450 2450 1450 3600
Wire Wire Line
	1450 3600 2700 3600
$Comp
L LightProyect-rescue:AudioModule-AudioModule M3
U 1 1 5EED2E37
P 5150 1400
F 0 "M3" H 5428 1221 50  0000 L CNN
F 1 "AudioModule" H 5428 1130 50  0000 L CNN
F 2 "LightProyect:Audio_Senso_module" H 5150 1400 50  0001 C CNN
F 3 "" H 5150 1400 50  0001 C CNN
	1    5150 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4350 1550 4700 1550
$Comp
L power:GND #PWR03
U 1 1 5EED4411
P 4850 1750
F 0 "#PWR03" H 4850 1500 50  0001 C CNN
F 1 "GND" V 4855 1622 50  0000 R CNN
F 2 "" H 4850 1750 50  0001 C CNN
F 3 "" H 4850 1750 50  0001 C CNN
	1    4850 1750
	0    1    1    0   
$EndComp
Wire Wire Line
	4850 1650 4600 1650
Wire Wire Line
	4600 1650 4600 2000
Wire Wire Line
	4600 2000 4750 2000
Wire Wire Line
	4750 2000 4750 3350
Wire Wire Line
	4750 3350 4900 3350
Wire Wire Line
	6150 1100 4700 1100
Wire Wire Line
	4700 1100 4700 1550
Connection ~ 4700 1550
Wire Wire Line
	4700 1550 4850 1550
$Comp
L power:GND #PWR04
U 1 1 5EED96E6
P 4850 1750
F 0 "#PWR04" H 4850 1500 50  0001 C CNN
F 1 "GND" V 4855 1622 50  0000 R CNN
F 2 "" H 4850 1750 50  0001 C CNN
F 3 "" H 4850 1750 50  0001 C CNN
	1    4850 1750
	0    1    1    0   
$EndComp
Connection ~ 4850 1750
$Comp
L power:GND #PWR05
U 1 1 5EED9B42
P 6150 1700
F 0 "#PWR05" H 6150 1450 50  0001 C CNN
F 1 "GND" H 6155 1527 50  0000 C CNN
F 2 "" H 6150 1700 50  0001 C CNN
F 3 "" H 6150 1700 50  0001 C CNN
	1    6150 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 1400 5850 2200
Wire Wire Line
	5850 2200 4700 2200
Wire Wire Line
	4700 2200 4700 3450
Wire Wire Line
	4700 3450 4900 3450
$Comp
L power:GND #PWR08
U 1 1 5EF010D0
P 2200 2350
F 0 "#PWR08" H 2200 2100 50  0001 C CNN
F 1 "GND" H 2205 2177 50  0000 C CNN
F 2 "" H 2200 2350 50  0001 C CNN
F 3 "" H 2200 2350 50  0001 C CNN
	1    2200 2350
	1    0    0    -1  
$EndComp
$Comp
L LightProyect-rescue:NeoPixel_THT-neopixel_custom D1
U 1 1 5EF06E3B
P 6150 1400
F 0 "D1" H 6380 1446 50  0000 L CNN
F 1 "NeoPixel_THT" H 6380 1355 50  0000 L CNN
F 2 "LightProyect:Neopixel" H 6200 1100 50  0001 L TNN
F 3 "https://www.adafruit.com/product/1938" H 6250 1025 50  0001 L TNN
	1    6150 1400
	1    0    0    -1  
$EndComp
$Comp
L LightProyect-rescue:AudioJack2_SwitchT-ONOFF J2
U 1 1 5EF4A436
P 2000 2250
F 0 "J2" H 2032 2575 50  0000 C CNN
F 1 "AudioJack2_SwitchT" H 2032 2484 50  0000 C CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 2000 2250 50  0001 C CNN
F 3 "~" H 2000 2250 50  0001 C CNN
	1    2000 2250
	1    0    0    -1  
$EndComp
$EndSCHEMATC
