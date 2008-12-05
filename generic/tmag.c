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

/* open and init lib magic
 */

magic_t TmagInit(Tcl_Interp *interp, int flags) {
  magic_t magic_h;
  Tcl_Obj *result_obj;

  if ((magic_h = magic_open(MAGIC_ERROR|MAGIC_RAW|flags)) == NULL) {
    result_obj = Tcl_NewStringObj("inititializing libmagic failed: ", -1);
    Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
    Tcl_SetObjResult(interp, result_obj);
    return NULL;
  }

  /* would use whatever compiled in default database is, or if MAGIC environment
   * variable is set, uses this */
  if ((magic_load(magic_h, NULL)) != 0) {
    result_obj = Tcl_NewStringObj("loading the default libmagic database failed: ", -1);
    Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
    Tcl_SetObjResult(interp, result_obj);
    magic_close(magic_h);
    return NULL;
  }

  return magic_h;
}

/* libmagic::filetype filename ?(-isbuffer|-follow)? ?-all?
 * 
 */
int TmagTypeCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    magic_t magic_h;
    const char *result_str, *fname_str, *flags[] = {"-isbuffer", "-follow", "-all", NULL};
    const unsigned char *buffer_ptr;
    enum flagIdx {IsBufferIdx, FollowIdx, AllIdx};
    Tcl_Obj *result_obj;
    int buffer_len, rc=TCL_OK, flag_isbuffer=0, flag_follow=0, flag_all=0, magic_flags=0;

    if (objc < 2 || objc > 4) {
	Tcl_WrongNumArgs(interp, 1, objv, "filename ?flags ...?");
	return TCL_ERROR;
    }
    while (objc > 2) {
      int idx;
      if (Tcl_GetIndexFromObj(interp, objv[objc-1], flags, "flag", 0, &idx) != TCL_OK) {
	return TCL_ERROR;
      }
      switch (idx) {
      case IsBufferIdx:
	flag_isbuffer = 1;
	break;
      case FollowIdx:
	flag_follow = 1;
	magic_flags |= MAGIC_SYMLINK;
	break;
      case AllIdx:
	flag_all = 1;
	magic_flags |= MAGIC_CONTINUE;
	break;
      }
      objc--;
    }
    if (flag_follow && flag_isbuffer) {
      Tcl_SetObjResult(interp, Tcl_NewStringObj( "cannot resolve symlinks on buffer", -1 ));
      return TCL_ERROR;
    }

    if (flag_isbuffer) {
      buffer_ptr = Tcl_GetByteArrayFromObj(objv[1], &buffer_len);
    } else {
      fname_str = Tcl_GetString(objv[1]);
    }

    if ((magic_h = TmagInit(interp, magic_flags)) == NULL) {
        return TCL_ERROR;
    }

    if (flag_isbuffer) {
      result_str = magic_buffer(magic_h, (const void *)buffer_ptr, buffer_len);
    } else {
      result_str = magic_file(magic_h, fname_str);
    }

    if (result_str == NULL) {
      char * errmsg = (flag_isbuffer ? "buffer type identification failed: " : "file type identification failed: ");
      result_obj = Tcl_NewStringObj(errmsg, -1);
      Tcl_AppendToObj(result_obj, magic_error(magic_h), -1);
      rc = TCL_ERROR;
    } else {
      result_obj = Tcl_NewStringObj(result_str, -1);
    }
    magic_close(magic_h);
    Tcl_SetObjResult(interp, result_obj);
    return rc;
}

/* libmagic::mimetype filename ?(-isbuffer|-follow)? ?-all? ?-with-encoding?
 * see also documentation of fileutil::magic::mimetype
 */
int TmagMimeCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    magic_t magic_h;
    const char *result_str, *fname_str, *flags[] = {"-isbuffer", "-follow", "-all", "-with-encoding", NULL};
    const unsigned char *buffer_ptr;
    enum flagIdx {IsBufferIdx, FollowIdx, AllIdx, WithEncodingIdx};
    Tcl_Obj *result_obj;
    int buffer_len, rc=TCL_OK, flag_isbuffer=0, flag_follow=0, flag_all=0, flag_with_encoding=0, magic_flags=MAGIC_MIME_TYPE;

    if (objc < 2 || objc > 5) {
	Tcl_WrongNumArgs(interp, 1, objv, "filename ?flags ...?");
	return TCL_ERROR;
    }
    while (objc > 2) {
      int idx;
      if (Tcl_GetIndexFromObj(interp, objv[objc-1], flags, "flag", 0, &idx) != TCL_OK) {
	return TCL_ERROR;
      }
      switch (idx) {
      case IsBufferIdx:
	flag_isbuffer = 1;
	break;
      case FollowIdx:
	flag_follow = 1;
	magic_flags |= MAGIC_SYMLINK;
	break;
      case AllIdx:
	flag_all = 1;
	magic_flags |= MAGIC_CONTINUE;
	break;
      case WithEncodingIdx:
	flag_with_encoding = 1;
	magic_flags |= MAGIC_MIME_ENCODING;
	break;
      }
      objc--;
    }
    if (flag_follow && flag_isbuffer) {
      Tcl_SetObjResult(interp, Tcl_NewStringObj( "cannot resolve symlinks on buffer", -1 ));
      return TCL_ERROR;
    }

    if (flag_isbuffer) {
      buffer_ptr = Tcl_GetByteArrayFromObj(objv[1], &buffer_len);
    } else {
      fname_str = Tcl_GetString(objv[1]);
    }

    if ((magic_h = TmagInit(interp, magic_flags)) == NULL) {
        return TCL_ERROR;
    }

    if (flag_isbuffer) {
      result_str = magic_buffer(magic_h, (const void *)buffer_ptr, buffer_len);
    } else {
      result_str = magic_file(magic_h, fname_str);
    }

    if (result_str == NULL) {
      char * errmsg = (flag_isbuffer ? "buffer type identification failed: " : "file type identification failed: ");
      result_obj = Tcl_NewStringObj(errmsg, -1);
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
