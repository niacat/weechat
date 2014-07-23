/*
 * tests.cpp - run WeeChat tests
 *
 * Copyright (C) 2014 Sébastien Helleu <flashcode@flashtux.org>
 *
 * This file is part of WeeChat, the extensible chat client.
 *
 * WeeChat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * WeeChat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WeeChat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

extern "C"
{
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#include "../src/core/weechat.h"
#include "../src/core/wee-hook.h"
#include "../src/core/wee-input.h"
#include "../src/plugins/plugin.h"
#include "../src/gui/gui-main.h"
#include "../src/gui/gui-buffer.h"
#include "../src/gui/curses/gui-curses.h"
}

#include "CppUTest/CommandLineTestRunner.h"

/* import tests from libs */
IMPORT_TEST_GROUP(Eval);
IMPORT_TEST_GROUP(Hashtable);
IMPORT_TEST_GROUP(Hdata);
IMPORT_TEST_GROUP(Infolist);
IMPORT_TEST_GROUP(List);
IMPORT_TEST_GROUP(String);
IMPORT_TEST_GROUP(Url);
IMPORT_TEST_GROUP(Utf8);
IMPORT_TEST_GROUP(Util);


/*
 * Callback for any message displayed by WeeChat or a plugin.
 */

int
test_print_cb (void *data, struct t_gui_buffer *buffer,
               time_t date, int tags_count,
               const char **tags, int displayed,
               int highlight, const char *prefix,
               const char *message)
{
    /* make C compiler happy */
    (void) data;
    (void) buffer;
    (void) date;
    (void) tags_count;
    (void) tags;
    (void) displayed;
    (void) highlight;

    printf ("%s%s%s\n",  /* with color: "\33[34m%s%s%s\33[0m\n" */
            (prefix && prefix[0]) ? prefix : "",
            (prefix && prefix[0]) ? " " : "",
            (message && message[0]) ? message : "");

    return WEECHAT_RC_OK;
}

/*
 * Initializes GUI for tests.
 */

void
test_gui_init ()
{
    /*
     * Catch all messages to display them directly on stdout
     * (Curses library is not used for tests).
     */
    hook_print (NULL,  /* plugin */
                NULL,  /* buffer */
                NULL,  /* tags */
                NULL,  /* message */
                1,     /* strip colors */
                &test_print_cb,
                NULL);

    /*
     * Call the function "gui_main_init" from Curses sources (all Curses
     * calls are made with the fake ncurses library).
     */
    gui_main_init ();
}

/*
 * Runs tests in WeeChat environment.
 */

int
main (int argc, char *argv[])
{
    int rc;
    const char *weechat_argv[] = { NULL, "--dir", NULL, NULL };

    /* setup environment: default language, no specific timezone */
    setenv ("LANG", "C", 1);
    setenv ("TZ", "", 1);

    /* command line arguments: "<this_binary> --dir ./tmp_weechat_test" */
    weechat_argv[0] = argv[0];
    weechat_argv[2] = "./tmp_weechat_test";

    /* init WeeChat */
    weechat_init (3, (char **)weechat_argv, &test_gui_init);

    /* display WeeChat version */
    input_data (gui_buffer_search_main (), "/command core version");

    /* run all tests */
    printf ("\n");
    printf (">>>>>>>>>> TESTS >>>>>>>>>>\n");
    rc = CommandLineTestRunner::RunAllTests (argc, argv);
    printf ("<<<<<<<<<< TESTS <<<<<<<<<<\n");
    printf ("\n");

    /* end WeeChat */
    weechat_end (&gui_main_end);

    /* display status */
    printf ("\n");
    printf ("\33[%d;1m*** %s ***\33[0m\n",
            (rc == 0) ? 32 : 31,  /* 32 = green (OK), 31 = red (error) */
            (rc == 0) ? "OK" : "ERROR");

    return rc;
}