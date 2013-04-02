#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "main/php_output.h"

#include "php_template_parser.h"

ZEND_DECLARE_MODULE_GLOBALS(template_parser)

#ifdef COMPILE_DL_TEMPLATE_PARSER
ZEND_GET_MODULE(template_parser)
#endif

ZEND_BEGIN_ARG_INFO_EX(template_parser_pause_args,0,0,2)
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
    template_parser_parse_result_buffer *source;
    template_parser_parse_result_buffer *dest;
    unsigned long total_length;

    if(str){
        source = TEMPLATE_PARSER_G(result_buffer);
        if(source){
            source->string = (char *)erealloc(source->string,source->length+length+1);
            if(source->string){
                dest = source->string+source->length;
            }
        } else {
            source = (template_parser_parse_result_buffer *)emalloc(sizeof(template_parser_parse_result_buffer));
            source->string = (char *)emalloc(length+1);
            if(source&&source->string){
                dest = source->string;
            }
        }
        memcpy(dest,str,length);
        source->string[total_length] = '\n';
        TEMPLATE_PARSER_G(result_buffer) = source;

        return 0;
    } else {
        //TODO:Error handling.
        return 1;
    } 
}

PHP_FUNCTION(template_parser_pause){
    //Input params.
    zval *object_container = NULL;
    char *template = NULL;
    zval *param = NULL;
    int template_length = 0;
    zend_bool openTest = 0;
    zend_object *real_object = NULL;
    //Result.
    template_parser_parse_result_buffer *result = NULL;
    //Opcode.
    zend_op_array *execute_array = NULL;
    zval template_zval;
    char *desc = NULL;
    
    TEMPLATE_PARSER_PARSE_STORE_RESULT_BUFFER_AND_OUTPUT_HANDLER(template_parser_output_writer);

    //Fetch parameters.
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"os|ab",&object_container,&template,&template_length,&param,&openTest) == FAILURE){
        return ;
    }
    //Start output.
    if(php_output_start_user(NULL,0,PHP_OUTPUT_HANDLER_STDFLAGSS TSRMLS_CC) == FAILURE){
        return 1; 
    }
    //Fetch real object in order to fetch it's scope.
    if(Z_TYPE_P(object_container) == IS_OBJECT){
        real_object = Z_OBJ_P(object_container);
    } else {
        real_object = NULL;
    }

    if(template_length){
        TEMPLATE_PARSER_PARSE_STORE_ENV(real_object->ce);
        //Extract params.
        template_parser_extract_param(param TSRMLS_CC);
        //Format template zval.
        INIT_ZVAL(template_zval);
        Z_TYPE(template_zval) = IS_STRING;
        Z_STRVAL(template_zval) = emalloc(template_length+1);
        Z_STRLEN(template_zval) = template_length;
        snprintf(Z_STRVAL(template_zval),template_length+1,"%s",template);
        //Format desc.
        desc = zend_make_compiled_string_description("Template" TSRMLS_CC);
        //Compile template.
        execute_array = zend_compile_string(&template_zval,desc TSRMLS_CC);

        if(execute_array){
            TEMPLATE_PARSER_PARSE_STORE_OPCODE_ENV();
            EG(active_op_array) = execute_array;
            zend_execute(execute_array TSRMLS_CC);

            TEMPLATE_PARSER_PARSE_RESTORE_OPCODE_ENV();
        }
        destory_op_array(execute_array TSRMLS_CC);
        efree(execute_array);
        efree(desc);
        zval_dtor(&template_zval);

        TEMPLATE_PARSER_PARSE_RESTORE_ENV();
    }


    //Return processed template string and give it back to PHP.
    result = TEMPLATE_PARSER_G(result_buffer);
    TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER();
    php_output_end(TSRMLS_CC);
    //TODO: result leak memory every time it's called. Must be solved.
    if(template_length){
        RETURN_STRINGL(result->string,result->length,0);
    } else {
        return ;
    }
}
