# Emulador CHIP-8

Emulador de CHIP-8 / SCHIP / XO-CHIP con dos versiones: una para **Windows** (Visual Studio 2022 + SDL2) y otra para **ESP32** (PlatformIO + pantalla ILI9341, teclado MCP23017, tarjeta SD y audio).

---

## Versión Windows

### Requisitos

- Visual Studio 2022 (cualquier edición, con el workload *Desarrollo de escritorio con C++*)
- Los paquetes NuGet de SDL2 se descargan automáticamente al abrir la solución

### Abrir y compilar

1. Abre `Visual Studio 2022/Chip8.sln` con Visual Studio 2022.
2. Restaura los paquetes NuGet si lo pide (menú *Proyecto → Restaurar paquetes NuGet*).
3. Selecciona la configuración `Debug | x64` (o `Release | x64`).
4. Compila con **Ctrl+Shift+B**.

El ejecutable queda en `bin/Debug/Chip8.exe` junto con `SDL2.dll`.

### Estructura de archivos necesaria

El ejecutable busca la configuración y las ROMs en rutas relativas a su propia ubicación:

```
bin/Debug/
├── Chip8.exe
├── SDL2.dll
├── Config/
│   └── Config.toml
├── games/
│   ├── Pong.ch8
│   └── ...
└── testRoms/
    └── ...
```

### Ejecutar

Desde la terminal, pasa la ruta de la ROM como argumento:

```cmd
cd "bin\Debug"
Chip8.exe "games\Pong (1 player).ch8"
```

O desde Visual Studio, ve a *Proyecto → Propiedades → Depuración → Argumentos del comando* e introduce la ruta de la ROM.

### Teclado (versión Windows)

El teclado CHIP-8 (4×4 hex) se mapea al teclado del PC con el layout definido en `Config/Config.toml`:

```
CHIP-8   PC
──────   ──────
1 2 3 C  1 2 3 4
4 5 6 D  Q W E R
7 8 9 E  A S D F
A 0 B F  Z X C V
```

El mapeo se puede cambiar editando la sección `[Keyboard]` del `Config.toml`. Los nombres de tecla deben ser nombres SDL válidos (los mismos que devuelve `SDL_GetKeyName`).

### Config.toml (Windows)

```toml
[Logging]
Log = 0               # 0=desactivado  1=CHIP-8  2=Octo

[InterpreterMode]
interpreter = 3       # 0=VIP  1=SCHIP  2=OCTO  3=XO-CHIP

[Quirks]
# -1=valor por defecto del intérprete  0=desactivado  1=activado
VFreset     = -1
Memory      = -1
DisplayWait = -1
Clipping    = -1
Shifting    = -1
Jumping     = -1

[IPF]
IPF = 1000            # instrucciones por frame

[Display]
windowResolution  = [640, 320]   # resolución de la ventana en píxeles
displayResolution = [64, 32]     # resolución lógica CHIP-8
backgroundColor   = 0xffF9FFB3  # ARGB
foreground1Color  = 0xff3D8026
foreground2Color  = 0xffABCC47
blendedColor      = 0xffABCC47

[Audio]
frequency  = 440
sampleRate = 44100
volume     = 3000

[Keyboard]
# hex_digit = ["nombre_tecla_SDL", índice_chip8]
0 = ["X",  0]
1 = ["1",  1]
2 = ["2",  2]
3 = ["3",  3]
4 = ["Q",  4]
5 = ["W",  5]
6 = ["E",  6]
7 = ["A",  7]
8 = ["S",  8]
9 = ["D",  9]
A = ["Z", 10]
B = ["C", 11]
C = ["4", 12]
D = ["R", 13]
E = ["F", 14]
F = ["V", 15]
```

---

## Versión ESP32

### Hardware

| Componente | Detalles |
|---|---|
| MCU | ESP32 (DevKit 38 pines) |
| Pantalla | ILI9341 TFT 320×240 (apaisado) |
| Teclado | Matriz 4×4 a través del expansor I2C MCP23017 (dirección 0x20) |
| Almacenamiento | Tarjeta MicroSD (FAT32) |
| Audio | DAC / I2S |

### Conexiones

**Pantalla ILI9341 (SPI2 / HSPI)**

| Pin pantalla | GPIO ESP32 |
|---|---|
| SCLK | 14 |
| MOSI | 13 |
| MISO | 12 |
| DC | 2 |
| CS | 15 |
| RST | 4 |
| BL | 21 |

**Tarjeta SD (SPI3 / VSPI)**

| Pin SD | GPIO ESP32 |
|---|---|
| CLK | 18 |
| MOSI | 23 |
| MISO | 19 |
| CS | 5 |

**Teclado MCP23017 (I2C)**

| Señal | GPIO ESP32 |
|---|---|
| SDA | 22 o 27 |
| SCL | 27 o 22 |

El firmware detecta automáticamente cuál es SDA y cuál es SCL al arrancar: prueba las dos combinaciones y se queda con la que recibe ACK en la dirección `0x20`. Conecta los 16 botones entre los pines GPIOA/B del MCP23017 y GND; los pull-ups internos se activan por software.

### Estructura de la tarjeta SD

Formatea la tarjeta como FAT32 y crea la siguiente estructura:

```
/
├── config/
│   └── Config.toml
└── roms/
    ├── Pong.ch8
    ├── Space Invaders.ch8
    └── ...
```

Extensiones de ROM soportadas: `.ch8`, `.c8`, `.sc8`, `.c8x`

### Config.toml (ESP32)

```toml
[Logging]
Log = 0           # 0=desactivado  1=CHIP-8  2=Octo

[InterpreterMode]
interpreter = 3   # 0=VIP  1=SCHIP  2=OCTO  3=XO-CHIP

[Quirks]
# -1=valor por defecto del intérprete  0=desactivado  1=activado
VFreset     = -1
Memory      = -1
DisplayWait = -1
Clipping    = -1
Shifting    = -1
Jumping     = -1

[IPF]
IPF = 100000      # instrucciones por frame

[Display]
windowResolution  = [320, 240]
displayResolution = [64, 32]
backgroundColor   = 0xF7DE    # RGB565
foreground1Color  = 0x1B05
foreground2Color  = 0xB6E9
blendedColor      = 0xB6E9

[Panel]
panelResolution = [320, 240]
rotation = 0
rgbOrder = 1
flipY    = 1      # 1 si la pantalla está montada al revés
flipX    = 1

[Audio]
frequency  = 440
sampleRate = 44100
volume     = 3000

[Keyboard]
# hex_digit = ["Key_NOMBRE", gpio_bit]
# gpio_bit es el índice de bit en el MCP23017 (orden de cableado físico, no valor hex)
1 = ["Key_1",  0]
2 = ["Key_2",  1]
3 = ["Key_3",  2]
C = ["Key_C",  3]
4 = ["Key_4",  4]
5 = ["Key_5",  5]
6 = ["Key_6",  6]
D = ["Key_D",  7]
7 = ["Key_7",  8]
8 = ["Key_8",  9]
9 = ["Key_9", 10]
E = ["Key_E", 11]
A = ["Key_A", 12]
0 = ["Key_0", 13]
B = ["Key_B", 14]
F = ["Key_F", 15]
```

El campo `gpio_bit` es la posición física del bit en el MCP23017. El firmware extrae el valor hex CHIP-8 del nombre de la clave automáticamente, por lo que ambos no tienen que coincidir.

### Compilar y flashear

Requisitos: [PlatformIO](https://platformio.org/) (CLI o extensión de VS Code).

```bash
# Solo compilar
pio run

# Compilar, flashear y abrir monitor serie (115200 baudios)
pio run --target upload && pio device monitor
```

El proyecto apunta a la placa `esp32dev`. La tabla de particiones está definida en `partitions.csv`.

### Uso

1. Inserta la SD con `config/Config.toml` y al menos una ROM en `roms/`.
2. Enciende la placa.
3. Aparece el selector de ROMs en la pantalla:
   - **Tecla 2** — mover cursor arriba
   - **Tecla 8** — mover cursor abajo
   - **Tecla 5** — cargar la ROM seleccionada
4. El juego arranca inmediatamente.

Si no se detecta teclado al arrancar, la primera ROM de la lista se carga sola tras una cuenta atrás de 5 segundos.

---

## Multijugador por Wi-Fi (ESP32)

El emulador implementa los opcodes de red XO-CHIP experimentales (`FxA0`–`FxA8`). Una placa actúa como host (crea un punto de acceso Wi-Fi) y hasta otras dos se unen como clientes. El estado de red se intercambia por UDP en el puerto 7777. Solo las ROMs que usen estos opcodes harán uso de esta función.
