ifeq ($(BOARD_HAVE_BLUETOOTH), true)
ifeq ($(SW_BOARD_HAVE_BLUETOOTH_RTK), true)
	include $(call all-subdir-makefiles)
endif	
endif

