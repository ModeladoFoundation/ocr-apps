#ifndef _ANSI_H_
#define _ANSI_H_

#define _PTR        void *
#define _AND        ,
#define _NOARGS     void
#define _CONST      const
#define _VOLATILE   volatile
#define _SIGNED     signed
#define _DOTS       , ...
#define _VOID       void
#define _EXFUN_NOTHROW(name, proto) name proto _NOTHROW
#define _EXFUN(name, proto)         name proto
#define _EXPARM(name, proto)        (* name) proto
#define _EXFNPTR(name, proto)       (* name) proto
#define _DEFUN(name, arglist, args) name(args)
#define _DEFUN_VOID(name)           name(_NOARGS)

#define weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));

#endif // _ANSI_H_
