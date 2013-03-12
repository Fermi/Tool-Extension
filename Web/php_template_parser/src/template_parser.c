#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#include "php_template_parser.h"

ZEND_BEGIN_ARG_INFO_EX(template_parser_pause_args,ZEND_ZEND_BY_VAL,ZEND_RETURN_VAL,2)
    ZEND_ARG_INFO(0,object)
    ZEND_ARG_INFO(0,template)
    ZEND_ARG_ARRAY_INFO(0,param,1)
    ZEND_ARG_INFO(0,openTest)
ZEND_END_ARG_INFO()

zend_function_entry template_parser_functions[] = {
    PHP_FE(template_parser_pause,template_parser_pause_args)
    PHP_FE_END
}

zend_module_entry template_parser_module_entry = {
    STANDARD_MODULE_HEADER,
    "template_parser",
    template_parser_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_TEMPLATE_PARSER_VERSION,
    STANDARD_MODULE_PROPERTIES
}

PHP_FUNCTION(template_parser_pause){
    zval *object = NULL;
    char *template = NULL;
    zval *param = NULL;
    int template_length = 0;
    zend_bool openTest = 0;
    int process_start = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"os|ab",&object,&template,&template_length,&param) == FAILURE){
        return ;
    }
   
    for (process_start = 0;process_start <= template_length;){
        switch(){
            
        }
    }

}

static int template_parser_extract_param(){
    
}
static int template_parser_add_return_string(){
    
}
static int template_parser_find_token(){
    
}
static int template_parser_get_process_start(){
    
}
static inline int template_parser_process_token_if(){
    
}
static inline int template_parser_process_token_for(){
    
}
static inline int template_parser_process_token_switch(){
    
}
static inline int template_parser_process_token_while(){
    
}
static inline int template_parser_process_token_foreach(){
    
}


