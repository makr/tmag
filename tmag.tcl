# --------------------------------------------------------------------------
# -- tmag.tcl
#
#	This file implements a Tcl interface to the libmagic functions.
#
# Copyright © 2008-2009 Matthias Kraft <M.Kraft@gmx.com>.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# --------------------------------------------------------------------------

package require critcl

# Header files: string header, Tcl header, tmag header
# FIXME: Tcl header required? better use critcl::cheaders for tmag.h?
::critcl::ccode {
#include <string.h>
#include <tcl.h>
#include "tmag.h"
}

# --------------------------------------------------------------------------
# TmagSetErrorResult --
#
# Create Tcl objects from strings and set them as interp result.
# --------------------------------------------------------------------------

::critcl::ccode {
void TmagSetErrorResult(Tcl_Interp *interp, const char *message, const char *reason) {
  Tcl_Obj *result_obj;

  result_obj = Tcl_NewStringObj(message, -1);
  Tcl_AppendToObj(result_obj, reason, -1);
  Tcl_SetObjResult(interp, result_obj);
}
}

# --------------------------------------------------------------------------
# TmagSessionInit --
#
# Initialize a new libmagic session and load the default database.
# --------------------------------------------------------------------------

::critcl::ccode {
magic_t TmagSessionInit(Tcl_Interp *interp, int flags) {
  magic_t magic_h;

  if ((magic_h = magic_open(TMAG_STANDARD_FLAGS|flags)) == NULL) {
    TmagSetErrorResult(interp, TMAG_OPEN_ERROR_MSG, magic_error(magic_h));

  } else if ((magic_load(magic_h, NULL)) != 0) {
    TmagSetErrorResult(interp, TMAG_LOAD_ERROR_MSG, magic_error(magic_h));
    magic_close(magic_h);
    magic_h = NULL;
  }

  return magic_h;
}
}

# --------------------------------------------------------------------------
# TmagFileCmd --
#
# Implements Tcl command
# libmagic::filetype filename ?(-isbuffer|-follow)? ?-all?
#
# Returns result string from libmagic or an error.
# --------------------------------------------------------------------------
# FIXME: change to ::critcl::cproc!

::critcl::ccode {
int TmagFileCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    tmagData *session = (tmagData *)data;
    const char *result_str;
    CONST char *fname_str=NULL;
    const unsigned char *buffer_ptr=NULL;
    int buffer_len=0, rc=TCL_OK, flag_isbuffer=0, flag_follow=0, magic_flags=TMAG_STANDARD_FLAGS;

    /* possible flags */
    const char *flags[] = {"-isbuffer", "-follow", "-all", "-raw", NULL};
    enum flagsIdx {IsBufferIdx, FollowIdx, AllIdx, RawIdx};

    /* check possible range of arguments and process them,
     * note that -isbuffer and -follow are mutual exclusive */
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
	magic_flags |= MAGIC_CONTINUE;
	break;
      case RawIdx:
	magic_flags |= MAGIC_RAW;
      }
      objc--;
    }
    if (flag_follow && flag_isbuffer) {
      TmagSetErrorResult(interp, TMAG_ILL_FLAGS_MSG, TMAG_ILL_FLAGS_RSN);
      return TCL_ERROR;
    }
    if (flag_isbuffer) {
      buffer_ptr = Tcl_GetByteArrayFromObj(objv[1], &buffer_len);
    } else {
      fname_str = Tcl_GetString(objv[1]);
    }

    /* now that we have all required input, configure libmagic */
    if ((magic_setflags(session->magic_h, magic_flags)) < 0) {
      TmagSetErrorResult(interp, TMAG_FLAGS_ERROR_MSG, magic_error(session->magic_h));
      return TCL_ERROR;
    }

    /* identify content */
    if (flag_isbuffer) {
      result_str = magic_buffer(session->magic_h, (const void *)buffer_ptr, buffer_len);
    } else {
      result_str = magic_file(session->magic_h, fname_str);
    }

    /* check for error, setup result, close libmagic session */
    if (result_str == NULL) {
      int eno;
      if ((eno = magic_errno(session->magic_h)) == 0) {
	TmagSetErrorResult(interp, (flag_isbuffer ? TMAG_BUFFER_ERROR_MSG : TMAG_FILE_ERROR_MSG), magic_error(session->magic_h) );
      } else {
	TmagSetErrorResult(interp, (flag_isbuffer ? TMAG_BUFFER_ERROR_MSG : TMAG_FILE_ERROR_MSG), strerror(eno) );
      }
      rc = TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewStringObj(result_str, -1));
    }
    return rc;
}
}

# --------------------------------------------------------------------------
# TmagMimeCmd --
#
# Implements Tcl command
# libmagic::mimetype filename ?(-isbuffer|-follow)? ?-all? ?-with-encoding?
#
# Returns the mime type as result string, with a string identifying the
# encoding optionally appended. May return an error.
# --------------------------------------------------------------------------
# FIXME: change to critcl::cproc!
::critcl::ccode {
int TmagMimeCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    tmagData *session = (tmagData *)data;
    const char *result_str;
    CONST char *fname_str=NULL;
    const unsigned char *buffer_ptr=NULL;
    int buffer_len=0, rc=TCL_OK, flag_isbuffer=0, flag_follow=0, magic_flags=(TMAG_STANDARD_FLAGS|MAGIC_MIME_TYPE);

    /* possible flags */
    const char *flags[] = {"-isbuffer", "-follow", "-all", "-with-encoding", NULL};
    enum flagsIdx {IsBufferIdx, FollowIdx, AllIdx, WithEncodingIdx};

    /* check possible range of arguments and process them,
     * note that -isbuffer and -follow are mutual exclusive */
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
	magic_flags |= MAGIC_CONTINUE;
	break;
      case WithEncodingIdx:
	magic_flags |= MAGIC_MIME_ENCODING;
	break;
      }
      objc--;
    }
    if (flag_follow && flag_isbuffer) {
      TmagSetErrorResult(interp, TMAG_ILL_FLAGS_MSG, TMAG_ILL_FLAGS_RSN);
      return TCL_ERROR;
    }
    if (flag_isbuffer) {
      buffer_ptr = Tcl_GetByteArrayFromObj(objv[1], &buffer_len);
    } else {
      fname_str = Tcl_GetString(objv[1]);
    }

    /* now that we have all required input, configure libmagic */
    if ((magic_setflags(session->magic_h, magic_flags)) < 0) {
      TmagSetErrorResult(interp, TMAG_FLAGS_ERROR_MSG, magic_error(session->magic_h));
      return TCL_ERROR;
    }

    /* identify content */
    if (flag_isbuffer) {
      result_str = magic_buffer(session->magic_h, (const void *)buffer_ptr, buffer_len);
    } else {
      result_str = magic_file(session->magic_h, fname_str);
    }

    /* check for error, setup result, close libmagic session */
    if (result_str == NULL) {
      int eno;
      if ((eno = magic_errno(session->magic_h)) == 0) {
	TmagSetErrorResult(interp, (flag_isbuffer ? TMAG_BUFFER_ERROR_MSG : TMAG_FILE_ERROR_MSG), magic_error(session->magic_h) );
      } else {
	TmagSetErrorResult(interp, (flag_isbuffer ? TMAG_BUFFER_ERROR_MSG : TMAG_FILE_ERROR_MSG), strerror(eno) );
      }
      rc = TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewStringObj(result_str, -1));
    }
    return rc;
}
}

# --------------------------------------------------------------------------
# Tmag_Init --
#
# Register namespace, commands, and package with the Tcl interpreter.
# --------------------------------------------------------------------------
# FIXME: put into critcl::cinit?
::critcl::ccode {
int Tmag_Init(Tcl_Interp *interp) {
    Tcl_Namespace *nsPtr; /* pointer to hold our own new namespace */
    tmagData *session;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }

    if ((session = (tmagData *)Tcl_Alloc(sizeof(tmagData))) == NULL) {
      return TCL_ERROR;
    }

    /* now that we have all required input, init libmagic */
    if ((session->magic_h = TmagSessionInit(interp, 0)) == NULL) {
      Tcl_Free(session);
      return TCL_ERROR;
    }

    /* create the namespace */
    if ((nsPtr = Tcl_CreateNamespace(interp, TMAG_NS, NULL, NULL)) == NULL) {
      magic_close(session->magic_h);
      Tcl_Free(session);
      return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, TMAG_NS "::filetype", TmagFileCmd, (ClientData)session, NULL);
    Tcl_CreateObjCommand(interp, TMAG_NS "::mimetype", TmagMimeCmd, (ClientData)session, NULL);

    if (Tcl_PkgProvide(interp, TMAG_EXT_NAME, TMAG_EXT_VERSION) == TCL_ERROR) {
      /* close leaks */
        return TCL_ERROR;
    }

    return TCL_OK;
}
}

# close leak:
# - magic_close(session->magic_h)
# - Tcl_Free(session)
# FIXME: how to handle the unload stuff with critcl?

::critcl::ccode {
#if 10 * TCL_MAJOR_VERSION + TCL_MINOR_VERSION >= 85
int Tmag_Unload(Tcl_Interp *interp, int flags) {
  /* how do I get my hands on ClientData now? */
  /* check Tcl_(G|S)etAssocData() */
}
#endif /* Tcl 8.5 */
}
