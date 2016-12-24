# avarice -g -X -j usb :4242

define connect
target remote localhost:4242
end

file Keyboard.elf

connect

