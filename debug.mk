# Copyright (C) Juraj Hercek
# Based on work of WinAVR Makefile Template written by Eric B. Weddington et al.

$(GDBINIT_FILE):
	@rm -f $(GDBINIT_FILE)
	@echo define reset >> $(GDBINIT_FILE)
	@echo     signal SIGHUP >> $(GDBINIT_FILE)
	@echo end >> $(GDBINIT_FILE)
	@echo file $(TARGET).elf >> $(GDBINIT_FILE)
	@echo target remote $(DEBUG_HOST):$(DEBUG_PORT) >> $(GDBINIT_FILE)
	@echo source $(GDBINIT_FILE).user >> $(GDBINIT_FILE)
	@echo continue >> $(GDBINIT_FILE)

debug: $(GDBINIT_FILE) $(TARGET).elf
	@echo Starting AVaRICE...
	@# using grep because screen always returns "1" even if there's a match
	@while ! (screen -list | grep -q AVR_debugging); do \
		screen -S AVR_debugging -d -m \
			avarice $(AVARICE_FLAGS) $(DEBUG_HOST):$(DEBUG_PORT); \
		sleep 1; \
	 done
	@screen -S AVR_debugging -X screen avr-$(DEBUG_UI) --command=$(GDBINIT_FILE)
	@screen -S AVR_debugging -r
