#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_template_parser.h"

#ifdef COMPILE_DL_TEMPLATE_PARSER
ZEND_GET_MODULE(template_parser)
#endif

ZEND_DECLARE_MODULE_GLOBALS(template_parser);

#if ((PHP_MAJOR_VERSION == 5)&&(PHP_MINOR_VERSION < 4))

static int template_parser_output_writer(const char *str,uint length TSRMLS_DC){
    template_parser_parse_result_buffer *source;
    char *dest;
    unsigned long total_length;

    if(str){
        source = TEMPLATE_PARSER_G(result_buffer);
        if(source){
            source->string = (char *)erealloc(source->string,source->length+length+1);
            if(source->string){
                dest = source->string+source->length;
                total_length = source->length+length;
            }
        } else {
            source = (template_parser_parse_result_buffer *)emalloc(sizeof(template_parser_parse_result_buffer));
            source->string = (char *)emalloc(length+1);
            if(source&&source->string){
                dest = source->string;
                total_length = length;
            }
        }
        memcpy(dest,str,length);
        source->string[total_length] = '\0';
        TEMPLATE_PARSER_G(result_buffer) = source;

        return 0;
    } else {
        //TODO:Error handling.
        return 1;
    }
}

#endif

#if (PHP_MAJOR_VERSION == 5)

static int template_parser_extract_param(zval *param,zend_bool openTest TSRMLS_DC){
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

static int template_parser_compile_file(char *template_dir,int template_dir_length,zend_object *real_object,zval *param,zend_bool openTest TSRMLS_DC){
    //File handle.
    zend_file_handle file_handle;
    char real_path[MAXPATHLEN];
    //Opcode.
    zend_op_array *execute_array = NULL;

    if(IS_ABSOLUTE_PATH(template_dir,template_dir_length)&&virtual_realpath(template_dir,real_path)){
        TEMPLATE_PARSER_COMPILE_FILE_STORE_ENV(real_object->ce);
        //Extract params.
        template_parser_extract_param(param,openTest TSRMLS_CC);

        file_handle.filename      = template_dir;
        file_handle.free_filename = 0;
        file_handle.type          = ZEND_HANDLE_FILENAME;
        file_handle.opened_path    = NULL;
        file_handle.handle.fp     = NULL;

        execute_array = zend_compile_file(&file_handle,ZEND_INCLUDE TSRMLS_CC);

        if(execute_array&&file_handle.handle.stream.handle){
            int data = 1;
            if(!file_handle.opened_path){
                file_handle.opened_path = template_dir;
            }
            zend_hash_add(&EG(included_files),file_handle.opened_path,strlen(file_handle.opened_path)+1,(void*)(&data),sizeof(int),NULL);
            zend_destroy_file_handle(&file_handle TSRMLS_CC);
        }
        if(execute_array){
            TEMPLATE_PARSER_COMPILE_FILE_STORE_OPCODE_ENV();
            EG(active_op_array) = execute_array;
            if(!EG(active_symbol_table)){
                zend_rebuild_symbol_table(TSRMLS_C);
            }
            zend_execute(execute_array TSRMLS_CC);
            destroy_op_array(execute_array TSRMLS_CC);
            efree(execute_array);
            execute_array = NULL;
            TEMPLATE_PARSER_COMPILE_FILE_RESTORE_OPCODE_ENV();
        }

        TEMPLATE_PARSER_COMPILE_FILE_RESTORE_ENV();
        return 0;
    } else {
        return 1;
    }
}

PHP_FUNCTION(template_parser_parse){
    //Input params.
    zval *object_container = NULL;
    char *template_dir = NULL;
    zval *param = NULL;
    int template_dir_length = 0;
    zend_bool openTest = 0;
    zend_object *real_object = NULL;
    //Result.
    template_parser_parse_result_buffer *result = NULL;

    //Fetch parameters.
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"os|ab",&object_container,&template_dir,&template_dir_length,&param,&openTest) == FAILURE){
        return ;
    }
    //Start redirect output.
    TEMPLATE_PARSER_PARSE_STORE_RESULT_BUFFER_AND_OUTPUT_HANDLER();
    //Fetch real object in order to fetch it's scope.
    if(Z_TYPE_P(object_container) == IS_OBJECT){
        //TODO: Find out why below line can't work.
        //real_object = Z_OBJ_P(object_container);
        real_object = zend_objects_get_address(object_container TSRMLS_CC);
    } else {
        real_object = NULL;
    }

    if(template_dir_length){ 
        template_parser_compile_file(template_dir,template_dir_length,real_object,param,openTest TSRMLS_CC);
        
        /* Below is wrong zend_compile_string complemention.  
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
            destroy_op_array(execute_array TSRMLS_CC);

            TEMPLATE_PARSER_PARSE_RESTORE_OPCODE_ENV();
        }
        efree(execute_array);
        efree(desc);
        zval_dtor(&template_zval);

        TEMPLATE_PARSER_PARSE_RESTORE_ENV();
        */
    }

    //Fetch result & return processed template string and give it back to PHP.
#if ((PHP_MAJOR_VERSION == 5)&&(PHP_MINOR_VERSION < 4))
    result = TEMPLATE_PARSER_G(result_buffer);
    TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER();
#elif ((PHP_MAJOR_VERSION == 5)&&(PHP_MINOR_VERSION >= 4))
    TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER();
    result = TEMPLATE_PARSER_G(result_buffer);
#endif

    //Result memory leak solved.RETURN_* pass results to default return_value,it won't return until you write `return ;`.
    if(template_dir_length){
        RETURN_STRINGL(result->string,result->length,0);
        efree(result);
        result = NULL;
        return ;
    } else {
        return ;
    }
}

#elif (PHP_MAJOR_VERSION == 7)

static zend_array* template_parser_extract_param(zval *param,zend_bool openTest TSRMLS_DC){
	//Below extract param into new symbol table and generate new symbol table for zend_execute_data use.
	zval *value;
	zend_string *key;
	zend_array *symbol_table = NULL;

	symbol_table = (zend_array*)emalloc(sizeof(zend_array));

	zend_hash_init(symbol_table, 8, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_real_init(symbol_table, 0);

	if(param && (Z_TYPE_P(param) == IS_ARRAY)){
	    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(param), key, value) {
	    	if (EXPECTED(zend_hash_add_new(symbol_table, key, value))) {
	    		Z_TRY_ADDREF_P(value);
	    	}
	    } ZEND_HASH_FOREACH_END();
	}

	return symbol_table;
}

static int template_parser_execute(zend_object *real_object,zend_execute_data *call,zend_array *symbol_table,zend_op_array *execute_array){
	zval ret;
	ZVAL_UNDEF(&ret);
	execute_array->scope = real_object->ce;

	call = zend_vm_stack_push_call_frame(ZEND_CALL_NESTED_CODE
#if PHP_VERSION_ID >= 70100
		| ZEND_CALL_HAS_SYMBOL_TABLE
#endif
		,
		(zend_function*)execute_array, 0, execute_array->scope, real_object);

	call->symbol_table = symbol_table;

	zend_init_execute_data(call, execute_array, &ret);

	ZEND_ADD_CALL_FLAG(call, ZEND_CALL_TOP);
	zend_execute_ex(call);
	zend_vm_stack_free_call_frame(call);

	destroy_op_array(execute_array TSRMLS_CC);
	zend_array_destroy(symbol_table);
	efree(execute_array);
	execute_array = NULL;
	symbol_table = NULL;
	zval_ptr_dtor(&ret);
}

static int template_parser_compile_file(char *template_dir,int template_dir_length,zend_object *real_object,zval *param,zend_bool openTest TSRMLS_DC){
    //File handle.
    zend_file_handle file_handle;
    char real_path[MAXPATHLEN];
    //Opcode.
    zend_op_array *execute_array = NULL;
    zend_array *symbol_table = NULL;
    zend_execute_data **call = NULL;


    if(IS_ABSOLUTE_PATH(template_dir,template_dir_length)&&virtual_realpath(template_dir,real_path)){
        //Extract params into symbol table and generate new symbol table.
    	symbol_table = template_parser_extract_param(param,openTest TSRMLS_CC);

        file_handle.filename      = template_dir;
        file_handle.free_filename = 0;
        file_handle.type          = ZEND_HANDLE_FILENAME;
        file_handle.opened_path    = NULL;
        file_handle.handle.fp     = NULL;

        execute_array = zend_compile_file(&file_handle,ZEND_INCLUDE TSRMLS_CC);

        if(execute_array&&file_handle.handle.stream.handle){
            if(!file_handle.opened_path){
                file_handle.opened_path = template_dir;
            }

            zend_hash_add_empty_element(&EG(included_files),file_handle.opened_path);
            zend_destroy_file_handle(&file_handle TSRMLS_CC);
        }
        if(execute_array){
        	//Create necessary data and execute.
        	template_parser_execute(real_object,call,symbol_table,execute_array);
        }

        return 0;
    } else {
        return 1;
    }
}

PHP_FUNCTION(template_parser_parse){
    //Input params.
    zval *object_container = NULL;
    char *template_dir = NULL;
    zval *param = NULL;
    int template_dir_length = 0;
    zend_bool openTest = 0;
    zend_object *real_object = NULL;
    //Result.
    template_parser_parse_result_buffer *result = NULL;

    //Fetch parameters.
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"os|ab",&object_container,&template_dir,&template_dir_length,&param,&openTest) == FAILURE){
        return ;
    }
    //Start redirect output.
    TEMPLATE_PARSER_PARSE_STORE_RESULT_BUFFER_AND_OUTPUT_HANDLER();
    //Fetch real object in order to fetch it's scope.
    if(Z_TYPE_P(object_container) == IS_OBJECT){
        //TODO: Find out whether below line can work.
        real_object = Z_OBJ_P(object_container);
    } else {
        real_object = NULL;
    }

    if(template_dir_length){
        template_parser_compile_file(template_dir,template_dir_length,real_object,param,openTest TSRMLS_CC);
    }

    //Fetch result & return processed template string and give it back to PHP.
    TEMPLATE_PARSER_PARSE_RESTORE_RESULT_BUFFER_AND_OUTPUT_HANDLER();
    result = TEMPLATE_PARSER_G(result_buffer);

    //Result memory leak solved.RETURN_* pass results to default return_value,it won't return until you write `return ;`.
    if(template_dir_length){
        RETURN_STRINGL(result->string,result->length);
        efree(result);
        result = NULL;
        return ;
    } else {
        return ;
    }
}

#endif

ZEND_BEGIN_ARG_INFO_EX(template_parser_parse_args,0,0,2)
    ZEND_ARG_INFO(0,object)
    ZEND_ARG_INFO(0,template)
    ZEND_ARG_ARRAY_INFO(0,param,1)
    ZEND_ARG_INFO(0,openTest)
ZEND_END_ARG_INFO();

zend_function_entry template_parser_functions[] = {
    PHP_FE(template_parser_parse,template_parser_parse_args)
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
