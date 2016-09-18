/*
*   $Id: sh.c 443 2006-05-30 04:37:13Z darren $
*
*   Copyright (c) 2000-2002, Darren Hiebert
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License.
*
*   This module contains functions for generating tags for scripts for the
*   Bourne shell (and its derivatives, the Korn and Z shells).
*/

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

#include <string.h>

#include "parse.h"
#include "read.h"
#include "routines.h"
#include "vstring.h"

/*
*   DATA DEFINITIONS
*/
typedef enum {
	K_FUNCTION
} shKind;

static kindOption ShKinds [] = {
	{ TRUE, 'f', "function", "functions"}
};

/*
*   FUNCTION DEFINITIONS
*/

/*  Reject any tag "main" from a file named "configure". These appear in
 *  here-documents in GNU autoconf scripts and will add a haystack to the
 *  needle.
 */
static boolean hackReject (const vString* const tagName)
{
	const char *const scriptName = baseFilename (vStringValue (File.name));
	boolean result = (boolean) (
			strcmp (scriptName, "configure") == 0  &&
			strcmp (vStringValue (tagName), "main") == 0);
	return result;
}

static void findShTags (void)
{
	vString *name = vStringNew ();
	const unsigned char *line;

	while ((line = fileReadLine ()) != NULL)
	{
		const unsigned char* cp = line;
		boolean functionFound = FALSE;

		if (line [0] == '#')
			continue;

        // Skip white space
		while (isspace (*cp))
			cp++;

        // String 'function ' in text?
		if (strncmp ((const char*) cp, "function", (size_t) 8) == 0  &&
			isspace ((int) cp [8]))
		{
			functionFound = TRUE;
			cp += 8;

            // Why the additional check?
			if (! isspace ((int) *cp))
				continue;

            // Skip any whitespaces after 'function '
			while (isspace ((int) *cp))
				++cp;
		}

        // After 'function +' if there's no [[:alnum:]_-] then restart
		if (! (isalnum ((int) *cp) || *cp == '_' || *cp == '-' ))
			continue;

        // LOAD [[:alnum:]_-]
		while (isalnum ((int) *cp)  ||  *cp == '_' || *cp == '-' ) {
			vStringPut (name, (int) *cp);
			++cp;
		}
		vStringTerminate (name);

        // Skip spaces after [[:alnum:]_-]
		while (isspace ((int) *cp))
			++cp;

        // Detection of function not necessarily beginning with "function"
		if (*cp++ == '(')
		{
			while (isspace ((int) *cp))
				++cp;
			if (*cp == ')'  && ! hackReject (name))
				functionFound = TRUE;
		}

        // Function found?
		if (functionFound)
			makeSimpleTag (name, ShKinds, K_FUNCTION);

        // Forget the function
		vStringClear (name);

	}
	vStringDelete (name);
}

extern parserDefinition* ShParser (void)
{
	static const char *const extensions [] = {
		"sh", "SH", "bsh", "bash", "BASH", "ksh", "KSH", "zsh", "Zsh", "ZSH", NULL
	};
	parserDefinition* def = parserNew ("Sh");
	def->kinds      = ShKinds;
	def->kindCount  = KIND_COUNT (ShKinds);
	def->extensions = extensions;
	def->parser     = findShTags;
	return def;
}

/* vi:set tabstop=4 shiftwidth=4: */
