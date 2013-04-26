Questions found within writing:
1.1 zend_compile_string() can complement eval(),because of its source is string.Source must be well typed PHP string.
1.2 zend_compile_file() can complement renderFile(),because of its source is file.It can treat other strings equal.
2.1 CG is compiler global EG is executor global ,OG is output global.
2.2 CG and EG have there own function_table and class_table and others.
2.3 CG and EG can transport to each other.
2.4 I didnt understand why EG(active_op_array) must be bind to new op array before execute,and why if didn't bind it'll execute error, and why it don't need to store and restore.
