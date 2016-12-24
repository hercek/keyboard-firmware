#!/usr/bin/zsh
#i3-msg 'focus_on_window_activation none'
setsid xterm -T openocd -e 'avarice -g -X -j usb :4242' &
sleep 0.5
#i3-msg 'focus_on_window_activation smart'
i3-msg 'workspace 2' > /dev/null
trap 'killall -s INT -e avarice' INT
sleep 2
avr-gdb --command=gdbinit
killall -s INT -e avarice
