#ifndef _PHP_TEMPLATE_PARSER_H
#define _PHP_TEMPLATE_PARSER_H

#include "php.h"

#define PHP_TEMPLATE_PARSER_VERSION 1.0

extern zend_module_entry template_parser_module_entry;

//PHP_MINIT_FUNCTION(template_parser);
//PHP_MSHUTDOWN_FUNCTION(template_parser);
//PHP_MINFO_FUNCTION(template_parser);
//PHP_RINIT_FUNCTION(template_parser_request);
//PHP_RSHUTDOWN_FUNCTION(template_parser_request);

PHP_FUNCTION(template_parser_pause);

#endif


