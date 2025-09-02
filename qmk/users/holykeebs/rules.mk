VALID_POINTING_DEVICE_CONFIGURATIONS := trackball tps43 cirque40 cirque35 tps43_tps43 cirque35_cirque35 cirque40_cirque40 trackball_trackball trackball_tps43 tps43_trackball trackball_cirque35 cirque35_trackball trackball_cirque40 cirque40_trackball trackpoint trackpoint_tps43 tps43_trackpoint trackpoint_trackball trackball_trackpoint trackpoint_cirque35 cirque35_trackpoint trackpoint_cirque40 cirque40_trackpoint cirque40_tps43 tps43_cirque40 cirque35_tps43 tps43_cirque35 trackpoint_trackpoint
ifdef POINTING_DEVICE
    ifeq ($(filter $(POINTING_DEVICE),$(VALID_POINTING_DEVICE_CONFIGURATIONS)),)
        $(call CATASTROPHIC_ERROR,Invalid POINTING_DEVICE,POINTING_DEVICE="$(POINTING_DEVICE)" is not a valid pointing device configuration)
	endif
endif

ifeq ($(strip $(CONSOLE)), yes)
	CONSOLE_ENABLE = yes
endif

TRI_LAYER_ENABLE = yes
EXTRAKEY_ENABLE = yes
MOUSEKEY_ENABLE = yes
SERIAL_DRIVER = vendor

ifdef POINTING_DEVICE
	SRC += $(USER_PATH)/holykeebs.c $(USER_PATH)/hk_debug.c $(USER_PATH)/rpc.c $(USER_PATH)/pimoroni.c $(USER_PATH)/trackpoint.c
endif

MSG_POINTING_DEVICE = none
MSG_OLED = no
MASTER_SIDE = right

ifeq ($(strip $(POINTING_DEVICE_POSITION)), right)
	OPT_DEFS += -DPOINTING_DEVICE_POSITION_RIGHT
	MASTER_SIDE = right
else ifeq ($(strip $(POINTING_DEVICE_POSITION)), left)
	OPT_DEFS += -DPOINTING_DEVICE_POSITION_LEFT
	MASTER_SIDE = left
else ifeq ($(strip $(POINTING_DEVICE_POSITION)), thumb)
	OPT_DEFS += -DPOINTING_DEVICE_POSITION_THUMB
	MASTER_SIDE = right
else ifeq ($(strip $(POINTING_DEVICE_POSITION)), thumb_inner)
	OPT_DEFS += -DPOINTING_DEVICE_POSITION_THUMB_INNER
	MASTER_SIDE = right
else ifeq ($(strip $(POINTING_DEVICE_POSITION)), thumb_outer)
	OPT_DEFS += -DPOINTING_DEVICE_POSITION_THUMB_OUTER
	MASTER_SIDE = right
else ifeq ($(strip $(POINTING_DEVICE_POSITION)), middle)
	OPT_DEFS += -DPOINTING_DEVICE_POSITION_MIDDLE
	MASTER_SIDE = n/a (not a split keyboard)
endif

ifeq ($(strip $(POINTING_DEVICE)), trackball)
	ifeq ($(strip $(POINTING_DEVICE_POSITION)), right)
		OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_PIMORONI
	else ifeq ($(strip $(POINTING_DEVICE_POSITION)), left)
		OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_PIMORONI
	endif


	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = pimoroni_trackball

	MSG_POINTING_DEVICE = trackball
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque35)
	ifeq ($(strip $(POINTING_DEVICE_POSITION)), right)
		OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE35
	else ifeq ($(strip $(POINTING_DEVICE_POSITION)), left)
		OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE35
	endif

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c

	MSG_POINTING_DEVICE = cirque35 touchpad
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque40)
	ifeq ($(strip $(POINTING_DEVICE_POSITION)), right)
		OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE40
	else ifeq ($(strip $(POINTING_DEVICE_POSITION)), left)
		OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE40
	endif

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c

	MSG_POINTING_DEVICE = cirque40 touchpad
endif

ifeq ($(strip $(POINTING_DEVICE)), tps43)
	ifeq ($(strip $(POINTING_DEVICE_POSITION)), right)
		OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TPS43
	else ifeq ($(strip $(POINTING_DEVICE_POSITION)), left)
		OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TPS43
	endif

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = azoteq_iqs5xx

	MSG_POINTING_DEVICE = TPS43 touchpad

endif

ifeq ($(strip $(POINTING_DEVICE)), trackpoint)
	ifeq ($(strip $(POINTING_DEVICE_POSITION)), right)
		OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TRACKPOINT
	else ifeq ($(strip $(POINTING_DEVICE_POSITION)), left)
		OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TRACKPOINT
	endif

	PS2_ENABLE = yes
	PS2_DRIVER = vendor
	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = ps2
	MSG_POINTING_DEVICE = trackpoint
endif

ifeq ($(strip $(POINTING_DEVICE)), tps43_tps43)
	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TPS43
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TPS43

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = azoteq_iqs5xx

	ifeq ($(strip $(SIDE)), left)
	endif
	ifeq ($(strip $(SIDE)), right)
		OPT_DEFS += -DAZOTEQ_IQS5XX_ROTATION_180
	endif


	MSG_POINTING_DEVICE = TPS43 touchpad (left), TPS43 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque35_cirque35)
	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE35
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE35

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c

	MSG_POINTING_DEVICE = cirque35 touchpad (left), cirque35 touchpad (right)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque40_cirque40)
	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE40
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE40

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c

	MSG_POINTING_DEVICE = cirque40 touchpad (left), cirque40 touchpad (right)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackball_trackball)
	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_PIMORONI
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_PIMORONI

	POINTING_DEVICE_ENABLE = yes
	POINTING_DEVICE_DRIVER = pimoroni_trackball

	MSG_POINTING_DEVICE = trackball (left), trackball (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackball_cirque35)
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_PIMORONI
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE35

	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	POINTING_DEVICE_ENABLE = yes

	ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = pimoroni_trackball
	endif
	ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
	endif

	MSG_POINTING_DEVICE = trackball (left), cirque35 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque35_trackball)
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE35
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_PIMORONI

	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	POINTING_DEVICE_ENABLE = yes

	ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = pimoroni_trackball
	endif
	ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
	endif

	MSG_POINTING_DEVICE = cirque35 touchpad (left), trackball (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), trackball_cirque40)
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_PIMORONI
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE40

	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	POINTING_DEVICE_ENABLE = yes

	ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = pimoroni_trackball
	endif
	ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
	endif

	MSG_POINTING_DEVICE = trackball (left), cirque40 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque40_trackball)
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE40
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_PIMORONI

	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	POINTING_DEVICE_ENABLE = yes

	ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = pimoroni_trackball
	endif
	ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
	endif

	MSG_POINTING_DEVICE = cirque40 touchpad (left), trackball (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), trackball_tps43)
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_PIMORONI
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TPS43

	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	POINTING_DEVICE_ENABLE = yes

	ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = pimoroni_trackball
	endif
	ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
	endif

	MSG_POINTING_DEVICE = trackball (left), TPS43 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), tps43_trackball)
	OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TPS43
	OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_PIMORONI

	OPT_DEFS += -DSPLIT_POINTING_ENABLE
	OPT_DEFS += -DPOINTING_DEVICE_COMBINED
	POINTING_DEVICE_ENABLE = yes

	ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = pimoroni_trackball
	endif
	ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
	endif

	MSG_POINTING_DEVICE = TPS43 touchpad (left), trackball (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), tps43_cirque35)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TPS43
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE35

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
    endif
    ifeq ($(strip $(SIDE)), right)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = TPS43 touchpad (left), cirque35 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque35_tps43)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE35
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TPS43

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
    endif
    ifeq ($(strip $(SIDE)), left)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = cirque35 touchpad (left), TPS43 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), tps43_cirque40)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TPS43
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE40

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
    endif
    ifeq ($(strip $(SIDE)), right)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = TPS43 touchpad (left), cirque40 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque40_tps43)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE40
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TPS43

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
    endif
    ifeq ($(strip $(SIDE)), left)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = cirque40 touchpad (left), TPS43 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackpoint_trackpoint)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TRACKPOINT
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TRACKPOINT

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

	PS2_ENABLE = yes
	PS2_DRIVER = vendor
	POINTING_DEVICE_DRIVER = ps2

	MSG_POINTING_DEVICE = trackpoint (left), trackpoint (right)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackpoint_trackball)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TRACKPOINT
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_PIMORONI

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), left)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), right)
        POINTING_DEVICE_DRIVER = pimoroni_trackball
    endif

	MSG_POINTING_DEVICE = trackpoint (left), trackball (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), trackball_trackpoint)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_PIMORONI
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TRACKPOINT

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), right)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), left)
        POINTING_DEVICE_DRIVER = pimoroni_trackball
    endif

	MSG_POINTING_DEVICE = trackball (left), trackpoint (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackpoint_tps43)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TRACKPOINT
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TPS43

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), left)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), right)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
    endif

	MSG_POINTING_DEVICE = trackpoint (left), TPS43 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), tps43_trackpoint)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TPS43
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TRACKPOINT

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), right)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), left)
		POINTING_DEVICE_DRIVER = azoteq_iqs5xx
    endif

	MSG_POINTING_DEVICE = TPS43 touchpad (left), trackpoint (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackpoint_cirque35)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TRACKPOINT
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE35

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), left)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), right)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = trackpoint (left), cirque35 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque35_trackpoint)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE35
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TRACKPOINT

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), right)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), left)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = cirque35 touchpad (left), trackpoint (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(POINTING_DEVICE)), trackpoint_cirque40)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_TRACKPOINT
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_CIRQUE40

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), left)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), right)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = trackpoint (left), cirque40 touchpad (right); built for side = $(SIDE)
	MASTER_SIDE = left
endif

ifeq ($(strip $(POINTING_DEVICE)), cirque40_trackpoint)
    OPT_DEFS += -DHK_POINTING_DEVICE_LEFT_CIRQUE40
    OPT_DEFS += -DHK_POINTING_DEVICE_RIGHT_TRACKPOINT

    POINTING_DEVICE_ENABLE = yes
    OPT_DEFS += -DSPLIT_POINTING_ENABLE
    OPT_DEFS += -DPOINTING_DEVICE_COMBINED

    ifeq ($(strip $(SIDE)), right)
        PS2_ENABLE = yes
        PS2_DRIVER = vendor
        POINTING_DEVICE_DRIVER = ps2
    endif
    ifeq ($(strip $(SIDE)), left)
        POINTING_DEVICE_DRIVER = cirque_pinnacle_i2c
    endif

	MSG_POINTING_DEVICE = cirque40 touchpad (left), trackpoint (right); built for side = $(SIDE)
	MASTER_SIDE = right
endif

ifeq ($(strip $(MASTER_SIDE)), left)
	MSG_MASTER_SIDE = left
	OPT_DEFS += -DHK_MASTER_LEFT
else ifeq ($(strip $(MASTER_SIDE)), middle)
	MSG_MASTER_SIDE = n/a (not a split keyboard)
else
	MSG_MASTER_SIDE = right
	OPT_DEFS += -DHK_MASTER_RIGHT
endif

ifeq ($(strip $(OLED)), yes)
	OPT_DEFS += -DOLED_ENABLE
	OPT_DEFS += -DHK_OLED_ENABLE
	OLED_ENABLE     = yes
	OLED_DRIVER     = ssd1306
	MSG_OLED = yes
	SRC += $(USER_PATH)/oled.c
else ifeq ($(strip $(OLED)), stock)
	OPT_DEFS += -DOLED_ENABLE
	OLED_ENABLE     = yes
	OLED_DRIVER     = ssd1306
	MSG_OLED = yes
else
	OLED_ENABLE     = no
	MSG_OLED = no
endif

ifeq ($(strip $(OLED_FLIP)), yes)
	OPT_DEFS += -DOLED_FLIP
	MSG_OLED_FLIPPED = (flipped)
endif

ifeq ($(strip $(TRACKBALL_RGB_RAINBOW)), yes)
	OPT_DEFS += -DHK_PIMORONI_TRACKBALL_RGB_RAINBOW
	SRC += quantum/color.c
endif

print-summary: cpfirmware
	printf "\n%s" ' _           _       _             _         ' >&2
	printf "\n%s" '| |__   ___ | |_   _| | _____  ___| |__  ___ ' >&2
	printf "\n%s" "| '_ \ / _ \| | | | | |/ / _ \/ _ \ '_ \/ __|" >&2
	printf "\n%s" '| | | | (_) | | |_| |   <  __/  __/ |_) \__ \' >&2
	printf "\n%s" '|_| |_|\___/|_|\__, |_|\_\___|\___|_.__/|___/' >&2
	printf "\n%s" '               |___/                         ' >&2
	printf "\n\nPointing Device: $(BOLD)$(MSG_POINTING_DEVICE)$(NO_COLOR)\n" >&2
	printf "OLED: $(BOLD)$(MSG_OLED) $(MSG_OLED_FLIPPED)$(NO_COLOR)\n" >&2
	printf "Keyboard main side: $(BOLD)$(MSG_MASTER_SIDE)$(NO_COLOR)\n" >&2
	printf "\n$(WARN_COLOR)WARNING!$(NO_COLOR) Avoid connecting / disconnecting the TRRS cable when the keyboard is powered. This can short the GPIO pins of the controllers.\n\n" >&2
