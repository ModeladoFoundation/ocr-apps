#ifndef _GETOPT_H
#define _GETOPT_H

#include <ocr.h>
#include <string.h>

static char *optarg = NULL;
static int optind = 1;

struct option {
    const char *name;
    /* has_arg can't be an enum because some compilers complain about
       type mismatches in all the code that assumes it is an int.  */
    int has_arg;
    int *flag;
    int val;
};

static inline int __attribute__((always_inline)) getopt_long(int argc, char * const argv[],
                                                             const char *optstring,
                                                             const struct option *longopts,
                                                             int *longindex) {
#warning "getopt_long not fully implemented, use at your own risk!"
    int ret = -1;
    int a = optind; ++optind;
    if(a<argc) {
        if(strlen(argv[a])==2) { //short
            int o;
            for(o=0; o<strlen(optstring) && optstring[o] != argv[a][1]; ++o);
            if(o<strlen(optstring)) {
                ret = argv[a][1];
                if(o+1<strlen(optstring) && optstring[o+1]==':') {
                    if(optind<argc && *argv[optind]!='-')
                        optarg = argv[optind]; ++optind;
                }
            }
        } else { //long
            int o;
            for(o=0; longopts[o].name && strcmp(longopts[o].name, argv[a]+2); ++o);
            if(longopts[o].name) {
                if(longopts[o].flag==NULL)
                    ret = longopts[o].val;
                else {
                    ret = 0;
                    *longopts[o].flag = longopts[o].val;
                }
                switch(longopts[o].has_arg) {
                case 2:
                    if(optind==argc || *argv[optind]=='-') break;
                case 1:
                    optarg = argv[optind]; ++optind;
                    break;
                case 0:
                default: ;
                }
            }
            if(longindex) *longindex=o;
        }
    }

    return ret;
}

#endif //_GETOPT_H
