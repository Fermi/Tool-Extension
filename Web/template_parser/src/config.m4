dnl
PHP_ARG_ENABLE(template_parser,whether to enable template_parser support,
[--disable-template_parser, Disable template_parser support], yes)

if test "$PHP_TEMPLATE_PARSER" != "no"; then
    PHP_NEW_EXTENSION(template_parser,template_parser.c,$ext_shared)
fi
