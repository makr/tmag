/*
 * tmag.c --
 *
 *	This file implements a Tcl interface to the libmagic functions.
 *
 * Copyright (c) 2008 Matthias Kraft <matzek@users.sourceforge.net>.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * [FIXME] refactor and make configurable, consider file(1) as reference
 */

#include <magic.h>

#include <tcl.h>

#include "tmag.h"

/* libmagic::filetype filename
 * see also documentation of fileutil::magic::filetype
 */
int TmagFileCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    magic_t magic_h;
    const char *result_str, *fname_str;
    Tcl_Obj *result_obj;
    int rc=TCL_OK;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "filename");
	return TCL_ERROR;
    }

    fname_str = Tcl_GetString(objv[1]);

    if ((magic_h = magic_open(MAGIC_SYMLINK)) == NULL) {
        result_obj = Tcl_NewStringObj("magic_open() failed: ", -1);
        Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
        Tcl_SetObjResult(interp, result_obj);
        return TCL_ERROR;
    }
    if ((magic_load(magic_h, NULL)) != 0) {
        result_obj = Tcl_NewStringObj("magic_load() failed: ", -1);
        Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
        Tcl_SetObjResult(interp, result_obj);
        magic_close(magic_h);
        return TCL_ERROR;
    }
    if ((result_str = magic_file(magic_h, fname_str)) == NULL) {
        result_obj = Tcl_NewStringObj("magic_file() failed: ", -1);
        Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
        rc = TCL_ERROR;
    } else {
        result_obj = Tcl_NewStringObj(result_str, -1);
    }
    magic_close(magic_h);
    Tcl_SetObjResult(interp, result_obj);
    return rc;
}

/* libmagic::mimetype filename
 * see also documentation of fileutil::magic::mimetype
 */
int TmagMimeCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    magic_t magic_h;
    const char *result_str, *fname_str;
    Tcl_Obj *result_obj;
    int rc=TCL_OK;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "filename");
	return TCL_ERROR;
    }

    fname_str = Tcl_GetString(objv[1]);

    if ((magic_h = magic_open(MAGIC_SYMLINK|MAGIC_MIME)) == NULL) {
        result_obj = Tcl_NewStringObj("magic_open() failed: ", -1);
        Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
        Tcl_SetObjResult(interp, result_obj);
        return TCL_ERROR;
    }
    if ((magic_load(magic_h, NULL)) != 0) {
        result_obj = Tcl_NewStringObj("magic_load() failed: ", -1);
        Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
        Tcl_SetObjResult(interp, result_obj);
        magic_close(magic_h);
        return TCL_ERROR;
    }
    if ((result_str = magic_file(magic_h, fname_str)) == NULL) {
        result_obj = Tcl_NewStringObj("magic_file() failed: ", -1);
        Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
        rc = TCL_ERROR;
    } else {
        result_obj = Tcl_NewStringObj(result_str, -1);
    }
    magic_close(magic_h);
    Tcl_SetObjResult(interp, result_obj);
    return rc;
}

/*
 * Tmag_Init --
 * Register namespace and commands with the tcl interpreter.
 */
int Tmag_Init(Tcl_Interp *interp) {
    Tcl_Namespace *nsPtr; /* pointer to hold our own new namespace */

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }

    /* create the namespace */
    if ((nsPtr = Tcl_CreateNamespace(interp, TMAG_NS, NULL, NULL)) == NULL) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, TMAG_NS "::filetype", TmagFileCmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, TMAG_NS "::mimetype", TmagMimeCmd, NULL, NULL);

    if (Tcl_PkgProvide(interp, TMAG_EXT_NAME, TMAG_EXT_VERSION) == TCL_ERROR) {
        return TCL_ERROR;
    }

    return TCL_OK;
}
