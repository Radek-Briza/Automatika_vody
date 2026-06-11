# I2C Timeout Diagnostics (SSD1306_OLED)

## Problém
Funkce `HAL_I2C_Master_Transmit()` vrací `HAL_ERROR` s `HAL_I2C_ERROR_TIMEOUT` v `I2C_WaitOnFlagUntilTimeout()`.

---

## Příčiny Timeout

### 1. **I2C2 Hardware Není Inicializován** ⚠️ NEJČASTĚJŠÍ
- **Příznaky:** Timeout v každém `I2C_Write_Byte` volání
- **Řešení:** Ujistěte se, že `MX_I2C2_Init()` je volána v `main.c` PŘED `OLEDbegin()`
- **Kde:** [Core/Src/main.cpp](Core/Src/main.cpp) - přidejte v inicializačním kódu:
  ```cpp
  MX_I2C2_Init();  // Inicializuj I2C2
  // ... pak později ...
  oled.OLEDbegin(0x3C, true);  // true = debug mode
  ```

### 2. **I2C Bus Je Blokovaný/Zamrzlý**
- **Příznaky:** Timeout s `State = 0x24` (BUSY)
- **Řešení:** Přidaná logika `I2C_WaitOnFlagUntilTimeout` nyní provádí I2C bus recovery
  - Automaticky resetuje I2C2 pomocí `HAL_I2C_DeInit()` + `HAL_I2C_Init()`
  - Vidíte zprávu: `"I2C Bus Recovery: DeInit and reinit I2C"`

### 3. **OLED Není Fyzicky Připojen**
- **Příznaky:** Timeout při čekání na ACK; zařízení nereaguje
- **Ověření:**
  - Zkontrolujte SDA (PA15) a SCL (PA12) piny
  - Měřte napětí: obě by měly být ~3.3V v klidovém stavu
  - I2C přístroj/analizátor by měl vidět START a ACK pulsy

### 4. **Pull-Up Rezistory Chybí/Jsou Špatné**
- **Konfigurace:** SDA (PA15) a SCL (PA12) mají `GPIO_NOPULL` v [Core/Src/i2c.c](Core/Src/i2c.c)
- **Požadavek:** Externe připájené pull-up rezistory (4.7kΩ na 3.3V) jsou NUTNÉ
- **Diagnostika:**
  ```
  Bez pull-upů:
  - SDA/SCL zůstávají LOW po chybě
  - Timeout v každém dalším volání
  ```

### 5. **Špatná I2C Adresa**
- **Konfigurace:** Default 0x3C, alternativě 0x3D (závislé na desce OLED)
- **Ověření v `OLEDbegin()`:**
  ```cpp
  oled.OLEDbegin(0x3C, true);  // try 0x3C (default)
  // Pokud selhá, zkuste:
  // oled.OLEDbegin(0x3D, true);  // alternative address
  ```

### 6. **Timing Settings Jsou Nesprávné**
- **Konfigurační hodnota:** `hi2c2.Init.Timing = 0x10805D88` v [Core/Src/i2c.c:41](Core/Src/i2c.c#L41)
- **Frekvence:** Toto nastavení by mělo odpovídat 100 kHz (standartní mode)
- **Pokud potřebujete vyšší frekvenci:** 
  - Kontaktujte STM32CubeMX a regenerujte timing

---

## Debugging Postup

### Krok 1: Povolte Debug Výstupy
```cpp
#include "../Display/include/SSD1306_OLED.hpp"

SSD1306 oled(128, 64);
uint8_t buffer[128 * 64 / 8];

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_I2C2_Init();        // ← KRITICKÉ: I2C musí být inicializován
    
    // Inicializujte OLED s debug módem
    oled.OLEDSetBufferPtr(128, 64, buffer, sizeof(buffer));
    oled.OLEDbegin(0x3C, true);  // ← true = debug mode
    
    // Nyní budete vidět detailní zprávy
}
```

### Krok 2: Podívejte se na Serializované Výstupy
```
Očekávaný výstup (bez chyb):
  (OLED zobrazuje obsah bez chybových zpráv)

Problematický výstup (timeout):
  Error I2C_Write_Byte : Cannot Write byte, attempts left :: 0
  HAL_I2C_GetError code :: 0x20, I2C State :: 0x24
  I2C Bus Recovery: DeInit and reinit I2C
  
  Vysvětlení:
  - ErrorCode 0x20 = HAL_I2C_ERROR_TIMEOUT
  - State 0x24 = HAL_I2C_STATE_BUSY (hardware je zamrzlý)
```

### Krok 3: Ověřte I2C Sběrnici (Osciloskop/Analyzer)
```
Bez komunikace:
  SDA, SCL = nízká napětí (0V nebo osciluje)
  
Správná komunikace:
  START, ADDR (0x78 nebo 0x7A), ACK, DATA, STOP
  SDA/SCL by měly mít cleaný čtverec wave
```

---

## Nouzové Řešení

Pokud běžné řešení nefunguje, přidejte **hard reset I2C sběrnice** v `main()`:

```cpp
void I2C_Bus_HardReset(void) {
    // Deaktivujte I2C periférii
    HAL_I2C_DeInit(&hi2c2);
    
    // Resetujte GPIO na output a pulzujte SDA/SCL
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // Togglujte SCL 9krát, aby se sbíraly jakékoliv застряле bity
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);  // SCL low
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);    // SCL high
        HAL_Delay(1);
    }
    
    // Generujte STOP podmínku
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);  // SDA low
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);    // SDA high
    
    // Reinicializujte I2C
    HAL_I2C_Init(&hi2c2);
}
```

Volejte před `oled.OLEDbegin()`:
```cpp
I2C_Bus_HardReset();
oled.OLEDbegin(0x3C, true);
```

---

## Checklist pro Řešení

- [ ] `MX_I2C2_Init()` je volána v `main()`
- [ ] OLED je fyzicky připojen (SDA=PA15, SCL=PA12)
- [ ] Pull-up rezistory 4.7kΩ jsou připájeny na obě linky
- [ ] Debug režim je zapnutý (`OLEDbegin(..., true)`)
- [ ] Sériový monitor zobrazuje debug zprávy
- [ ] I2C adresy jsou zkušeny (0x3C nebo 0x3D)

---

## Reference

- I2C Handle: `hi2c2` [Core/Inc/i2c.h:35](Core/Inc/i2c.h#L35)
- I2C Init: [Core/Src/i2c.c:38-56](Core/Src/i2c.c#L38-L56)
- GPIO konfigurace: [Core/Src/i2c.c:84-104](Core/Src/i2c.c#L84-L104)
- HAL Error Codes: [Drivers/STM32WLxx_HAL_Driver/Inc/stm32wlxx_hal_i2c.h:162-175](Drivers/STM32WLxx_HAL_Driver/Inc/stm32wlxx_hal_i2c.h#L162-L175)

