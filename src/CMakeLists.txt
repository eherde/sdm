cmake_minimum_required(VERSION 2.8.12.2)
project(sdm)
set(CMAKE_C_FLAGS "-Wall -Wextra -g -static")
add_executable(sdm sdm/cmdline.c sdm/main.c sdm/paths.c sdm/proc.c sdm/start.c
	sdm/status.c sdm/stop.c sdm/which.c)
install(TARGETS sdm DESTINATION ${DESTDIR}/usr/bin)
