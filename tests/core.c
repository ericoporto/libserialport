/*
 * This file is part of the libserialport project.
 *
 * Copyright (C) 2015 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <check.h>
#include "libserialport.h"

#define SERIAL_PORT "/dev/ttyS0"

static void handler(const char *s, ...)
{
	(void)s;
}

START_TEST(test_sp_get_port_by_name)
{
	int ret;
	struct sp_port *port;

	/* Existing port. */
	fail_unless(sp_get_port_by_name(SERIAL_PORT, &port) == SP_OK);
	fail_unless(port != NULL);
	sp_free_port(port);

	/* Non-existing port. */
	fail_unless(sp_get_port_by_name("/dev/foobar", &port) < 0);
	fail_unless(port == NULL);

	/* NULL port name */
	fail_unless(sp_get_port_by_name(NULL, &port) < 0);
	fail_unless(port == NULL);

	/* NULL return pointer */
	fail_unless(sp_get_port_by_name(SERIAL_PORT, NULL) < 0);

	/* NULL port name and NULL return pointer */
	fail_unless(sp_get_port_by_name(NULL, NULL) < 0);
}
END_TEST

/*
 * None of the calls should segfault if certain parameters are NULL.
 * An error code (or other error indicator) should be returned instead.
 */
START_TEST(test_null_checks)
{
	int ret, x, y;
	struct sp_port *port, *port2, **list;
	char *s;
	int fd, *fdp;
	uint8_t buf[10];
	struct sp_event_set *event_set;
	enum sp_signal signal_mask;
	int baudrate, bits;
	struct sp_port_config *config;

	ret = sp_get_port_by_name(SERIAL_PORT, &port);
	fail_unless(ret == SP_OK);

	fail_unless(sp_get_port_name(port) != NULL);
	fail_unless(sp_get_port_name(NULL) == NULL);

	fail_unless(sp_get_port_description(port) != NULL);
	fail_unless(sp_get_port_description(NULL) == NULL);

	fail_unless(sp_get_port_transport(port) == SP_OK);
	/* Can't merge the following two lines into one, test would fail. */
	ret = sp_get_port_transport(NULL);
	fail_unless(ret < 0);

	// fail_unless(sp_get_port_usb_bus_address(port, &x, &y) == SP_OK);
	// fail_unless(sp_get_port_usb_vid_pid(port, &x, &y) == SP_OK);

	fail_unless(sp_get_port_handle(port, &fd) == SP_OK);
	fail_unless(sp_get_port_handle(NULL, &fd) < 0);
	fdp = &fd;
	fail_unless(sp_get_port_handle(NULL, fdp) < 0);
	// fail_unless(fdp == NULL);
	fail_unless(sp_get_port_handle(port, NULL) < 0);
	fail_unless(sp_get_port_handle(NULL, NULL) < 0);

	sp_free_port(NULL); /* Should not segfault. */

	fail_unless(sp_list_ports(&list) == SP_OK);
	sp_free_port_list(list);
	fail_unless(sp_list_ports(NULL) < 0);

	sp_free_port_list(NULL); /* Should not segfault. */

	fail_unless(sp_copy_port(port, &port2) == SP_OK);
	fail_unless(port2 != NULL);
	sp_free_port(port);
	fail_unless(sp_copy_port(port, NULL) < 0);
	fail_unless(sp_copy_port(NULL, &port2) < 0);
	fail_unless(port2 == NULL);
	fail_unless(sp_copy_port(NULL, NULL) < 0);

	// fail_unless(sp_open(port, SP_MODE_READ) == SP_OK); /* TODO */
	fail_unless(sp_open(NULL, SP_MODE_READ) < 0);

	fail_unless(sp_close(NULL) < 0);

	// fail_unless(sp_flush(port, SP_BUF_BOTH) == SP_OK); // TODO: Port must be open
	fail_unless(sp_flush(NULL, SP_BUF_BOTH) < 0);

	// fail_unless(sp_drain(port) == SP_OK); // TODO: Port must be open
	fail_unless(sp_drain(NULL) < 0);

	// fail_unless(sp_blocking_write(port, &buf, 1, 0) == SP_OK); // TODO: Must be open.
	fail_unless(sp_blocking_write(NULL, &buf, 1, 0) < 0);
	fail_unless(sp_blocking_write(port, NULL, 1, 0) < 0);
	// fail_unless(sp_blocking_write(port, &buf, 0, 0) == 0); // TODO

	// fail_unless(sp_nonblocking_write(port, &buf, 1) == SP_OK); // TODO: Must be open.
	fail_unless(sp_nonblocking_write(NULL, &buf, 1) < 0);
	fail_unless(sp_nonblocking_write(port, NULL, 1) < 0);
	// fail_unless(sp_nonblocking_write(port, &buf, 0) == 0); // TODO

	// fail_unless(sp_blocking_read(port, &buf, 1, 0) == SP_OK); // TODO: Must be open.
	fail_unless(sp_blocking_read(NULL, &buf, 1, 0) < 0);
	fail_unless(sp_blocking_read(port, NULL, 1, 0) < 0);
	// fail_unless(sp_blocking_read(port, &buf, 0, 0) == 0); // TODO

	// fail_unless(sp_nonblocking_read(port, &buf, 1) == SP_OK); // TODO: Must be open.
	fail_unless(sp_nonblocking_read(NULL, &buf, 1) < 0);
	fail_unless(sp_nonblocking_read(port, NULL, 1) < 0);
	// fail_unless(sp_nonblocking_read(port, &buf, 0) == 0); // TODO

	// fail_unless(sp_input_waitig(port) == SP_OK); // TODO: Port must be open
	fail_unless(sp_input_waiting(NULL) < 0);
	// fail_unless(sp_output_waitig(port) == SP_OK); // TODO: Port must be open
	fail_unless(sp_output_waiting(NULL) < 0);

	fail_unless(sp_new_event_set(NULL) < 0);
	fail_unless(sp_new_event_set(&event_set) == SP_OK);
	sp_free_event_set(event_set);
	fail_unless(sp_new_event_set(NULL) < 0);

	fail_unless(sp_new_event_set(&event_set) == SP_OK);
	fail_unless(sp_add_port_events(NULL, port, SP_EVENT_ERROR) < 0);
	fail_unless(sp_add_port_events(event_set, NULL, SP_EVENT_ERROR) < 0);
	fail_unless(sp_add_port_events(event_set, port, 999) < 0);
	fail_unless(sp_add_port_events(event_set, port, SP_EVENT_ERROR) == SP_OK);
	fail_unless(sp_wait(event_set, 100) == SP_OK);
	// fail_unless(sp_wait(event_set, 0) == SP_OK); /* Won't return. */
	fail_unless(sp_wait(NULL, 100) < 0);
	sp_free_event_set(event_set);

	fail_unless(sp_get_signals(port, &signal_mask) < 0);
	fail_unless(sp_get_signals(NULL, &signal_mask) < 0);
	fail_unless(sp_get_signals(port, NULL) < 0);
	fail_unless(sp_get_signals(NULL, NULL) < 0);

	// fail_unless(sp_start_break(port) == SP_OK); // Needs real, open port?
	fail_unless(sp_start_break(NULL) < 0);

	// fail_unless(sp_end_break(port) == SP_OK); // Needs real, open port?
	fail_unless(sp_end_break(NULL) < 0);

#if 0
	fail_unless(sp_get_port_by_name("/dev/foobar", &port) < 0);
//	fail_unless(sp_open("/dev/foobar", SP_MODE_READ) < 0);
	fail_unless(sp_last_error_code() < 0);
#endif

	fail_unless((s = sp_last_error_message()) != NULL);
	sp_free_error_message(s);

	sp_free_error_message(NULL); /* Should not segfault. */

	sp_set_debug_handler(handler);
	sp_set_debug_handler(NULL); /* Should not segfault. */
	sp_set_debug_handler(sp_default_debug_handler);

	sp_default_debug_handler("%s", "foo");
	sp_default_debug_handler("%d", 100);
	sp_default_debug_handler(NULL); /* Should not segfault. */

	fail_unless(sp_new_config(&config) == SP_OK);
	fail_unless(sp_get_config_baudrate(config, &baudrate) == SP_OK);
	fail_unless(sp_get_config_baudrate(NULL, &baudrate) < 0);
	fail_unless(sp_get_config_baudrate(config, NULL) < 0);
	fail_unless(sp_get_config_bits(config, &bits) == SP_OK);
	fail_unless(sp_get_config_bits(NULL, &bits) < 0);
	fail_unless(sp_get_config_bits(config, NULL) < 0);
	sp_free_config(config);

	fail_unless(sp_set_config_flowcontrol(config, SP_FLOWCONTROL_NONE) == SP_OK);

	// fail_unless(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE) == SP_OK); // Needs open port.
	fail_unless(sp_set_flowcontrol(NULL, SP_FLOWCONTROL_NONE) < 0);

	// ret = sp_get_port_handle(port, &fd);
	// fail_unless(ret == SP_OK);
}
END_TEST

Suite *suite_core(void)
{
	Suite *s;
	TCase *tc;

	s = suite_create("core");

	tc = tcase_create("ports");
	tcase_add_test(tc, test_sp_get_port_by_name);
	tcase_add_test(tc, test_null_checks);
	suite_add_tcase(s, tc);

	return s;
}
