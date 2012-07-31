/*
 * vim: noet ts=4 sw=4 sts=4
 *
 * keys.c - handle keyboard input
 * for tasknc
 * by mjheagle
 */

#define _GNU_SOURCE
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "keys.h"
#include "log.h"
#include "tasklist.h"
#include "tasknc.h"

/* keymap struct */
struct keymap
{
	int value;
	char *name;
};

/* keymaps {{{ */
struct keymap keymaps[] =
{
	{ERR, "ERR"},
	{KEY_SRESET, "sreset"},
	{KEY_RESET, "reset"},
	{KEY_DOWN, "down"},
	{KEY_UP, "up"},
	{KEY_LEFT, "left"},
	{KEY_RIGHT, "right"},
	{KEY_HOME, "home"},
	{KEY_BACKSPACE, "backspace"},
	{KEY_DL, "dl"},
	{KEY_IL, "il"},
	{KEY_DC, "dc"},
	{KEY_IC, "ic"},
	{KEY_EIC, "eic"},
	{KEY_CLEAR, "clear"},
	{KEY_EOS, "eos"},
	{KEY_EOL, "eol"},
	{KEY_SF, "sf"},
	{KEY_SR, "sr"},
	{KEY_NPAGE, "npage"},
	{KEY_PPAGE, "ppage"},
	{KEY_STAB, "stab"},
	{KEY_CTAB, "ctab"},
	{KEY_CATAB, "catab"},
	{KEY_ENTER, "enter"},
	{KEY_PRINT, "print"},
	{KEY_LL, "ll"},
	{KEY_A1, "a1"},
	{KEY_A3, "a3"},
	{KEY_B2, "b2"},
	{KEY_C1, "c1"},
	{KEY_C3, "c3"},
	{KEY_BTAB, "btab"},
	{KEY_BEG, "beg"},
	{KEY_CANCEL, "cancel"},
	{KEY_CLOSE, "close"},
	{KEY_COMMAND, "command"},
	{KEY_COPY, "copy"},
	{KEY_CREATE, "create"},
	{KEY_END, "end"},
	{KEY_EXIT, "exit"},
	{KEY_FIND, "find"},
	{KEY_HELP, "help"},
	{KEY_MARK, "mark"},
	{KEY_MESSAGE, "message"},
	{KEY_MOVE, "move"},
	{KEY_NEXT, "next"},
	{KEY_OPEN, "open"},
	{KEY_OPTIONS, "options"},
	{KEY_PREVIOUS, "previous"},
	{KEY_REDO, "redo"},
	{KEY_REFERENCE, "reference"},
	{KEY_REFRESH, "refresh"},
	{KEY_REPLACE, "replace"},
	{KEY_RESTART, "restart"},
	{KEY_RESUME, "resume"},
	{KEY_SAVE, "save"},
	{KEY_SBEG, "sbeg"},
	{KEY_SCANCEL, "scancel"},
	{KEY_SCOMMAND, "scommand"},
	{KEY_SCOPY, "scopy"},
	{KEY_SCREATE, "screate"},
	{KEY_SDC, "sdc"},
	{KEY_SDL, "sdl"},
	{KEY_SELECT, "select"},
	{KEY_SEND, "send"},
	{KEY_SEOL, "seol"},
	{KEY_SEXIT, "sexit"},
	{KEY_SFIND, "sfind"},
	{KEY_SHELP, "shelp"},
	{KEY_SHOME, "shome"},
	{KEY_SIC, "sic"},
	{KEY_SLEFT, "sleft"},
	{KEY_SMESSAGE, "smessage"},
	{KEY_SMOVE, "smove"},
	{KEY_SNEXT, "snext"},
	{KEY_SOPTIONS, "soptions"},
	{KEY_SPREVIOUS, "sprevious"},
	{KEY_SPRINT, "sprint"},
	{KEY_SREDO, "sredo"},
	{KEY_SREPLACE, "sreplace"},
	{KEY_SRIGHT, "sright"},
	{KEY_SRSUME, "srsume"},
	{KEY_SSAVE, "ssave"},
	{KEY_SSUSPEND, "ssuspend"},
	{KEY_SUNDO, "sundo"},
	{KEY_SUSPEND, "suspend"},
	{KEY_UNDO, "undo"},
	{KEY_MOUSE, "mouse"},
	{KEY_RESIZE, "resize"},
	{KEY_EVENT, "event"}
};
const int nkeys = sizeof(keymaps)/sizeof(struct keymap);
/* }}} */

void add_int_keybind(const int key, void *function, const int argint, const prog_mode mode) /* {{{ */
{
	/* convert integer to string, then add keybind */
	char *argstr;

	asprintf(&argstr, "%d", argint);
	add_keybind(key, function, argstr, mode);
	free(argstr);
} /* }}} */

void add_keybind(const int key, void *function, char *arg, const prog_mode mode) /* {{{ */
{
	/* add a keybind to the list of binds */
	keybind *this_bind, *new;
	int n = 0;
	char *modestr, *name;

	/* create new bind */
	new = calloc(1, sizeof(keybind));
	new->key = key;
	new->function = function;
	new->argint = 0;
	new->argstr = NULL;
	new->argstr = arg;
	new->next = NULL;
	new->mode = mode;

	/* append it to the list */
	if (keybinds==NULL)
		keybinds = new;
	else
	{
		this_bind = keybinds;
		for (n=0; this_bind->next!=NULL; n++)
			this_bind = this_bind->next;
		this_bind->next = new;
		n++;
	}

	/* write log */
	if (mode == MODE_PAGER)
		modestr = "pager - ";
	else if (mode == MODE_TASKLIST)
		modestr = "tasklist - ";
	else
		modestr = " ";
	name = name_key(key);
	tnc_fprintf(logfp, LOG_DEBUG, "bind #%d: key %s (%d) bound to @%p %s%s(args: %d/%s)", n, name, key, function, modestr, name_function(function), new->argint, new->argstr);
	free(name);
} /* }}} */

char *name_key(const int val) /* {{{ */
{
	/* return a string naming the key */
	char *name = NULL;
	int i, len;

	/* single char */
	if (val>31 && val<127)
	{
		name = calloc(5, sizeof(char));
		sprintf(name, "'%c'", val);
	}

	/* special value */
	else
	{
		for (i=0; i<nkeys; i++)
		{
			if (keymaps[i].value == val)
			{
				len = strlen(keymaps[i].name);
				name = calloc(len+3, sizeof(char));
				sprintf(name, "<%s>", keymaps[i].name);
				break;
			}
		}
	}

	/* fallback: integer -> string */
	if (name == NULL)
	{
		name = calloc(5, sizeof(char));
		sprintf(name, "%d", val);
	}
	return name;
} /* }}} */

int parse_key(const char *keystr) /* {{{ */
{
	/* determine a key value from a string specifier */
	int key;

	/* try for integer key */
	if (1==sscanf(keystr, "%d", &key))
		return key;

	/* take the first character as the key */
	return (int)(*keystr);
} /* }}} */

int remove_keybinds(const int key) /* {{{ */
{
	/* remove all keybinds to key */
	int counter = 0;
	keybind *this, *last = NULL, *next;

	this = keybinds;
	while (this!=NULL)
	{
		next = this->next;
		if (this->key == (int)key)
		{
			if (last!=NULL)
				last->next = next;
			else
				keybinds = next;
			free(this);
			counter++;
		}
		else
			last = this;
		this = next;
	}

	return counter;
} /* }}} */
