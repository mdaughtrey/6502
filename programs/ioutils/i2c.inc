.ifndef I2C_INC
.define I2C_INC
.macro scl_high
    ora #I2C_SCL
    sta SELECTPORT
.endmacro

.macro scl_low
    and #NOT_I2C_SCL
    sta SELECTPORT
.endmacro

.macro sda_high
    ora #I2C_SDA
    sta SELECTPORT
.endmacro

.macro sda_low
    and #NOT_I2C_SDA
    sta SELECTPORT
.endmacro

.macro sda_in
    lda SELECTIO        ; set SDA to input
    and #NOT_I2C_SDA
    sta SELECTIO
.endmacro

.macro sda_out
    lda SELECTIO        ; set SDA to input
    ora #I2C_SDA
    sta SELECTIO
.endmacro

.macro i2c_start
    lda SELECTPORT      
    scl_low
    sda_high
    scl_high
    sda_low
    scl_low
.endmacro

.macro i2c_stop
    lda SELECTPORT      ; stop condition, SCL is Low
    and #NOT_I2C_SDA         ; SDA Low
    sta SELECTPORT      ; ...
    ora #I2C_SCL         ; SCL High
    sta SELECTPORT      ; ...
    ora #I2C_SDA        ; SDA High
    sta SELECTPORT      ; ...
    and #NOT_I2C_SCL        ; SCK Low
    sta SELECTPORT      ; ...
    and #NOT_I2C_SDA         ; SDA Low
    sta SELECTPORT      ; ...
.endmacro

I2C_SCL = %10000000    ; 6522 SELECTPORT D7
NOT_I2C_SCL = %01111111
I2C_SDA = %01000000    ; 6522 SELECTPORT D6
NOT_I2C_SDA = %10111111

.endif ;  I2C_INC
