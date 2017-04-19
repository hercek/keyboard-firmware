# avarice -g -X -j usb :4242

define run
monitor reset halt
continue
end

define connect
target remote localhost:4242
end

file Keyboard.elf

connect
