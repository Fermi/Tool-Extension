#ifndef _PHP_TEMPLATE_PARSER_H
#define _PHP_TEMPLATE_PARSER_H

#include "php.h"

#define PHP_TEMPLATE_PARSER_VERSION 1.0

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define TEMPLATE_PARSER_G(v) TSRMG(template_parser_globals_id,zend_template_parser_globals *,v)
#else
#define TEMPLATE_PARSER_G(v) (template_parser_globals.v)
#endif

extern zend_module_entry template_parser_module_entry;

//PHP_MINIT_FUNCTION(template_parser);
//PHP_MSHUTDOWN_FUNCTION(template_parser);
//PHP_MINFO_FUNCTION(template_parser);
//PHP_RINIT_FUNCTION(template_parser_request);
//PHP_RSHUTDOWN_FUNCTION(template_parser_request);

struct _TEMPLATE_PARSER_OUTPUT_{
    char *string;
    unsigned long length;
};

typedef _TEMPLATE_PARSER_OUTPUT_ template_parser_output;

ZEND_BEGIN_MODULE_GLOBALS(template_parser)
    template_parser_output *result_string;
ZEND_END_MODULE_GLOBALS(template_parser)



extern ZEND_DECLARE_MODULE_GLOBALS(template_parser);

PHP_FUNCTION(template_parser_pause);

#endif



