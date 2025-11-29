.ifndef MCP23017_INC
.define MCP23017_INC

IODIRA   = $00      ; I/O Direction A
IODIRB   = $01      ; I/O Direction B
IPOLA    = $02      ; Input Polarity A
IPOLB    = $03      ; Input Polarity B
GPINTENA = $04      ; Interrupt on Change Enable A
GPINTENB = $05      ; Interrupt on Change Enable B
DEFVALA  = $06      ; Default Value A
DEFVALB  = $07      ; Default Value B
INTCONA  = $08      ; Interrupt on Change Control A
INTCONB  = $09      ; Interrupt on Change Control B
IOCON    = $0A      ; I/O Expander Configuration
;IOCON    = $0B      ; I/O Expander Configuration
GPPUA    = $0C      ; Pullup Resistor A
GPPUB    = $0D      ; Pullup Resistor B
INTFA    = $0E      ; Interrupt Flag A
INTFB    = $0F      ; Interrupt Flag B
INTCAPA  = $10      ; Interrupt Captured A
INTCAPB  = $11      ; Interrupt Captured B
GPIOA    = $12      ; GPIO A
GPIOB    = $13      ; GPIO B
OLATA    = $14      ; Output Latch A
OLATB    = $15      ; Output Latch B

.endif ;  MCP23017_INC
