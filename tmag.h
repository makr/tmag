/* --------------------------------------------------------------------------
 * tmag.h --
 *
 *	This file contains definitions for the implemention of a
 *	Tcl interface to the libmagic functions.
 *
 * Copyright © 2008-2009 Matthias Kraft <M.Kraft@gmx.com>.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 * -------------------------------------------------------------------------- */

#ifndef TMAG_H
#define TMAG_H

/* libmagic header */
#include <magic.h>

/* package definition, from -D options*/
#define TMAG_EXT_NAME		PACKAGE_NAME
#define TMAG_EXT_VERSION	PACKAGE_VERSION

/* namespace definition */
#define TMAG_NS			"libmagic"

/* error message definitions */
#define TMAG_OPEN_ERROR_MSG	"initializing new libmagic session failed: "
#define TMAG_LOAD_ERROR_MSG	"loading the default libmagic database failed: "
#define TMAG_BUFFER_ERROR_MSG	"buffer type identification failed: "
#define TMAG_FILE_ERROR_MSG	"file type identification failed: "
#define TMAG_FLAGS_ERROR_MSG	"configuring libmagic query: "

#define TMAG_ILL_FLAGS_MSG	"illegal combination of flags: "
#define TMAG_ILL_FLAGS_RSN	"cannot resolve symlinks on buffer input"

/* data definitions */
typedef struct tmagData {
  magic_t magic_h;
} tmagData;

#define TMAG_STANDARD_FLAGS	(MAGIC_ERROR)

#endif /* TMAG_H */
