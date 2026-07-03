FILE=level_shifter
GALETTE=~/dev/6502/galette/Cargo.toml

build:
	cargo run --manifest-path ${GALETTE} ${FILE}.pld

upload:
	minipro -p GAL16V8D -w ${FILE}.jed

