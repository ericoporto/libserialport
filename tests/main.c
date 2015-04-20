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

Suite *suite_core(void);
Suite *suite_version(void);

int main(void)
{
	int ret;
	Suite *s;
	SRunner *srunner;

	s = suite_create("mastersuite");
	srunner = srunner_create(s);

	/* Add all testsuites to the master suite. */
	srunner_add_suite(srunner, suite_core());
	srunner_add_suite(srunner, suite_version());

	srunner_run_all(srunner, CK_VERBOSE);
	ret = srunner_ntests_failed(srunner);
	srunner_free(srunner);

	return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
