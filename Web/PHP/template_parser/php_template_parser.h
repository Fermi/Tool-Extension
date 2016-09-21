#ifndef _PHP_TEMPLATE_PARSER_H
#define _PHP_TEMPLATE_PARSER_H

#include "php.h"

#ifdef ZTS
#include "TSRM.h"
#endif

extern zend_module_entry template_parser_module_entry;

#define PHP_TEMPLATE_PARSER_VERSION "2.0"

typedef struct _TEMPLATE_PARSER_PARSE_RESULT_BUFFER_ template_parser_parse_result_buffer;
struct _TEMPLATE_PARSER_PARSE_RESULT_BUFFER_{
    char *string;
    unsigned long length;
};
ZEND_BEGIN_MODULE_GLOBALS(template_parser)
    template_parser_parse_result_buffer *result_buffer;
ZEND_END_MODULE_GLOBALS(template_parser)

#ifdef ZTS
#define TEMPLATE_PARSER_G(v) TSRMG(template_parser_globals_id,zend_template_parser_globals *,v)
#else
#define TEMPLATE_PARSER_G(v) (template_parser_globals.v)
#endif

#if ((PHP_MAJOR_VERSION == 5)&&(PHP_MINOR_VERSION < 4))

static int template_parser_output_writer(const char *str,uint length TSRMLS_DC);

#define TEMPLATE_PARSER_PARSE_STORE_RESULT_BUFFER_AND_OUTPUT_HANDLER() \
    template_parser_parse_result_buffer *stored_result_buffer = TEMPLATE_PARSER_G(result_buffer); \
    TEMPLATE_PARSER_G(result_buffer) = NULL; \
    int (*stored_output_func)(const char *str,uint length TSRMLS_DC) = OG(php_body_write); \
    OG(php_body_write) = template_parser_output_writer;

#define TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER() \
    TEMPLATE_PARSER_G(result_buffer) = stored_result_buffer; \
    OG(php_body_write) = stored_output_func;

#elif ((PHP_MAJOR_VERSION == 5)&&(PHP_MINOR_VERSION >= 4))

#define TEMPLATE_PARSER_PARSE_STORE_RESULT_BUFFER_AND_OUTPUT_HANDLER() \
	zval *output_handler = NULL; \
	if (php_output_start_user(output_handler, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) { \
			return ; \
	}

#define TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER() \
	//TODO: Finish this change according to version implementation.
	zval *str; \
	unsigned long total_length = 0; \
	MAKE_STD_ZVAL(str); \
	if(php_output_get_contents(str) == FAILURE){ \
		return ; \
	} \
	template_parser_parse_result_buffer *dest; \
	total_length = Z_STRLEN_P(str); \
	dest = (template_parser_parse_result_buffer *)emalloc(sizeof(template_parser_parse_result_buffer)); \
	dest->string = (char *)emalloc(total_length+1); \
	dest->length = total_length; \
	memcpy(dest->string,Z_STRVAL_P(str),total_length); \
	dest->string[total_length] = '\0'; \
	TEMPLATE_PARSER_G(result_buffer) = dest; \
	php_output_end(TSRMLS_C); \
	zval_ptr_dtor(&str); \

#elif (PHP_MAJOR_VERSION == 7)

#define TEMPLATE_PARSER_PARSE_STORE_RESULT_BUFFER_AND_OUTPUT_HANDLER() \
	zval *output_handler = NULL; \
	if (php_output_start_user(output_handler, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) { \
			return ; \
	}

#define TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER() \
	//TODO: Finish this change according to version implementation.
	zval *str; \
	unsigned long total_length = 0; \
	MAKE_STD_ZVAL(str); \
	if(php_output_get_contents(str) == FAILURE){ \
		return ; \
	} \
	template_parser_parse_result_buffer *dest; \
	total_length = Z_STRLEN_P(str); \
	dest = (template_parser_parse_result_buffer *)emalloc(sizeof(template_parser_parse_result_buffer)); \
	dest->string = (char *)emalloc(total_length+1); \
	dest->length = total_length; \
	memcpy(dest->string,Z_STRVAL_P(str),total_length); \
	dest->string[total_length] = '\0'; \
	TEMPLATE_PARSER_G(result_buffer) = dest; \
	php_output_end(TSRMLS_C); \
	zval_ptr_dtor(&str); \

#endif

#if (PHP_MAJOR_VERSION == 5)

#define TEMPLATE_PARSER_COMPILE_FILE_STORE_ENV(SCOPE) \
    HashTable *stored_active_symbol_table; \
    zend_class_entry *stored_scope; \
    if(EG(active_symbol_table)) { \
        stored_active_symbol_table = EG(active_symbol_table); \
    } else { \
        stored_active_symbol_table = NULL; \
    } \
    stored_scope = EG(scope); \
    EG(scope) = SCOPE; \
    ALLOC_HASHTABLE(EG(active_symbol_table)); \
    zend_hash_init(EG(active_symbol_table),0,NULL,ZVAL_PTR_DTOR,0); \

#define TEMPLATE_PARSER_COMPILE_FILE_RESTORE_ENV() \
    HashTable *free_active_symbol_table = EG(active_symbol_table); \
    EG(scope) = stored_scope; \
    EG(active_symbol_table) = stored_active_symbol_table; \
    zend_hash_destroy(free_active_symbol_table); \
    FREE_HASHTABLE(free_active_symbol_table);

#define TEMPLATE_PARSER_COMPILE_FILE_STORE_OPCODE_ENV() \
    zend_op_array *stored_op_array = EG(active_op_array); \
    zend_op **stored_opline_ptr = EG(opline_ptr); \
    zval **stored_return_value_ptr_ptr = EG(return_value_ptr_ptr);

#define TEMPLATE_PARSER_COMPILE_FILE_RESTORE_OPCODE_ENV() \
    EG(active_op_array) = stored_op_array; \
    EG(opline_ptr) = stored_opline_ptr; \
    EG(return_value_ptr_ptr) = stored_return_value_ptr_ptr;

#endif

PHP_FUNCTION(template_parser_parse);

//PHP_MINIT_FUNCTION(template_parser);
//PHP_MSHUTDOWN_FUNCTION(template_parser);
//PHP_MINFO_FUNCTION(template_parser);
//PHP_RINIT_FUNCTION(template_parser_request);
//PHP_RSHUTDOWN_FUNCTION(template_parser_request);

#endif



