#include <stdint.h>
#include <sys/time.h>

#include <tg-types.h>
#include <tg-cmdline.h>
#include <tg-console.h>

#include "tgr-ce.h"
#include "util.h"
#include "mem-util.h"
#include "cfg_parse.h"
//
// This file implements an ini file parser.
// Usage:
//  The xx() method will read in a named ini file, parse it into a db
//  and return it. The yy() method can be used to search the db by
//  section and key.
//
struct cfg_db {
    cfg_item * items;       // pointer to array of items after this db
    size_t     nitems;      // number of parsed items
    mem_seg *  db_seg;      // segment referencing the allocated db
    mem_seg *  file_seg;    // segment referencing the allocated file data
    char *     cfg_start;   // start of cfg file data
    size_t     cfg_size;    // size of cfg file data
};

//
// Utility methods
//
static int streq( const char * s1, const char * s2 )
{
    if( s1 == NULL || s2 == NULL )
        return 0;
    while( *s1 || *s2 )
        if( *s1++ != *s2++ )
            return 0;
    return 1;
}

static int streqn( const char * s1, const char * s2, int n )
{
    if( s1 == NULL || s2 == NULL )
        return 0;
    while( n-- > 0 )
        if( *s1++ != *s2++ )
            return 0;
    return 1;
}

static int iswsp( char c )
{
    return (c == ' ') | (c == '\t') | (c == '\r');
}

static char * skipwsp( char * s )
{
    if( s ) {   // paranoid
        while( *s && iswsp(*s) )
            s++;
    }
    return s;
}

static char * nextline( char * s )
{
    while( *s && *s != '\n' )
        s++;
    return *s ? s + 1 : s;
}

static char * trim( char * s )
{
    if( s == NULL )
        return s;

    s = skipwsp(s); // left trim
        s++;

    char *e = s;
    while( *e )  // find the end
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
static char * cfg_get_item( char * line, cfg_item * item )
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
    //
    // terminate the line
    //
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
// Create a cfg_db holding the parsed contents of the cfg file
// contents starting at 'start'
// Returns the db or NULL if there was an allocation of syntax error
//
cfg_db * cfg_parse( ce_info * cei, char * start, size_t size )
{
    int lno = 1;
    size_t nitems = 0;
    char *s = start;
    //
    // count the number entries (not section or comment lines)
    //
    for( ; s < start + size && *s ; lno++ ) {
        s = skipwsp(s);
        if( *s ) {
            if (*s != '#' && *s != '[')
                nitems++;
            s = nextline(s);
        }
    }
    //
    // and allocate our db and items (which start immediately after the db)
    //
    mem_seg * seg = mem_alloc( cei->CE, sizeof(cfg_db) + nitems * sizeof(cfg_item) );
    if( seg == NULL ) {
        //
        // ERROR, ERROR, ...
        //
        return NULL;
    }
    cfg_db * db = (cfg_db *) seg->va;

    db->nitems = nitems;
    db->items  = (cfg_item *) (db + 1);
    db->db_seg = seg;
    db->file_seg = NULL;
    db->cfg_start = start;
    db->cfg_size  = size;

    memset( db->items, 0, nitems * sizeof(cfg_item) );
    //
    // walk through parsing the lines and populating the items
    //
    cfg_item * item = db->items;
    s = start;
    lno = 1;

    for( ; s && *s ; lno++ ) {
        s = cfg_get_item( s, item );
        //
        // key/value
        //
        if( item->key && item->value ) {
            if( item->section == NULL ) {
                //
                // illegal key/value outside of a section
                //
                ce_error( "CFG", "key/value outside of section, line %d\n", lno );
                cfg_destroy(db);
                return NULL;
            }
            //
            // item captured, move on to next item
            //
            (item + 1)->section = item->section;
            item++;
        }
        //
        // error - not an empty line or section header
        //
        else if( s == NULL || item->key || item->value ) {
            ce_error( "CFG", "syntax error line %d\n", lno );
            cfg_destroy(db);
            return NULL;
        }
    }
    ce_vprint( "CFG", "%d lines, %d est items, %d items found\n",
                lno, db->nitems, item - db->items );
    db->nitems = item - db->items;

    return db;
}

//
// Create a cfg_db holding the parsed contents of the cfg file 'fname'
// Returns the db or NULL if there was a file reading, allocation, or syntax error
//
cfg_db * cfg_parse_file( ce_info * cei, char * fname )
{
    //
    // open the file and read it into CE memory
    //
    mem_seg * file_seg = tgr_load_file( cei, fname );

    if( file_seg == NULL ) {
        ce_error( "CFG", "can't get cfg file '%s'\n", fname );
        return NULL;
    }
    cfg_db * db = cfg_parse( cei, (char *) file_seg->va, file_seg->len );

    if( db ) {
        db->file_seg = file_seg;
    }
    return db;
}

//
// free memory associated with the db (and file if defined)
//
void cfg_destroy( cfg_db * db )
{
    if( db != NULL ) {
        if( db->file_seg != NULL )
            mem_free( db->file_seg );
        if( db->db_seg != NULL )
            mem_free( db->db_seg );
    }
}

//
// section / key matching methods
// Both in-file section and keys as well as search section/keys can be formed
// as patterns.
// Section/Key pattern syntax:
//  Sections and keys (S/K) can be a single string or multi-part separated by '.'s
//  Patterns (supplied to the matching methods) areas follows:
//  Definitions:
//      <empty-string>  - a string with just a terminator (not NULL)
//                        this matches anything at the same part offset
//      <string>        - a non-empty string with no '.'s
//                        only matches empty string or exact same string
//      <estring>       - an empty or non-empty string
//  Syntax:
//      <empty-string>              - matches all (single-part?) S/K
//      <string>                    - matches a whole single-part S/K (no '.'
//                                    in the S/K)
//      <estring>[.<estring>]*      - matches a multi-part S/K of the same
//                                    number of parts.
//      <estring>[.<estring>]*...   - matches the a multi-part S/K beginning
//                                    with the supplied parts.
//     An <empty-string> matches anything.
//     A pattern with just ... is interpreted as matching any 4 part S/K
//

typedef struct {
    const char *start;  // pointer to the first char of the current token
    int len;            // number of chars in the current token (no '\0')
    int flags;          // special state indications
#define CT_INITIAL      1   // this is the initial token
#define CT_MATCH_ANY    2   // this is an empty string (matches anything)
#define CT_TERM_ANY     4   // this is a terminal ... (matches anything)
} cfg_token;

#define TOK_SEP             '.'
#define TOK_ANY             "..."
#define tok_match_any(t)    ((t)->flags & CT_MATCH_ANY)
#define tok_term_any(t)     ((t)->flags & CT_TERM_ANY)
#define tok_at_end(t)       (*(t)->start == '\0' )

//
// Get the next token
// 'str' is only used on the initial call for a parsed string.
// If 'str' is NULL, advance 'start' by 'len' and eat the '.'
// Returns 0 on success, 1 on error (syntax error)
//
static int cfg_next_tok( cfg_token * tok, const char *str )
{
    const char *s;

    if( str != NULL ) {
        tok->start = s = str;
        tok->len = 0;
        tok->flags = CT_INITIAL;

    } else if( tok->flags & CT_TERM_ANY ) {
        return 0;
    //
    // an error to try to go past the end
    //
    } else if( tok_at_end(tok) ) {
        return 1;

    } else {
        s = tok->start + tok->len;
        if( *s == TOK_SEP )
            s++;
        tok->start = s;
        tok->flags &= ~CT_INITIAL;
    }
    //
    // are we at 'DOTDOTDOT$ ? (special case at start of str)
    //
    if( ! (tok->flags & CT_INITIAL) && streq( s, TOK_ANY ) ) {
        tok->flags |= CT_MATCH_ANY | CT_TERM_ANY;
        return 0;
    }
    while( *s && *s != TOK_SEP )
        s++;
    tok->len = s - tok->start;

    if( tok->len == 0 )
        tok->flags |= CT_MATCH_ANY;
    else
        tok->flags &= ~CT_MATCH_ANY;

    return 0;
}

//
// Compare the tokens
// Return 1 on equal, 0 otherwise
//
static int cfg_tok_cmp( cfg_token * t1, cfg_token * t2 )
{
    return tok_match_any(t1) ||
           tok_match_any(t2) ||
           (t1->len == t2->len && streqn( t1->start, t2->start, t1->len));
}

int cfg_match_str( const char *pat, const char *str )
{
    cfg_token pat_tok;
    cfg_token str_tok;

    memset( & pat_tok, 0, sizeof(pat_tok) );
    memset( & str_tok, 0, sizeof(str_tok) );

    if( pat == NULL || str == NULL )
        return 0;
    //
    // Walk the pattern and str comparing tokens until both exhausted (success)
    // or one runs out of tokens (fail)
    //
    if( cfg_next_tok( &pat_tok, pat ) || cfg_next_tok( &str_tok, str ) )
        return 0;

    while( 1 ) {
        //
        // termination test
        // if both at dotdotdot
        // if one at dotdotdot and other at end
        // if both at end
        //
        if( (tok_term_any(&pat_tok) || tok_at_end(&pat_tok)) &&
            (tok_term_any(&str_tok) || tok_at_end(&str_tok)) )
            break;

        if( cfg_tok_cmp( & pat_tok, & str_tok ) )
            return 0;
        //
        // progress to the next tokens
        //
        if( cfg_next_tok( &pat_tok, NULL ) || cfg_next_tok( &str_tok, NULL ) )
            return 0;

    }
    return 1;
}
//
// cfg find callback
// Returns 1 to stop search
//
typedef int (* cfg_find_cb)( const cfg_item *, void * );
//
// Search the 'db' for entries matching 'section' and 'key'.
// The callback 'fn' is called for each matching entry
// Returns the number of entries found.
//
int cfg_find_items( cfg_db * db, const char * section, const char * key,
                    cfg_find_cb fn, void * closure )
{
    int found = 0;

    for( cfg_item * ci = db->items ; ci < db->items + db->nitems ; ci++ ) {
        if( cfg_match_str( ci->section, section) &&
            cfg_match_str( ci->key, key) &&
            fn( ci, closure) )
                return ++found;
    }
    return 0;
}

typedef struct {
    int flags;
#define  CGD_UINT64     1
#define  CGD_STRING     2
    const char * value;
} cfg_get_data;

static int cfg_get_cb( const cfg_item * ci, void * closure )
{
    cfg_get_data * cgd = closure;
    //
    // add code to cvt the str to the proper value
    //
    cgd->value = ci->value;

    return 1;
}

uint64_t cfg_get_uint64( cfg_db * db, const char * section, const char * key,
                         uint64_t def )
{
    cfg_get_data cgd;
    cgd.flags = CGD_UINT64;

    if( cfg_find_items( db, section, key, cfg_get_cb, & cgd ) == 0 )
        return def;
    return 0;
}

const char * cfg_get_string( cfg_db * db, const char * section, const char * key,
                             const char * def )
{
    return def;
}
//
// value conversion utility methods
//
