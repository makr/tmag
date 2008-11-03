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
int TmagTypeCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    magic_t magic_h;
    const char *result_str, *fname_str;
    Tcl_Obj *result_obj;
    int rc=TCL_OK;
    const char *options[] = {"-file", "-i", "-h", "-e", "-L", "-p", "-s", "-z", NULL};
    enum keywordIdx {file, mime, no_dereference, exclude, dereference, preserve_date, special_files, uncompress};

    /* this will take the first argument given to the Tcl level command
       and compare it with the keywords listed in subCmds.
       If there is no match, and error is raised and an error message
       is automatically generated with the word 'action' -> bad action "...": must be start, stop, or rewind
       If there is a match, the index of the matched item in the arry is assigned to the 'index' variable */
    int index;
    if (Tcl_GetIndexFromObj(interp, objv[1], options, "action", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    /* now use the enumeration to do your work depending on the keyword */
    switch (index) {
            case startIdx: {
                 /* do the start action */
            }
            case stopIdx: {
                /* do the stop action */
            }
            case rewindIdx: {
                /* do rewind action here */
            }
    }

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

    Tcl_CreateObjCommand(interp, TMAG_NS "::type", TmagFileCmd, NULL, NULL);

    if (Tcl_PkgProvide(interp, TMAG_EXT_NAME, TMAG_EXT_VERSION) == TCL_ERROR) {
        return TCL_ERROR;
    }

    return TCL_OK;
}
