#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "main/php_output.h"

#include "php_template_parser.h"

ZEND_DECLARE_MODULE_GLOBALS(template_parser)

#ifdef COMPILE_SO_TEMPLATE_PARSER
ZEND_GET_MODULE(template_parser)
#endif
ZEND_BEGIN_ARG_INFO_EX(template_parser_pause_args,ZEND_ZEND_BY_VAL,ZEND_RETURN_VAL,2)
    ZEND_ARG_INFO(0,object)
    ZEND_ARG_INFO(0,template)
    ZEND_ARG_ARRAY_INFO(0,param,1)
    ZEND_ARG_INFO(0,openTest)
ZEND_END_ARG_INFO()

zend_function_entry template_parser_functions[] = {
    PHP_FE(template_parser_pause,template_parser_pause_args)
    {NULL,NULL,NULL}
};

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
};

PHP_FUNCTION(template_parser_pause){
    //Input params.
    zval *object = NULL;
    char *template = NULL;
    zval *param = NULL;
    int template_length = 0;
    zend_bool openTest = 0;
    //Storage of default callback
    int(*template_parser_output_func)(const *str,uint length TSRMLS_DC);
    //Storage 4 Opcode.
    zend_op_array *execute_array;
    zend_op_array *callback_op_array;
    zend_op **callback_opline_ptr;
    zval **callback_return_value_ptr_ptr;
    HashTable *callback_symbol_table;
    zval template_zval;
    char *desc;

    //Free mem which have been allocated to store output.
    template_parser_output *free = TEMPLATE_PARSER_G(result_string);
    efree(free->string);
    efree(free);
    free = NULL;
    TEMPLATE_PARSER_G(result_string) = NULL;

    //Fetch parameters.
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"os|ab",&object,&template,&template_length,&param,&openTest) == FAILURE){
        return ;
    }

    //redirect php output.
    template_parser_output_func = OG(php_body_write);
    OG(php_body_write) = template_parser_output_writer;
    //Store default symbol table & scope.
    //EG(scope)暂时继承原来的上下文.
    if(EG(active_symbol_table)){
        callback_symbol_table = EG(active_symbol_table);
    } else {
        callback_symbol_table = NULL; 
    }
    ALLOC_HASHTABLE(EG(active_symbol_table));
    zend_hash_init(EG(active_symbol_table),0,NULL,ZVAL_PTR_DTOR,0);
    //extract params.
    template_parser_extract_param(param TSRMLS_CC);
    //Compile & execute template.
    if(template_length){
        INIT_ZVAL(template_zval);
        Z_TYPE(template_zval) = IS_STRING;
        Z_STRLEN(template_zval) = template_length;
        Z_STRVAL(template_zval) = emalloc(template_length+1);
        snprintf(Z_STRVAL(template_zval),Z_STRLEN(template_zval)+1,"%s",template);

        desc = zend_make_compiled_string_description("template string" TSRMLS_CC);
        execute_array = zend_compile_string(&template_zval,desc TSRMLS_CC);
        efree(desc);
        zval_dtor(&template_zval);

        if(execute_array){
            //Before execute save opcode params.
            callback_op_array = EG(active_op_array);
            callback_opline_ptr = EG(opline_ptr);
            callback_return_value_ptr_ptr = EG(return_value_ptr_ptr);

            EG(active_op_array) = execute_array;
            zend_execute(execute_array TSRMLS_CC);

            //After execute restore opcode params.
            EG(return_value_ptr_ptr) = callback_return_value_ptr_ptr;
            EG(opline_ptr) = callback_opline_ptr;
            EG(active_op_array) = callback_op_array;
        }
        destroy_op_array(execute_array TSRMLS_CC);
        efree(execute_array);
    }
    //Recover default symbol table & scope.
    if(EG(active_symbol_table)){
        zend_hash_destory(EG(active_symbol_table));
        FREE_HASHTABLE(EG(active_symbol_table));
    }
    EG(active_symbol_table) = callback_symbol_table;
    //EG(scope)暂时继承原来的上下文.
    //give php output back to normal.
    OG(php_body_write) = template_parser_output_func;
    
    //Return processed template string and give it back to PHP.
    RETURN_STRINGL(TEMPLATE_PARSER_G(string),TEMPLATE_PARSER_G(length),0);

}
static int template_parser_extract_param(zval *param TSRMLS_DC){
    HashPosition it_pos;
    zval **param_value = NULL;
    char *name;
    ulong count;
    uint length;

    
    if(!EG(active_symbol_table)){
        return 1;
    }

    if(param && (Z_TYPE_P(param) == IS_ARRAY)){
        for(zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(param),&it_pos);
            zend_hash_get_current_data_ex(Z_ARRVAL_P(param),(void **)&param_value,&it_pos) == SUCCESS;
            zend_hash_move_forward_ex(Z_ARRVAL_P(param),&it_pos)){
            if(zend_hash_get_current_key_ex(Z_ARRVAL_P(param),&name,&length,&count,0,&it_pos) != HASH_KEY_IS_STRING){
                continue;
            }
            //TODO: key name validation.
            ZEND_SET_SYMBOL_WITH_LENGTH(EG(active_symbol_table),name,length,*param_value,Z_REFCOUNT_P(*param_value)+1,PZVAL_IS_REF(*param_value));
        }
        return 0;
    }
    return 1;
}
static int template_parser_output_writer(const char *str,uint length TSRMLS_DC){
    unsigned long size = 0;
    template_parser_output *source = TEMPLATE_PARSER_G(result_string);
    template_parser_output *dest = NULL;
    char *realloc_mem = NULL;
    char *copy_start = NULL;

    //Input exists.
    if(str){
        if(!source){
            dest = (template_parser_output *)emalloc(sizeof(template_parser_output));
            dest->string = (char *)emalloc(sizeof(length+1));
            if(!dest->string){
                //TODO: Error reporting.
                return 1;
            }
            dest->length = length+1;
            copy_start = dest->string;
        } else {
             realloc_mem = (char *)realloc(source->string,source->length+length);
             if(!realloc_mem){
                //TODO: Error reporting.
                return 1;
             }
             dest->length = source->length+length;
             dest->string = realloc_mem;
             copy_start = dest->string+source->length-1;
        }
        memcpy(copy_start,str,length);
        dest->string[dest->length+length]='\0';
        TEMPLATE_PARSER_G(result_string) = dest;
        return length;
        
    } else {
        return 1;
    }
}

