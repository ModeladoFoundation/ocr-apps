#ifndef __CFG_PARSE_H__
#define __CFG_PARSE_H__

//
// opaque object representing a parsed cfg file
//
typedef struct cfg_db cfg_db;

//
// Structure representing a parsed cfg db item
//
typedef struct {
    const char * section;
    const char * key;
    const char * value;
} cfg_item;

//
// Create a cfg_db holding the parsed contents of the cfg file
// contents starting at 'start'
// Returns the db or NULL if there was an allocation of syntax error
//
cfg_db * cfg_parse( ce_info * cei, char * start, size_t size );

//
// Create a cfg_db holding the parsed contents of the cfg file 'fname'
// Returns the db or NULL if there was a file reading, allocation, or syntax error
//
cfg_db * cfg_parse_file( ce_info * cei, char * fname );

//
// free memory associated with the db (and file if defined)
//
void cfg_destroy( cfg_db * db );

#endif // __CFG_PARSE_H__
