# Compilación

Para compilar y grabar M4 Core:

	make

	make download

Para compilar y grabar M0 Core:

	make TARGET=lpc4337_m0

	make TARGET=lpc4337_m0 download


# Utilización

Funciona sobre EduCIAA NXP.
Conectar un pendrive al USB conteniendo archivos mp3 en el directorio raíz.

	TEC2 y TEC3	Seleccionar archivo del "File Browser"
	TEC1		Play/Stop.

Las conexiones al TFT LCD ILI9341 son:

		ILI9341 --> EduCIAA-NXP
		V+	-->	+3.3V  
		CS	-->	GPIO0  
		D/C	-->	GPIO1  
		DI	-->	SPI_MOSI  
		CLK	-->	SPI_SCK  
		DO	-->	SPI_MISO  
		GND	-->	GND				
    
# educiaa-cese
Proyectos de la Carrera de Especialización en Sistemas Embibidos 2018
    
