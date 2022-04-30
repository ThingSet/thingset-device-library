#ifndef __JSMN_H_
#define __JSMN_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JSMN_STRICT

/**
 * JSON type identifier.
 */
typedef enum {
	/** Undefined type */
	JSMN_UNDEFINED = 0,
	/** JSON object */
	JSMN_OBJECT = 1,
	/** JSON array */
	JSMN_ARRAY = 2,
	/** JSON string */
	JSMN_STRING = 3,
	/** JSON primitive value: number, boolean (true/false) or null */
	JSMN_PRIMITIVE = 4
} jsmntype_t;

enum jsmnerr {
	/** Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/** Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/** The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 */
typedef struct {
	jsmntype_t type;	/**< Type (object, array, string etc.) */
	int16_t start;		/**< Start position in JSON data string */
	int16_t end;		/**< End position in JSON data string */
	int16_t size;		/**< Size of the token (in bytes) */
#ifdef JSMN_PARENT_LINKS
	int16_t parent;		/**< Parent token */
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
	int16_t pos;      /**< Offset in the JSON string */
	int16_t toknext;  /**< Next token to allocate */
	int16_t toksuper; /**< Superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */
