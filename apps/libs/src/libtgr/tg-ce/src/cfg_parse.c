#include "tg-types.h"
#include "stddef.h"
//
// This file implements a SAX style ini file parser. No state is stored in
// the methods here.
//
typedef struct {
    const char * section;
    const char * key;
    const char * value;
} cfg_item;

static int iswsp( char c )
{
    return (c == ' ') | (c == '\t') | (c == '\r');
}

static char * trim( char * s )
{
    while( s && *s && iswsp(*s) )   // left trim
        s++;

    char *e = s;
    while( *e ) // find the end
        e++;
    while( --e > s && iswsp(*e) )   // right trim
        *e = '\0';

    return s;
}

//
// Parse a line from cfg file line providing item section, key, and value
// Comments starting with '#' are allowed, don't allow '#' in values
// This modifies the incoming data
//
// 's' is the start of the line to be parsed
// 'item' is modified to point to the parsed text
//      Section lines:
//          item->section is modified, item->key, item->value are set to NULL
//      Key/value lines:
//          item->key and item->value are set, item->section is not modified
//      Empty lines:
//          item->key, item->value are set to NULL, item->section is not modified
//
//  Returns a pointer to the start of next line or NULL if the 'file' is exhausted.
//
char * cfg_get_item( char * line, cfg_item * item )
{
    char * p = line;
    char * st = NULL;   // start of interesting line contents
    char * e = NULL;    // end of interesting line contents
    char * eq = NULL;   // marker for an equal sign (if any)
    //
    // NULL or empty line (not even a newline) is EOF
    //
    if( line == NULL ) {
        return NULL;
    }
    if( *line == '\0' ) {
        item->key = NULL;
        item->value = NULL;
        return NULL;
    }
    //
    // find the end of the line, dealing with other things as we go
    //
    while( *p && *p != '\n' ) {
        if( e == NULL && *p == '#' ) {
            e = p - 1;
            *p++ = '\0';
            continue;
        }
        if( e == NULL ) {
            if( st == NULL && ! iswsp(*p) ) // first non-wsp char in the line
                st = p;
            if( *p == '=' )
                eq = p;
        }
        p++;
    }
    if( *p )
        *p++ = '\0';
    //
    // empty line?
    //
    if( st == NULL || *st == '\0' ) {
        item->key = NULL;
        item->value = NULL;
        return p;
    }
    //
    // collect a section name
    //
    if( *st == '[' ) {
        item->section =  ++st;

        while( *st && *st != ']' )
            st++;
        *st = '\0';

        item->section = trim( (char *) item->section );

        item->key = NULL;
        item->value = NULL;
    //
    // key = value pair
    //
    } else {
        if( eq == NULL ) {  // illegal data
            item->section = NULL;
            item->key = NULL;
            item->value = NULL;
            return NULL;
        }
        *eq++ = '\0';
        //
        // collect the key
        //
        item->key = trim( st );
        item->value = trim( eq );
    }
    return p;
}

//
// section / key matching methods
//
// value conversion utility methods
//
