# BME280_fse
**Projeto dividido em duas partes, sendo uma em C e outra em Python**

## 4.1 Python
- Printa no LCD temperatura, pressao e humidade;  
`$ python3 bme_lcd.py`

## 4.2 C
- Le temperatura, pressao e humidade;
- Cria um arquivo .CSV e adiciona a media das 3 a cada 10 segundos, junto com a data;
```
$ make
$ ./bin/bin /dev/i2c-1
```
